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
#include "Actor.h"
#include <pthread.h>

int sock;
struct sockaddr_in server_addr , client_addr;
int bytes_read;
socklen_t addr_len;

char recvBuff[BUFLEN];
char PDU_TYPE;
char PDU_ID[ID_LEN + 1];
char PDU_DATA[DATALEN];

std::unordered_map<std::string, Actor> current_actors;
std::unordered_map<std::string, struct sockaddr_in> new_actors;

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

char * SetupPDU(Actor actor){
  char * PDU;
  PDU = (char *) malloc(BUFLEN);
  // fill PDU with null characters
  memset(&PDU[0], 0, sizeof(PDU));

  // assigning PDU type
  PDU[0] = SETUP;

  // assigning ID
  strncpy(PDU + PDU_ID_INDEX, actor.get_id().c_str(), ID_LEN);

  // exchange vision radius
  sprintf(PDU + SETUP_VISION_INDEX,"%d",map.get_total_surroundings_grid_size());

  // exchange step size
  sprintf(PDU + SETUP_STEP_INDEX,"%d",map.get_step_size());

  // current actor row
  sprintf(PDU + SETUP_CURR_ROW_INDEX,"%d",actor.get_position().get_row());

  // current actor col
  sprintf(PDU + SETUP_CURR_COL_INDEX,"%d",actor.get_position().get_col());

  // destination row
  sprintf(PDU + SETUP_DEST_ROW_INDEX,"%d",actor.get_destination().get_row());

  // destination col
  sprintf(PDU + SETUP_DEST_COL_INDEX,"%d",actor.get_destination().get_col());

  return PDU;
}

void AddNewActors(){
  std::string new_actor_id;
  struct sockaddr_in new_actor_address;
  // create a copy of new_actors - this allows new actors
  // to be added to the list while addition is occurring
  std::unordered_map<std::string, struct sockaddr_in> temp(new_actors);
  std::unordered_map<std::string, struct sockaddr_in>::iterator it = temp.begin();
  while(it != temp.end()){
    new_actor_id      = it->first;
    new_actor_address = it->second;
    // create a new Actor object
    Actor new_actor = Actor(new_actor_id, map.RandomEmptyLocation(),
        map.RandomDestination(), new_actor_address);
    // add the new Actor to the map -- should be guaranteed no collision
    map.AddActor(new_actor);
    // add the new Actor to the currently active Actors
    current_actors[new_actor_id] = new_actor;
    // Send a SETUP PDU to the client
    SendPDU(SetupPDU(new_actor), new_actor.get_address());
    // erase the key-value pair in new_actors
    new_actors.erase(new_actor_id);
    it++;
  }
}

void ParseRecvdPDU(){
  bytes_read = recvfrom(sock,recvBuff,BUFLEN,0,
      (struct sockaddr *)&client_addr, &addr_len);
  PDU_TYPE = recvBuff[PDU_TYPE_INDEX];
  switch(PDU_TYPE){
    case REGISTER :
      RegisterNewActor(client_addr);
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

void *MapManager(void *args){
  while(1){
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

  printf("\nUDPServer Waiting for client on port ");
  printf(service);
  fflush(stdout);


  while (1)
  {

    bytes_read = recvfrom(sock,recv_data,BUFLEN,0,
        (struct sockaddr *)&client_addr, &addr_len);


    recv_data[bytes_read] = '\0';

    printf("\n(%s , %d) said : ",inet_ntoa(client_addr.sin_addr),
        ntohs(client_addr.sin_port));
    printf("%s", recv_data);
    fflush(stdout);

  }
  return 0;
}
