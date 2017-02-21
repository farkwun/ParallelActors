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
#include "Coordinate.h"
#include <iostream>
#include <string>

int sock;
struct sockaddr_in server_addr;
char recv_data[BUFLEN];
char send_data[BUFLEN];

Coordinate position;
Coordinate destination;
int vision_grid_size;
int step_size;
std::string id;

int sequence;
bool collided = false;
bool arrived  = false;
bool timeout  = false;
char * vision;

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

std::string GetField(int start, char * fields, int field_len){
  char field_array[field_len];
  int end = start + field_len;

  std::copy(fields + start, fields + end, field_array);

  std::string field(field_array);

  return field;
}

int StoI(std::string number){
  return std::stoi(number, nullptr, 10);
}

void PrintAttributes(){
  std::cout << "My attributes are... " << std::endl;
  std::cout << "ID : " << id << std::endl;
  std::cout << "Vision grid size : " << vision_grid_size << std::endl;
  std::cout << "Step size : " << step_size << std::endl;
  std::cout << "Position : " << position.to_str() << std::endl;
  std::cout << "Destination : " << destination.to_str() << std::endl;
}

char * RegisterPDU(){
  char * PDU;

  PDU = (char *) malloc(BUFLEN);
  memset(&PDU[0], 0, sizeof(PDU));

  PDU[0] = REGISTER;

  return PDU;
}

void SendPDU(char * PDU){
  sendto (sock, PDU, BUFLEN, 0,
      (struct sockaddr *)&server_addr, sizeof(server_addr));
}

void ParseServerPDU(char * PDU){
  char PDU_type = PDU[PDU_TYPE_INDEX];

  switch(PDU_type){
    case SETUP :
      id = GetField(PDU_ID_INDEX, PDU, SETUP_FIELD_LEN);
      vision_grid_size = StoI(GetField(SETUP_VISION_INDEX, PDU, SETUP_FIELD_LEN));
      step_size = StoI(GetField(SETUP_STEP_INDEX, PDU, SETUP_FIELD_LEN));
      position.set_row(StoI(GetField(SETUP_CURR_ROW_INDEX, PDU, SETUP_FIELD_LEN)));
      position.set_col(StoI(GetField(SETUP_CURR_COL_INDEX, PDU, SETUP_FIELD_LEN)));
      destination.set_row(StoI(GetField(SETUP_DEST_ROW_INDEX, PDU, SETUP_FIELD_LEN)));
      destination.set_col(StoI(GetField(SETUP_DEST_COL_INDEX, PDU, SETUP_FIELD_LEN)));
      break;
    case VISION :
      collided = IsTrueChar(PDU[VISION_COLLIDED_INDEX]);
      arrived  = IsTrueChar(PDU[VISION_ARRIVED_INDEX]);
      timeout  = IsTrueChar(PDU[VISION_TIMEOUT_INDEX]);
      position.set_row(StoI(GetField(VISION_CURR_ROW_INDEX, PDU, SETUP_FIELD_LEN)));
      position.set_col(StoI(GetField(VISION_CURR_COL_INDEX, PDU, SETUP_FIELD_LEN)));
      std::copy(PDU + VISION_GRID_INDEX,
          PDU + VISION_GRID_INDEX + vision_grid_size, vision);
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
    printf("In Receive loop\n");
    recvfrom(sock, recv_data, BUFLEN, 0,
        (struct sockaddr *)&server_addr, (unsigned int*)sizeof(server_addr));
    ParseServerPDU(recv_data);
  }
}
