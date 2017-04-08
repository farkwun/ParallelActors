/* udpclient.c */
// Copied from http://www.pythonprasanna.com/Papers%20and%20Articles/Sockets/udpclient.c
//
// Sourced from http://www.prasannatech.net/2008/07/socket-programming-tutorial.html
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "PDUConstants.h"
#include "NetworkHelpers.h"
#include "Coordinate.h"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

bool debug = false;

int sock;
struct sockaddr_in server_addr;
char recv_data[BUFLEN];
char send_data[BUFLEN];

Coordinate position;
Coordinate destination;
int vision_grid_size;
int step_size;
std::string id;

int sequence = 0;;
bool collided = false;
bool arrived  = false;
bool timeout  = false;
char vision[BUFLEN];

Coordinate next_move;

bool IsRightSequence(char sequence_num){
  if ((int)sequence_num == sequence){
    return true;
  }else{
    return false;
  }
}

bool IsTrueChar(char boolean){
  if (boolean == TRUE){
    return true;
  }else{
    return false;
  }
}

bool CheckSequence(int seq_num){
  // TODO:implement or decide on how sequence check
  // is going to work
  return true;
}

void SetPosFromPDU(char * PDU, int row_index, int col_index, int field_len){
  position.set_row(StoI(GetField(row_index, PDU, field_len)));
  position.set_col(StoI(GetField(col_index, PDU, field_len)));
}

void SetDestFromPDU(char * PDU, int row_index, int col_index, int field_len){
  destination.set_row(StoI(GetField(row_index, PDU, field_len)));
  destination.set_col(StoI(GetField(col_index, PDU, field_len)));
}

void SetVision(char * PDU){
  strncpy(vision, PDU + VISION_GRID_INDEX, vision_grid_size);
}

void PrintAttributes(){
  if (debug) {
    std::cout << std::boolalpha;
    std::cout << "My attributes are... " << std::endl;
    std::cout << "ID : " << id << std::endl;
    std::cout << "Vision grid size : " << vision_grid_size << std::endl;
    std::cout << "Step size : " << step_size << std::endl;
    std::cout << "Position : " << position.to_str() << std::endl;
    std::cout << "Destination : " << destination.to_str() << std::endl;
    std::cout << "Next Move : " << next_move.to_str() << std::endl;
    std::cout << "Sequence : " << sequence << std::endl;
    std::cout << "Arrived - " << arrived << std::endl;
    std::cout << "Collided - " << collided << std::endl;
    std::cout << "Timeout - " << timeout << std::endl;
    std::cout << std::endl;
  }
}

void DecideNextMove(char * vision, Coordinate position){
  // TODO: Implement actual control logic
  // rudimentary control logic -- just moves towards destination
  int diff;
  if (position.get_row() != destination.get_row()){
    diff = position.get_row() - destination.get_row();
    next_move.set_row(position.get_row() - (diff/abs(diff)));
    next_move.set_col(position.get_col());
  }else if (position.get_col() != destination.get_col()){
    diff = position.get_col() - destination.get_col();
    next_move.set_col(position.get_col() - (diff/abs(diff)));
    next_move.set_row(position.get_row());
  }else{
    next_move.set_row(position.get_row());
    next_move.set_col(position.get_col());
  }
}

char * RegisterPDU(){
  char * PDU;

  PDU = (char *) malloc(BUFLEN);
  memset(&PDU[0], 0, sizeof(PDU));

  PDU[PDU_TYPE_INDEX] = REGISTER;

  return PDU;
}

char * MovementPDU(){
  char * PDU;
  PDU = (char *) malloc(BUFLEN);
  memset(&PDU[0], 0, sizeof(PDU));

  PDU[PDU_TYPE_INDEX] = MOVEMENT;

  PDU[MOVE_SEQ_INDEX]  = (char) sequence;

  strncpy(PDU + PDU_ID_INDEX, id.c_str(), ID_LEN);

  sprintf(PDU + MOVE_NEXT_ROW_INDEX, "%d", next_move.get_row());

  sprintf(PDU + MOVE_NEXT_COL_INDEX, "%d", next_move.get_col());

  return PDU;
}

void SendPDU(char * PDU){
  sendto (sock, PDU, BUFLEN, 0,
      (struct sockaddr *)&server_addr, sizeof(server_addr));
  free(PDU);
}

void ParseServerPDU(char * PDU){
  char PDU_type = PDU[PDU_TYPE_INDEX];
  if (debug) {
    PrintPDU(PDU);
  }

  switch(PDU_type){
    case SETUP :
      id = GetField(PDU_ID_INDEX, PDU, SETUP_FIELD_LEN);
      vision_grid_size = StoI(GetField(SETUP_VISION_INDEX, PDU, SETUP_FIELD_LEN));
      step_size = StoI(GetField(SETUP_STEP_INDEX, PDU, SETUP_FIELD_LEN));
      SetPosFromPDU(PDU, SETUP_CURR_ROW_INDEX, SETUP_CURR_COL_INDEX, SETUP_FIELD_LEN);
      SetDestFromPDU(PDU, SETUP_DEST_ROW_INDEX, SETUP_DEST_COL_INDEX, SETUP_FIELD_LEN);
      PrintAttributes();
      break;
    case VISION :
      if(CheckSequence((int)PDU[VISION_SEQ_NUM_INDEX])){
        collided = IsTrueChar(PDU[VISION_COLLIDED_INDEX]);
        arrived  = IsTrueChar(PDU[VISION_ARRIVED_INDEX]);
        timeout  = IsTrueChar(PDU[VISION_TIMEOUT_INDEX]);
        SetPosFromPDU(PDU, VISION_CURR_ROW_INDEX, VISION_CURR_COL_INDEX, VISION_FIELD_LEN);
        SetVision(PDU);
        DecideNextMove(vision, position);
        SendPDU(MovementPDU());
        if(collided || arrived){
          SendPDU(RegisterPDU());
        }
      }
      break;
    default :
      break;
  }
}

int main(int argc, char *argv[])
{
  struct hostent *host;
  const char *input_host = "localhost";
  const char *service = "3000";

  switch (argc) {
    case 1:
      input_host = "localhost";
      break;
    case 3:
      service = argv[2];
      /* FALL THROUGH */
      break;
    case 2:
      input_host = argv[1];
      break;
    default:
      fprintf(stderr, "usage: udpclient [host [port]]\n");
      exit(1);
  }

  host= (struct hostent *) gethostbyname(input_host);


  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
  {
    perror("socket");
    exit(1);
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons((u_short)atoi(service));
  server_addr.sin_addr = *((struct in_addr *)host->h_addr);
  bzero(&(server_addr.sin_zero),8);

  SendPDU(RegisterPDU());

  while(1){
    if (debug) {
      printf("In Receive loop\n");
    }
    recvfrom(sock, recv_data, BUFLEN, 0,
        (struct sockaddr *)&server_addr, (unsigned int*)sizeof(server_addr));
    ParseServerPDU(recv_data);
  }
}
