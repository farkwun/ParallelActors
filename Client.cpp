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

int main(int argc, char *argv[])
{
int sock;
struct sockaddr_in server_addr;
struct hostent *host;
const char *input_host = "localhost";
const char *service = "3000";
char send_data[1024];

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

    printf("Type Something (q or Q to quit):");
    gets(send_data);

    if ((strcmp(send_data , "q") == 0) || strcmp(send_data , "Q") == 0)
       break;

    else
       sendto(sock, send_data, strlen(send_data), 0,
              (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
     
   }

}
