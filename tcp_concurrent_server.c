#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define LISTENQ 100
#define MAX_PROCESS 16

void sig_child(int signo);
void sig_term(int signo); 
void sig_quit(int signo);

int pids[MAX_PROCESS];
void init_pids();
void kill_all_pids();

int main(int argc, char *argv[])
{
  pid_t pid, pid2;
  int listenfd, connfd;
  struct sockaddr_in addr;
  struct sockaddr_in client;
  socklen_t len;
  int cli;
  char *cli_addr;

  int sock[MAX_PROCESS];
  int n;
  struct addrinfo hints, *res0, *res;
  int yes = 1;

  char buff[256];
  
  signal(SIGCHLD, sig_child); 
  signal(SIGTERM, sig_term);
  signal(SIGQUIT, sig_quit);

  init_pids();

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
      //listen用プロセス
      listen(sock[n], LISTENQ);
      while(1){
        len = sizeof(client);
        connfd = accept(sock[n], (struct sockaddr *)&client, &len);
        sleep(10);
        if ( (pid2 = fork() ) == 0 ) {
          close(sock[n]);
          printf("accepted connection from %s, port=%d at PIDD=%d\n",
                  inet_ntoa(client.sin_addr), ntohs(client.sin_port), getpid());
          write(connfd, "HELLO\n", 6);
          
          close(connfd); exit(0);
        }else{
           if (pid2 != -1) {
             // listen用プロセス
           } else {
             perror("pid2 fork");
           }
        }
        close(connfd);
      }
    }else{
      if(pid != -1){
        //大元のプロセス
        pids[n] = pid;
      }else{
        perror("pid fork");
      }
    }
    res = res->ai_next;
      
    if(++n == MAX_PROCESS) break;
  }


  while(1){
    // listenをforkした子プロセスでやっているため、大元のプロセスが終了するのを防ぐため
  }
//  freeaddrinfo(res0);
//  
//  for(n = 0; n < 16; n++){
//    close(sock[n]);
//  }
  
  return 0;
}

void sig_child(int signo)
{
  printf("accept SIGCHLD at PID: %d\n", getpid());
  int pid, status;
  pid = wait(&status);
  printf("PID: %d, terminated\n", pid);
} 

void sig_term(int signo)
{
  printf("accept SIGTERM at PID: %d\n", getpid());
  kill_all_pids();
}

void sig_quit(int signo)
{
  printf("accept SIGQUIT at PID: %d\n", getpid());
  kill_all_pids();
}

void init_pids()
{
  int i;

  for(i = 0; i < MAX_PROCESS; i++){
    pids[i] = -1;
  }
}

void kill_all_pids()
{
  int i, pid, status;

  for(i = 0; i < MAX_PROCESS; i++){
    if(pids[i] > 0){
      kill(pids[i], SIGTERM);
      pids[i] = -1;
      pid = wait(&status);
      printf("PID: %d, terminated\n", pid);
    }
  }
 
}
