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
  pid_t pid;
  int listenfd, connfd;
  struct sockaddr_in addr;
  struct sockaddr_in client;
  socklen_t len;
  int cli;
  char *cli_addr;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(12345);
  addr.sin_addr.s_addr = INADDR_ANY;
  bind(listenfd, (struct sockaddr *)&addr, sizeof(addr));
  listen(listenfd, LISTENQ);

  for ( ; ; ) {
    len = sizeof(client);
    connfd = accept(listenfd, (struct sockaddr *)&client, &len);
    if ( (pid = fork() ) == 0 ) {
      close(listenfd);
      printf("accepted connection from %s, port=%d\n",
              inet_ntoa(client.sin_addr), ntohs(client.sin_port));
      write(connfd, "HELLO\n", 6);
      
      close(connfd); exit(0);
    }
    close(connfd);
  }
//  while (1) {
//    len = sizeof(client);
//    cli = accept(sock, (struct sockaddr *)&client, &len);
//    printf("accepted connection from %s, port=%d\n",
//              inet_ntoa(client.sin_addr), ntohs(client.sin_port));
//
//    write(cli, "HELLO\n", 6);
// 
//    close(cli);
//  }

  close(listenfd);

  return 0;
}
