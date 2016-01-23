#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include"socket.h"

/*创建socket监听描述符 server*/
int open_listenfd(int port) {
  int listenfd;
  SOCKADDR_IN addr;

  listenfd = open_socketfd();

  addr = mk_sockaddr(NULL, port);

  /*将socket描述符与addr进行绑定*/
  if(bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1){
    printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
    exit(0);
  }

  if(listen(listenfd, 10) == -1){
    printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
    exit(0);
  }

  return listenfd;
}

/*创建socket连接描述符 client*/
int open_connectfd(char *ip, int port) {
  int connectfd;
  SOCKADDR_IN addr;

  /*创建一个socket描述符*/
  connectfd = open_socketfd();

  /*构建addr*/
  addr = mk_sockaddr(ip, port);

  if(connect(connectfd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
    printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
    exit(0);
  }

  return connectfd;
}

/*创建socket描述符*/
int open_socketfd() {
  int socketfd;
  if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
    printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
    exit(0);
  }

  return socketfd;
}

/*创建socket连接地址*/
SOCKADDR_IN mk_sockaddr(char *ip, int port) {
  SOCKADDR_IN addr;

  memset(&addr, 0, sizeof(addr));

  /*如果ip为NULL,表示为listen addr*/
  if (ip == (char *)NULL) {
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
  } else {
    if(inet_pton(AF_INET, ip, &addr.sin_addr) <= 0){
      printf("inet_pton error for %s\n", ip);
      exit(0);
    }
  }
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  return addr;
}
