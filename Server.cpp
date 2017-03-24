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

bool debug = false;

int sock;
struct sockaddr_in server_addr , client_addr;
int bytes_read;
socklen_t addr_len;

pthread_t receive_manager_id;
pthread_t map_manager_id;

char recvBuff[BUFLEN];
char PDU_TYPE;
char PDU_ID[ID_LEN + 1];
char PDU_DATA[DATALEN];

std::unordered_map<std::string, Actor> current_actors;
std::unordered_map<std::string, struct sockaddr_in> new_actors;
std::unordered_map<std::string, struct sockaddr_in> dead_actors;

Map map;

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
}

void UpdateActorMove(char * PDU, struct sockaddr_in address){
  std::string actor_id;
  Coordinate new_move;
  Actor * actor;

  actor_id = GetField(PDU_ID_INDEX, PDU, ID_LEN);

  if (current_actors.count(actor_id) == 0){
    std::cout << "NO such actor with id = " << actor_id << " was found" << std::endl;
    return;
  }

  actor = &current_actors.at(actor_id);

  if (actor->get_address().sin_addr.s_addr == address.sin_addr.s_addr){
    actor->set_timeout(false);

    new_move.set_row(StoI(GetField(MOVE_NEXT_ROW_INDEX, PDU, MOVE_FIELD_LEN)));
    new_move.set_col(StoI(GetField(MOVE_NEXT_COL_INDEX, PDU, MOVE_FIELD_LEN)));

    actor->set_next_move(new_move);
  }
}

void RegisterNewActor(struct sockaddr_in address){
  std::string new_id;
  bool already_exists = true;

  while(already_exists){
    new_id = GenerateID();
    if (current_actors.count(new_id) == 0
        && new_actors.count(new_id)  == 0){
      already_exists = false;
    }
  }

  new_actors[new_id] = address;
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

  sprintf(PDU + SETUP_CURR_ROW_INDEX,"%d",actor.get_position().get_row());

  sprintf(PDU + SETUP_CURR_COL_INDEX,"%d",actor.get_position().get_col());

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
  vision_grid = (char *) malloc(BUFLEN);

  // fill PDU and vision_grid with null characters
  for (int i = 0; i < BUFLEN; i++){
    PDU[i] = '\0';
    vision_grid[i] = '\0';
  }

  vision_grid = Unroll2DVector(map.GetSurroundings(actor.get_position()));

  PDU[PDU_TYPE_INDEX] = VISION;
  PDU[VISION_COLLIDED_INDEX] = BooltoChar(actor.get_collided());
  PDU[VISION_ARRIVED_INDEX]  = BooltoChar(actor.get_arrived());
  PDU[VISION_TIMEOUT_INDEX]  = BooltoChar(actor.get_timeout());

  sprintf(PDU + VISION_CURR_ROW_INDEX,"%d",actor.get_position().get_row());

  sprintf(PDU + VISION_CURR_COL_INDEX,"%d",actor.get_position().get_col());

  for (int i = VISION_GRID_INDEX; i < VISION_GRID_INDEX + strlen(vision_grid); i++){
    PDU[i] = vision_grid[i-VISION_GRID_INDEX];
  }

  return PDU;
}

void InitializeMap(){
  map.InitializeActorDimensions();
  map.InitializeVision();
  map.InitializeStepSize();
}

// moves waiting actors to current list
void AddNewActors(){
  if (new_actors.empty()){
    return;
  }

  std::string new_actor_id;
  struct sockaddr_in new_actor_address;
  std::unordered_map<std::string, struct sockaddr_in> temp(new_actors);
  std::unordered_map<std::string, struct sockaddr_in>::iterator it;

  it = temp.begin();

  while(it != temp.end()){
    new_actor_id      = it->first;
    new_actor_address = it->second;

    Actor new_actor(new_actor_id, map.RandomEmptyLocation(),
        map.RandomDestination(), new_actor_address);

    map.AddActor(new_actor);

    current_actors[new_actor_id] = new_actor;

    SendPDU(SetupPDU(new_actor), new_actor.get_address());

    SendPDU(VisionPDU(new_actor), new_actor.get_address());

    new_actors.erase(new_actor_id);

    it++;
  }
}

void ParseRecvdPDU(){
  bytes_read = recvfrom(sock,recvBuff,BUFLEN,0,
      (struct sockaddr *)&client_addr, &addr_len);
  PDU_TYPE = recvBuff[PDU_TYPE_INDEX];
  if (debug){
    PrintPDU(recvBuff);
  }
  switch(PDU_TYPE){
    case REGISTER :
      RegisterNewActor(client_addr);
      break;
    case MOVEMENT :
      UpdateActorMove(recvBuff, client_addr);
      break;
    default:
      break;
  }
}

void *ReceiveManager(void *args){
  while(1){
    ParseRecvdPDU();
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
    dead_actors[actor.get_id()] = actor.get_address();
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
    dead_actors[actor.get_id()] = actor.get_address();
  }
  return actor;
}

Actor SendUpdate(Actor actor){
  SendPDU(VisionPDU(actor), actor.get_address());
  return actor;
}

void DeleteDeadActors(){
  std::string dead_actor_id;

  if (dead_actors.empty()){
    return;
  }

  std::unordered_map<std::string, struct sockaddr_in> temp(dead_actors);
  std::unordered_map<std::string, struct sockaddr_in>::iterator it;

  it = temp.begin();

  while(it != temp.end()){
    dead_actor_id = it->first;
    if (current_actors.count(dead_actor_id) != 0){
      map.ClearActor(current_actors.at(dead_actor_id));
      current_actors.erase(dead_actor_id);
      dead_actors.erase(dead_actor_id);
    }
    it++;
  }
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
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

int main(int argc, char *argv[])
{
  char recv_data[BUFLEN];
  const char *service = "3000";

  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("Socket");
    exit(1);
  }

  switch (argc) {
    case 1:
      break;
    case 2:
      service = argv[1];
      break;
    default :
      fprintf(stderr, "usage: udpserver [port]\n");
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

  // initialize seed for rand()
  srand(time(0));

  if (debug){
    printf("\nUDPServer Waiting for client on port ");
    printf(service);
    printf("\n");
  }
  fflush(stdout);

  InitializeMap();

  pthread_create(&receive_manager_id, NULL, &ReceiveManager, NULL);
  pthread_create(&map_manager_id, NULL, &MapManager, NULL);

  while(1){
    sleep(1);
  }

  printf("Should never get here");

  return 0;
}
