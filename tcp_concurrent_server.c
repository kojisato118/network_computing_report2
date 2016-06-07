#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define LISTENQ 100

int main(int argc, char *argv[])
{
  pid_t pid, pid2;
  int listenfd, connfd;
  struct sockaddr_in addr;
  struct sockaddr_in client;
  socklen_t len;
  int cli;
  char *cli_addr;

  int sock[16];
  int n;
  struct addrinfo hints, *res0, *res;
  int yes = 1;

  char buff[256];

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_socktype = SOCK_STREAM;
  getaddrinfo(NULL, "12345", &hints, &res0);

  res = res0;
  n = 0;

  printf("start while!\n");
  while(res != NULL) {
    printf("%d回目\n", n);
    getnameinfo(res->ai_addr, res->ai_addrlen, buff, sizeof(buff), NULL, 0, NI_NUMERICHOST);
    printf("IP address : %s¥n", buff);

    sock[n] = socket(res->ai_family, res->ai_socktype, 0);
    setsockopt(sock[n], IPPROTO_IPV6, IPV6_V6ONLY, (char *)&yes, sizeof(yes));
    bind(sock[n], res->ai_addr, res->ai_addrlen);
    
    if ( (pid = fork() ) == 0) {
      listen(sock[n], LISTENQ);
      len = sizeof(client);
      connfd = accept(sock[n], (struct sockaddr *)&client, &len);
      if ( (pid2 = fork() ) == 0 ) {
        close(sock[n]);
        printf("accepted connection from %s, port=%d\n",
                inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        write(connfd, "HELLO\n", 6);
        
        close(connfd); exit(0);
      }
      close(connfd);
    }
    res = res->ai_next;
      
    if(++n == 16) break;
  }

//  freeaddrinfo(res0);
//  
//  for(n = 0; n < 16; n++){
//    close(sock[n]);
//  }
  
  return 0;
}
