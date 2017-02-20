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
#include <iostream>

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
      for (int i = 0; i < BUFLEN; i++){
        if (recv_data[i] == '\0'){
        std::cout << ' ';
        }else{
        std::cout << recv_data[i];
        }
      }
      std::cout << std::endl;
    }
   }

}
