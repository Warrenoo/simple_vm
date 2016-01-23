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

int main(int argc, char** argv) {
  int sockfd, n;
  char recvline[MAXLINE], sendline[MAXLINE];

  if(argc != 2){
    printf("没有输入<ipaddress>\n");
    exit(0);
  }

  sockfd = open_connectfd(argv[1], PORT);

  while(1) {
    printf("send msg to server: \n");
    fgets(sendline, MAXLINE, stdin);

    if(send(sockfd, sendline, strlen(sendline), 0) < 0){
      printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
      exit(0);
    }

    if (strcmp(sendline, "exit\n") == 0)
      break;
  }

  close(sockfd);
  return 0;
}
