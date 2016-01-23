#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <sys/select.h>
#include "socket.h"

#define MAXLINE 4096
#define PORT 6666
#define FD_SETSIZE 100

typedef struct pool {
  fd_set read_set; /*需查询可读操作的fd集合*/
  fd_set ready_set;/*需查询可读操作的fd集合(作为传递给select的变量)*/
  int maxfd; /*read_set 中的最大fd*/
  int nready; /*当前可读的fd*/
  int maxi; /*clientfd数组的大小*/
  int clientfd[FD_SETSIZE];
} POOL;

void init_pool(int listenfd, POOL *p) {
  int i;
  p->maxi = -1;

  for(i=0; i<FD_SETSIZE; i++)
    p->clientfd[i] = -1;

  FD_ZERO(&p->read_set);
  FD_SET(listenfd, &p->read_set);
  p->maxfd = listenfd;
}

/*插入客户端fd到pool*/
void add_client(int connfd, POOL *p) {
  int i;
  p->nready--;

  for(i=0; i<FD_SETSIZE; i++) {

    if (p->clientfd[i] < 0) {
      /*插入*/
      p->clientfd[i] = connfd;
      FD_SET(connfd, &p->read_set);

      /*reset最大值*/
      if(connfd > p->maxfd)
        p->maxfd = connfd;

      if(i > p->maxi)
        p->maxi = i;

      break;
    }
  }

  /*如果没有存放空间了*/
  if (i==FD_SETSIZE){
    printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
  }
}

void deal_msg(POOL *p) {
  int i, connfd, n;
  char buff[MAXLINE];

  for (i=0; (i<=p->maxi) && (p->nready > 0); i++) {
    connfd = p->clientfd[i];

    if((connfd > 0) && (FD_ISSET(connfd, &p->ready_set))) {
      p->nready--;

      n = recv(connfd, buff, MAXLINE, 0);
      buff[n] = '\0';

      if (strcmp(buff, "exit\n") == 0) {
        printf("client%d exit!!\n", i);
        close(connfd);
        FD_CLR(connfd, &p->read_set);
        p->clientfd[i] = -1;
      } else {
        printf("recv msg from client%d: %s\n", i, buff);
      }

    }
  }
}

int main() {
  int listenfd, connfd;
  POOL pool;

  listenfd = open_listenfd(PORT);
  init_pool(listenfd, &pool);

  /*挂起服务等待client连接*/
  printf("======waiting for client's request======\n");
  while(1){

    pool.ready_set = pool.read_set;
    pool.nready = select(pool.maxfd+1, &pool.ready_set, NULL, NULL, NULL);

    /*如果获取到的可读描述符中存在listenfd，说明有新的连接请求*/
    if (FD_ISSET(listenfd, &pool.ready_set)) {
      /*添加新的连接fd到pool*/
      if((connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) == -1){
          printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
      } else {
        add_client(connfd, &pool);
      }
    }

    /*处理消息*/
    deal_msg(&pool);
  }

  close(listenfd);
}
