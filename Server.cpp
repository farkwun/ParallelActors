/* udpserver.c */ 
// Copied from http://www.pythonprasanna.com/Papers%20and%20Articles/Sockets/udpserver.c
//
// Sourced from http://www.prasannatech.net/2008/07/socket-programming-tutorial.html
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <unordered_map>
#include <string>
#include <iostream>
#include "Map.h"
#include "PDUConstants.h"
#include "NetworkHelpers.h"
#include "Actor.h"
#include <pthread.h>
#include <chrono>
#include <thread>
#include <SFML/Graphics.hpp>
#include <mpi.h>
#include <set>
#include <mutex>

bool debug   = false;
bool gui     = true;
int sleep_ms = 0;

int sock;
struct sockaddr_in server_addr , client_addr;
int bytes_read;
socklen_t addr_len;

pthread_t receive_manager_id;
pthread_t map_manager_id;

pthread_t input_forwarder_id;
pthread_t output_forwarder_id;
pthread_t gui_manager_id;

std::mutex rt_mtx;

char recvBuff[BUFLEN];
char PDU_TYPE;
char PDU_ID[ID_LEN + 1];
char PDU_DATA[DATALEN];

std::unordered_map<std::string, Actor> current_actors;
std::set<std::string> new_actors;
std::set<std::string> dead_actors;

Map map;
Map original_map;

sf::RenderWindow * map_window;
int border_size = 100;
sf::Color win_bg_color = sf::Color::Black;
sf::Texture map_texture;
sf::Sprite map_sprite;
sf::Event event;

// MPI Stuff
int world_size, world_rank, segment_rows, segment_cols, segment_size;
int top_buf_edge_index = -1;
int bot_buf_edge_index = -1;
int start_index, end_index;
bool root = false;
static const int root_id = 0;
static const int DEFAULT_TAG = 0;
static const int MAP_TAG = 1;
char forwardBuff[BUFLEN];

std::unordered_map<std::string, struct sockaddr_in> routing_table;

int my_offset;

// MPI Funcs
void MPISend(char * data, int length, MPI_Datatype type, int id, int tag);
void MPIReceive(char * buffer, int length,  MPI_Datatype type, int id, int tag);
void MPISend(int * data, int length, MPI_Datatype type, int id, int tag);
void MPIReceive(int * buffer, int length,  MPI_Datatype type, int id, int tag);
int RandomRank();
int DerivedRank(char * PDU);
void DisplayMap();
std::vector< std::vector<char> > GatherSegments();

// Sourced from https://github.com/SFML/SFML/wiki/Source:-Zoom-View-At-(specified-pixel)
const float zoomAmount{ 1.1f }; // zoom by 10%

// Sourced from http://stackoverflow.com/a/41789414
bool moving = false;
sf::Vector2f oldPos;
sf::View map_view;

std::string GenerateID(){
  char new_id[ID_LEN + 1];

  static const char alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

  for (int i = 0; i < ID_LEN; i++){
    new_id[i] = alphanum[rand() % (sizeof(alphanum)-1)];
  }

  new_id[ID_LEN] = '\0';

  std::string output_id(new_id);

  return output_id;
}

char BooltoChar(bool input){
  if (input){
    return TRUE;
  }else{
    return FALSE;
  }
}

char * Unroll2DVector(std::vector< std::vector<char> > segment){
  int i, j, k;
  char * unrolled_segment;
  unrolled_segment = (char *) malloc(BUFLEN);

  k = 0;
  for(i = 0; i < segment.size(); i++){
    for(j = 0; j < segment[0].size(); j++){
      unrolled_segment[k] = segment[i][j];
      k++;
    }
  }

  unrolled_segment[k] = '\0';

  return unrolled_segment;
}

void print_map_segment(std::vector< std::vector<char> > segment){
  int rows = segment.size();
  int cols = segment[0].size();
  for (int i = 0; i < rows; i++){
    for (int j = 0; j < cols; j++){
      std::cout << segment[i][j];
    }
  }
}

void SendPDU(char * PDU, struct sockaddr_in in_address){
  sendto(sock, PDU, BUFLEN, 0,
      (struct sockaddr *)&in_address, sizeof(in_address));
  free(PDU);
}

