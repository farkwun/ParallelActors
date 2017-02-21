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

Coordinate current_position;
Coordinate destination;
int vision_grid_size;
int step_size;
std::string id;

std::string GetSetupField(int start, char * fields){
  char field_array[SETUP_FIELD_LEN];
  int end = start + SETUP_FIELD_LEN;

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
  std::cout << "Position : " << current_position.to_str() << std::endl;
  std::cout << "Destination : " << destination.to_str() << std::endl;
}

void ParseServerPDU(char * PDU){
  char PDU_type = PDU[PDU_TYPE_INDEX];

  switch(PDU_type){
    case SETUP :
      id = GetSetupField(PDU_ID_INDEX, PDU);
      vision_grid_size = StoI(GetSetupField(SETUP_VISION_INDEX, PDU));
      step_size = StoI(GetSetupField(SETUP_STEP_INDEX, PDU));
      current_position.set_row(StoI(GetSetupField(SETUP_CURR_ROW_INDEX, PDU)));
      current_position.set_col(StoI(GetSetupField(SETUP_CURR_COL_INDEX, PDU)));
      destination.set_row(StoI(GetSetupField(SETUP_DEST_ROW_INDEX, PDU)));
      destination.set_col(StoI(GetSetupField(SETUP_DEST_COL_INDEX, PDU)));
      break;
    default :
      break;
  }
}

int main(int argc, char *argv[])
{
int sock;
int bytes_read;
struct sockaddr_in server_addr;
struct hostent *host;
const char *input_host = "localhost";
const char *service = "3000";
char recv_data[BUFLEN];
char send_data[BUFLEN];

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


   while (1)
   {

    //printf("Type Something (q or Q to quit):");
    //fgets(send_data, 1024, stdin);
    //
    send_data[0] = REGISTER;

      sendto(sock, send_data, strlen(send_data), 0,
          (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    /*if ((strcmp(send_data , "q") == 0) || strcmp(send_data , "Q") == 0){
       break;
    }
    else{
      sendto(sock, send_data, strlen(send_data), 0,
          (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    }*/

    while(1){
      printf("In Receive loop\n");
      bytes_read = recvfrom(sock, recv_data, BUFLEN, 0,
          (struct sockaddr *)&server_addr, (unsigned int*)sizeof(server_addr));
      ParseServerPDU(recv_data);
    }
   }

}
