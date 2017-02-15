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

const static int ID_LEN = 10;
const static int BUFLEN = 1024;

std::string generate_id(){
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

int main(int argc, char *argv[])
{
  int sock;
  int bytes_read;
  socklen_t addr_len;
  char recv_data[BUFLEN];
  const char *service = "3000";
  struct sockaddr_in server_addr , client_addr;

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
