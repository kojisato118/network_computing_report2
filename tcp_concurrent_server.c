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
#include <errno.h>

#define LISTENQ 100
#define MAX_PROTOCOL 5
#define MAX_PROCESS 20
#define BUFF_SIZE 1024

void sig_child(int signo);
void sig_term(int signo); 
void sig_quit(int signo);
void sig_kill(int signo);

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

  int sock[MAX_PROTOCOL];
  int n;
  struct addrinfo hints, *res0, *res;
  int yes = 1;

  char buff[BUFF_SIZE];
  char inbuf[BUFF_SIZE];
  char obuf[BUFF_SIZE];

  signal(SIGCHLD, sig_child); 
  signal(SIGTERM, sig_term);
  signal(SIGQUIT, sig_quit);
  signal(SIGKILL, sig_kill);

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_socktype = SOCK_STREAM;
  getaddrinfo(NULL, "31614", &hints, &res0);

  res = res0;
  n = 0;

  while(res != NULL) {
    getnameinfo(res->ai_addr, res->ai_addrlen, buff, sizeof(buff), NULL, 0, NI_NUMERICHOST);
    printf("IP address : %s¥n", buff);

    sock[n] = socket(res->ai_family, res->ai_socktype, 0);
    if (sock[n] < 0) {
      perror("sock");
      continue;
    }

    setsockopt(sock[n], IPPROTO_IPV6, IPV6_V6ONLY, (char *)&yes, sizeof(yes));
    if(bind(sock[n], res->ai_addr, res->ai_addrlen) < 0){
      perror("bind");
      continue;
    }
    
    if ( (pid = fork() ) == 0) {
      //listen用プロセス
      if(listen(sock[n], LISTENQ) < 0){
        perror("listen");
        continue;
      }

      while(1){
        len = sizeof(client);
        connfd = accept(sock[n], (struct sockaddr *)&client, &len);
        
        if(connfd < 0){
          perror("accept");
          continue;
        }        
        // TODO: 今のままだと無限にプロセス生成になっちゃうから、制限をつける
        if ( (pid2 = fork() ) == 0 ) {
          close(sock[n]);
          printf("accepted connection from %s, port=%d at PIDD=%d\n",
                  inet_ntoa(client.sin_addr), ntohs(client.sin_port), getpid());
          memset(&inbuf, 0, sizeof(inbuf));
          recv(connfd, inbuf, sizeof(inbuf), 0);
          printf("%s", inbuf);
         
          memset(&obuf, 0, sizeof(obuf));
          snprintf(obuf, sizeof(obuf),
                "HTTP/1.0 200 OK\r\n"
                "Content-Type: text/html\r\n"
                 "\r\n"
                 "<font color=red><h1>HELLO</h1></font>\r\n");
          send(connfd, obuf, (int)strlen(obuf), 0);
          
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
      }else{
        perror("pid fork");
      }
    }
    res = res->ai_next;
      
    if(++n == MAX_PROTOCOL) break;
  }


  while(1){
    // listenをforkした子プロセスでやっているため、大元のプロセスが終了するのを防ぐため
  }
  
  return 0;
}

void sig_child(int signo)
{
  printf("accept SIGCHLD at PID: %d\n", getpid());
  int pid, status;
  pid = wait(&status);
  printf("PID: %d, terminated\n", pid);
} 


// TODO: 大元のプロセスをkillした時にプロセスグループを殺したいのだがうまくいかない。
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

void sig_kill(int signo)
{
  printf("accept SIGKILL at PID: %d\n", getpid());
  kill_all_pids();
}

void kill_all_pids()
{
  int status;
  kill(0, SIGKILL); 
  wait(&status);
}