void UpdateActorMove(char * PDU){
  std::string actor_id;
  Coordinate new_move;
  Actor * actor;

  actor_id = GetField(PDU_ID_INDEX, PDU, ID_LEN);

  try {
    actor = &current_actors.at(actor_id);
  } catch (const std::out_of_range& err) {
    if (debug) {
      std::cout << "No such actor with id = " << actor_id << " was found" << std::endl;
    }
    return;
  }
  actor->set_timeout(false);

  new_move.set_row(StoI(
        GetField(MOVE_NEXT_ROW_INDEX, PDU, MOVE_FIELD_LEN)) - my_offset);

  new_move.set_col(StoI(
        GetField(MOVE_NEXT_COL_INDEX, PDU, MOVE_FIELD_LEN)));

  actor->set_next_move(new_move);
}

void RegisterNewActor(char * PDU){
  new_actors.insert(GetField(PDU_ID_INDEX, PDU, ID_LEN));
}

std::string SetIDforRoute(struct sockaddr_in address){
  std::string new_id;
  bool already_exists = true;

  std::lock_guard<std::mutex> guard(rt_mtx);

  while(already_exists){
    new_id = GenerateID();
    if (routing_table.count(new_id) == 0){
      already_exists = false;
    }
  }

  routing_table[new_id] = address;
  return new_id;
}

// builds a Setup PDU for a given actor
char * SetupPDU(Actor actor){
  char * PDU;
  PDU = (char *) malloc(BUFLEN);
  // fill PDU with null characters
  memset(&PDU[0], 0, sizeof(PDU));

  // fill PDU fields - type, ID, vision_size, step_size, position, destination
  PDU[0] = SETUP;

  strncpy(PDU + PDU_ID_INDEX, actor.get_id().c_str(), ID_LEN);

  sprintf(PDU + SETUP_VISION_INDEX,"%d",map.get_total_surroundings_grid_size());

  sprintf(PDU + SETUP_STEP_INDEX,"%d",map.get_step_size());

  sprintf(PDU + SETUP_CURR_ROW_INDEX,"%d",actor.get_true_position().get_row());

  sprintf(PDU + SETUP_CURR_COL_INDEX,"%d",actor.get_true_position().get_col());

  sprintf(PDU + SETUP_DEST_ROW_INDEX,"%d",actor.get_destination().get_row());

  sprintf(PDU + SETUP_DEST_COL_INDEX,"%d",actor.get_destination().get_col());

  if(debug){
    actor.print();
  }

  return PDU;
}

char * VisionPDU(Actor actor){
  char * PDU;
  char * vision_grid;
  PDU = (char *) malloc(BUFLEN);

  // fill PDU and vision_grid with null characters
  for (int i = 0; i < BUFLEN; i++){
    PDU[i] = '\0';
  }

  vision_grid = Unroll2DVector(map.GetSurroundings(actor.get_position()));

  PDU[PDU_TYPE_INDEX] = VISION;
  strncpy(PDU + PDU_ID_INDEX, actor.get_id().c_str(), ID_LEN);

  PDU[VISION_COLLIDED_INDEX] = BooltoChar(actor.get_collided());
  PDU[VISION_ARRIVED_INDEX]  = BooltoChar(actor.get_arrived());
  PDU[VISION_TIMEOUT_INDEX]  = BooltoChar(actor.get_timeout());

  sprintf(PDU + VISION_CURR_ROW_INDEX,"%d",actor.get_true_position().get_row());

  sprintf(PDU + VISION_CURR_COL_INDEX,"%d",actor.get_true_position().get_col());

  for (int i = VISION_GRID_INDEX; i < VISION_GRID_INDEX + strlen(vision_grid); i++){
    PDU[i] = vision_grid[i-VISION_GRID_INDEX];
  }

  free(vision_grid);
  return PDU;
}

char * RegisterPDU(std::string id){
  char * PDU;
  PDU = (char *) malloc(BUFLEN);

  PDU[PDU_TYPE_INDEX] = REGISTER;
  strncpy(PDU + PDU_ID_INDEX, id.c_str(), ID_LEN);

  return PDU;
}

// wrapping PDU before forwarding
char * ForwardPDU(char * PDU){
  char * forwardPDU;
  forwardPDU = (char *) malloc(BUFLEN);

  memcpy(forwardPDU, PDU, BUFLEN);

  return forwardPDU;
}

void InitializeMap(){
  map.InitializeActorDimensions();
  map.InitializeVision();
  map.InitializeStepSize();
}

