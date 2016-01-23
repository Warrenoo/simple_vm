#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include"socket.h"

#define MAXLINE 4096
#define PORT 6666

int main() {
  int listenfd, connfd;
  char buff[MAXLINE];
  int n;

  listenfd = open_listenfd(PORT);

  /*挂起服务等待client连接*/
  printf("======waiting for client's request======\n");
  while(1){
    if((connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) == -1){
        printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
        continue;
    }
    /*读数据*/
    while(1){
      n = recv(connfd, buff, MAXLINE, 0);
      buff[n] = '\0';

      if (strcmp(buff, "exit\n") == 0) {
        close(connfd);
        break;
      } else {
        printf("recv msg from client: %s\n", buff);
      }
    }
  }

  close(listenfd);
}