void SetMaps(){
  InitializeMap();

  Map copy = map;
  original_map = copy;
}

void LaunchWindow(){
  if (!gui){
    return;
  }
  int map_rows = map.get_map_rows();
  int map_cols = map.get_map_cols();
  map_window = new sf::RenderWindow(sf::VideoMode(
        map_cols + border_size + border_size,
        map_rows + border_size + border_size),
      "Parallel Actors");

  map_texture.create(map_cols, map_rows);

  map_sprite.setPosition(border_size, border_size);
  map_sprite.setTexture(map_texture);

  map_view = map_window->getDefaultView();

}

// moves waiting actors to current list
void AddNewActors(){
  if (new_actors.empty()){
    return;
  }

  std::string new_actor_id;
  std::set<std::string> temp(new_actors);
  std::set<std::string>::iterator it;

  it = temp.begin();

  while(it != temp.end()){
    new_actor_id      = *it;

    Coordinate start       = map.RandomEmptyLocation();
    Coordinate destination = original_map.RandomDestination();

    Actor new_actor(new_actor_id, start, destination, my_offset);

    map.AddActor(new_actor);

    current_actors[new_actor_id] = new_actor;

    MPISend(SetupPDU(new_actor), BUFLEN, MPI_CHAR, root_id, DEFAULT_TAG);

    MPISend(VisionPDU(new_actor), BUFLEN, MPI_CHAR, root_id, DEFAULT_TAG);

    new_actors.erase(new_actor_id);

    it++;
  }
}

void ParseRecvdPDU(){
  std::string id;
  bytes_read = recvfrom(sock,recvBuff,BUFLEN,0,
      (struct sockaddr *)&client_addr, &addr_len);

  PDU_TYPE = recvBuff[PDU_TYPE_INDEX];
  if (debug){
    PrintPDU(recvBuff);
  }
  switch(PDU_TYPE){
    case REGISTER :
      id = SetIDforRoute(client_addr);
      MPISend(RegisterPDU(id), BUFLEN, MPI_CHAR, RandomRank(), DEFAULT_TAG);;
      break;
    case MOVEMENT :
      MPISend(ForwardPDU(recvBuff), BUFLEN, MPI_CHAR, DerivedRank(recvBuff), DEFAULT_TAG);
      break;
    default:
      break;
  }
}

void IterateCurrentActors(Actor (*f)(Actor)){
  if (current_actors.empty()){
    return;
  }

  std::unordered_map<std::string, Actor>::iterator it;

  it = current_actors.begin();

  while (it != current_actors.end()){
    current_actors[it->first] = (*f)(it->second);
    it++;
  }
};

Actor PrintActor(Actor actor){
  if(debug){
    std::cout << "PRINTING ACTOR ... " << actor.get_id() << std::endl;
    actor.print();
    std::cout << "====================== : " <<actor.get_id() << std::endl;
  }
  return actor;
}

Actor SetTimeouts(Actor actor){
  if (actor.get_next_move().get_row() < 0
      && actor.get_next_move().get_col() < 0
      && !actor.get_timeout()){
    actor.set_timeout(true);
  }
  return actor;
}

Actor MoveActor(Actor actor){
  if(!actor.get_timeout()){
    Coordinate reset_coordinate;
    actor = map.MoveActor(actor, actor.get_next_move());
    actor.set_next_move(reset_coordinate);
  }
  return actor;
}

Actor DetectCollision(Actor actor){
  actor = map.CheckCollision(actor);
  if(actor.get_collided()){
    dead_actors.insert(actor.get_id());
  }
  return actor;
}

Actor CheckDestination(Actor actor){
  if (map.AtDestination(actor)){
    actor.set_arrived(true);
  }else{
    actor.set_arrived(false);
  }
  if(actor.get_arrived()){
    // die happy
    dead_actors.insert(actor.get_id());
  }
  return actor;
}

Actor SendUpdate(Actor actor){
  MPISend(VisionPDU(actor), BUFLEN, MPI_CHAR, root_id, DEFAULT_TAG);
  return actor;
}

void DeleteDeadActors(){
  std::string dead_actor_id;

  if (dead_actors.empty()){
    return;
  }

  std::set<std::string> temp(dead_actors);
  std::set<std::string>::iterator it;

  it = temp.begin();

  while(it != temp.end()){
    dead_actor_id = *it;
    if (current_actors.count(dead_actor_id) != 0){
      map.ClearActor(current_actors.at(dead_actor_id));
      current_actors.erase(dead_actor_id);
      dead_actors.erase(dead_actor_id);
    }
    it++;
  }
}

void zoomViewAt(sf::Vector2i pixel, sf::RenderWindow* window, float zoom)
{
  // Sourced from https://github.com/SFML/SFML/wiki/Source:-Zoom-View-At-(specified-pixel)
  const sf::Vector2f beforeCoord{ window->mapPixelToCoords(pixel) };
  map_view.zoom(zoom);
  window->setView(map_view);
  const sf::Vector2f afterCoord{ window->mapPixelToCoords(pixel) };
  const sf::Vector2f offsetCoords{ beforeCoord - afterCoord };
  map_view.move(offsetCoords);
  window->setView(map_view);
}

void ParseEvent(sf::Event event){
  switch(event.type){
    case sf::Event::Closed :
      {
        map_window->close();
        exit(0);
        break;
      }
      // Sourced from https://github.com/SFML/SFML/wiki/Source:-Zoom-View-At-(specified-pixel)
    case sf::Event::MouseWheelScrolled :
      {
        if (event.mouseWheelScroll.delta > 0) {
          zoomViewAt({ event.mouseWheelScroll.x,
              event.mouseWheelScroll.y },
              map_window, (1.f / zoomAmount));
        }
        else if (event.mouseWheelScroll.delta < 0) {
          zoomViewAt({ event.mouseWheelScroll.x,
              event.mouseWheelScroll.y },
              map_window, zoomAmount);
        }
        break;
      }
      // Sourced from http://stackoverflow.com/a/41789414
    case sf::Event::MouseButtonPressed :
      {
        // Mouse button is pressed, get the position
        // and set moving as active
        if (event.mouseButton.button == 0) {
          moving = true;
          oldPos = map_window->mapPixelToCoords(sf::Vector2i(
                event.mouseButton.x, event.mouseButton.y));
        }
        break;
      }
    case sf::Event::MouseButtonReleased :
      {
        // Mouse button is released, no longer move
        if (event.mouseButton.button == 0) {
          moving = false;
        }
        break;
      }
    case sf::Event::MouseMoved :
      {
        // Ignore mouse movement unless a button is pressed (see above)
        if (!moving)
          break;
        // Determine the new position in world coordinates
        const sf::Vector2f newPos = map_window->mapPixelToCoords(
            sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        // Determine how the cursor has moved
        // Swap these to invert the movement direction
        const sf::Vector2f deltaPos = oldPos - newPos;

        // Move our view accordingly and update the window
        map_view.setCenter(map_view.getCenter() + deltaPos);
        map_window->setView(map_view);

        // Save the new position as the old one
        // We're recalculating this, since we've changed the view
        oldPos = map_window->mapPixelToCoords(sf::Vector2i(
              event.mouseMove.x, event.mouseMove.y));
        break;
      }
  }
}

void SendPixels(){
  if (!gui){
    return;
  }
  int start, end;
  start = top_buf_edge_index;
  end   = end_index - start_index;
  if (top_buf_edge_index < 0){
    start = start_index;
  }
  std::vector<int> send = map.Compress2DVectorTo1D(start, end, map.get_map());

  MPI_Send(&send.front(), send.size(), MPI_INT, 0, MAP_TAG, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);
}

void *MapManager(void *args){
  while(1){
    IterateCurrentActors(&PrintActor);
    IterateCurrentActors(&SetTimeouts);
    IterateCurrentActors(&MoveActor);
    IterateCurrentActors(&DetectCollision);
    IterateCurrentActors(&CheckDestination);
    AddNewActors();
    IterateCurrentActors(&SendUpdate);
    DeleteDeadActors();
    SendPixels();
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
  }
}

void MPI_Initialize(){
  // Get the number of processes
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // Get the rank of the process
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
}

void MPIReceive(char * buffer, int length, MPI_Datatype type,  int id, int tag){
  MPI_Recv(buffer, length, type, id, tag,
      MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

void MPIReceive(int * buffer, int length, MPI_Datatype type,  int id, int tag){
  MPI_Recv(buffer, length, type, id, tag,
      MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

void MPISend(char * data, int length, MPI_Datatype type, int id, int tag){
  MPI_Send(data, length, type, id, tag, MPI_COMM_WORLD);
  free(data);
}

void MPISend(int * data, int length, MPI_Datatype type, int id, int tag){
  MPI_Send(data, length, type, id, tag, MPI_COMM_WORLD);
  free(data);
}

int RandomRank(){
  int random_rank = (1 + (rand() % (world_size - 1)));
  return random_rank;
}

int DerivedRank(char * PDU){
  int current_row = StoI(GetField(MOVE_CURR_ROW_INDEX, PDU, MOVE_FIELD_LEN));

  // leverage 'truncation toward zero'
  int derived_rank = current_row/segment_size;

  // map machines begin at rank 1, not 0
  derived_rank += 1;

  return derived_rank;
}

void ParseMPIRecv(){
  MPIReceive(recvBuff, BUFLEN, MPI_CHAR, root_id, DEFAULT_TAG);

  PDU_TYPE = recvBuff[PDU_TYPE_INDEX];

  if (debug){
    PrintPDU(recvBuff);
  }
  switch(PDU_TYPE){
    case REGISTER :
      RegisterNewActor(recvBuff);
      break;
    case MOVEMENT :
      UpdateActorMove(recvBuff);
      break;
    default:
      break;
  }
}

void *ReceiveManager(void *args){
  while(1){
    ParseMPIRecv();
  }
}

void *InputForwarder(void *args){
  while(1){
    ParseRecvdPDU();
  }
}

struct sockaddr_in GetAddressFromID(std::string id){
  std::lock_guard<std::mutex> guard(rt_mtx);
  return routing_table[id];
}

void DeleteRouteForID(std::string id){
  std::lock_guard<std::mutex> guard(rt_mtx);
  routing_table.erase(id);
}

void ForwardRecvdPacket(){
  std::string id;
  MPIReceive(forwardBuff, BUFLEN, MPI_CHAR, MPI_ANY_SOURCE, DEFAULT_TAG);
  id = GetField(PDU_ID_INDEX, forwardBuff, ID_LEN);
  SendPDU(ForwardPDU(forwardBuff), GetAddressFromID(id));
  if (forwardBuff[VISION_COLLIDED_INDEX] == TRUE ||
      forwardBuff[VISION_ARRIVED_INDEX]  == TRUE){
    DeleteRouteForID(id);
  }
}

void *OutputForwarder(void *args){
  while(1){
    ForwardRecvdPacket();
  }
}

void CreateImageFromMap(sf::Uint8 * image, std::vector< std::vector<char> > in_map){
  int num_rows, num_cols, col, row, pindex;
  int bytes_per_pixel = 4;
  num_rows = in_map.size();
  num_cols = in_map[0].size();
  int num_pixels = num_cols * num_rows * bytes_per_pixel;
  image = new sf::Uint8[num_pixels];

  for (row = 0; row < num_rows; row++){
    for (col = 0; col < num_cols; col++){
      pindex = (row * num_cols + col) * bytes_per_pixel;

      image[pindex+map.R] = map.get_pixel_for_char(in_map[row][col], map.R);
      image[pindex+map.G] = map.get_pixel_for_char(in_map[row][col], map.G);
      image[pindex+map.B] = map.get_pixel_for_char(in_map[row][col], map.B);
      image[pindex+map.A] = map.get_pixel_for_char(in_map[row][col], map.A);
    }
  }
}

std::vector<char> DecompressRow(std::vector<int> compress_row){
  int size = compress_row.size();
  int i = 0;
  int j;
  char curr_char;
  int num_chars;
  std::vector<char> decompress_row;
  while (i < size){
    num_chars = compress_row[i];
    curr_char = compress_row[i+1];
    for (j = 0; j < num_chars; j++){
      decompress_row.push_back(curr_char);
    }
    i += 2;
  }
  return decompress_row;
}
std::vector< std::vector<char> > GatherSegments(){
  int i, amount;
  std::vector< std::vector<int> > compressed_map;
  int * received_vector;
  MPI_Status status;
  for (i = 1; i < world_size; i++){
    received_vector = NULL;
    MPI_Probe(i, 1, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_INT, &amount);
    received_vector = (int *) malloc (sizeof(int) * amount);
    MPIReceive(received_vector, amount, MPI_INT, i, MAP_TAG);
    std::vector<int> data(received_vector, received_vector + amount);
    compressed_map.push_back(data);
    free(received_vector);
  }
  std::vector< std::vector<char> > new_map;
  std::vector<int> temp;
  for (int i = 0; i < compressed_map.size(); i++)
  {
    for (int j = 0; j < compressed_map[i].size(); j++)
    {
      if (compressed_map[i][j] == map.DELIM){
        new_map.push_back(DecompressRow(temp));
        temp.clear();
      }else{
        temp.push_back(compressed_map[i][j]);
      }
    }
  }
  return new_map;
}

void DisplayMap(){
  if (!gui){
    return;
  }

  sf::Uint8 * image;

  if(map_window->isOpen()){
    while(map_window->pollEvent(event)){
      ParseEvent(event);
    }

    map.set_map(GatherSegments());
    InitializeMap();

    map_texture.update(map.get_map_pixels());

    map_window->clear(win_bg_color);
    map_window->draw(map_sprite);
    map_window->display();
  }
  MPI_Barrier(MPI_COMM_WORLD);
}

void *GUIManager(void *args){
  while(1){
    DisplayMap();
  }
}

void SetRoleVars(){
  if (world_rank == 0){
    root = true;
  }else {
    root = false;
  }
}

void GetMySegment(){
  std::vector< std::vector<char> > the_map = map.get_map();
  int map_rows = the_map.size();
  int map_cols = the_map[0].size();
  int vision_radius = map.get_vision_radius();
  segment_rows = map_rows;
  segment_cols = map_cols;
  segment_size = map_rows/(world_size - 1);
  start_index = (world_rank - 1) * segment_size;
  end_index = (world_rank) * segment_size - 1;

  std::vector< std::vector<char> > my_map;
  std::vector<char> empty_row(map_cols, '0');

  int i,j,k;

  if (root) {
    return;
  }

  if (start_index != 0){
    for (i = 0; i < vision_radius; i++){
      my_map.push_back(empty_row);
    }
    top_buf_edge_index = i;
  }

  for (i = start_index; i <= end_index; i++){
    my_map.push_back(the_map[i]);
  }

  if (world_rank == world_size - 1){
    for (i = end_index + 1; i < map_rows; i++){
      my_map.push_back(the_map[i]);
    }
    end_index = map_rows - 1;
  }else{
    for (i = 0; i < vision_radius; i++){
      my_map.push_back(empty_row);
    }
    bot_buf_edge_index = my_map.size() - vision_radius - 1;
  }
  segment_rows = my_map.size();
  segment_cols = my_map[0].size();

  if (world_rank == 1){
    my_offset = 0;
  }else{
    my_offset = start_index - top_buf_edge_index;
  }

  if (debug) {
    printf("My rank is %d, my start index is %d, my end index is %d, my top_buf_edge is %d, and my bot_buf_edge is %d\n", world_rank, start_index, end_index, top_buf_edge_index, bot_buf_edge_index);
  }
  map = Map(my_map);
  InitializeMap();
}

void LaunchThreads(){
  GetMySegment();
  if (root) {
    LaunchWindow();
    pthread_create(&input_forwarder_id, NULL, &InputForwarder, NULL);
    pthread_create(&output_forwarder_id, NULL, &OutputForwarder, NULL);
    pthread_create(&gui_manager_id, NULL, &GUIManager, NULL);
  }else {
    pthread_create(&receive_manager_id, NULL, &ReceiveManager, NULL);
    pthread_create(&map_manager_id, NULL, &MapManager, NULL);
  }
}

void run(){
  SetMaps();
  SetRoleVars();
  LaunchThreads();
}

int main(int argc, char *argv[])
{
  char recv_data[BUFLEN];
  const char *service = "3000";

  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

  MPI_Initialize();

  switch (argc) {
    case 1:
      break;
    case 2:
      service = argv[1];
      break;
    default :
      fprintf(stderr, "usage: udpserver [port]\n");
  }

  if (world_rank == 0){
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
      perror("Socket");
      exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((u_short)atoi(service));
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero),8);


    if (bind(sock,(struct sockaddr *)&server_addr,
          sizeof(struct sockaddr)) == -1)
    {
      perror("Bind");
      exit(1);
    }

    addr_len = sizeof(struct sockaddr);

    if (debug){
      printf("\nUDPServer Waiting for client on port ");
      printf(service);
      printf("\n");
    }
    fflush(stdout);
  }

  // initialize seed for rand()
  srand(time(0));

  run();

  while(1){
    sleep(1);
  }

  MPI_Finalize();
  printf("Should never get here");

  return 0;
}
