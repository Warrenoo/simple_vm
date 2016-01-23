#ifndef __SOCKET_H__
#define __SOCKET_H__

#define SOCKADDR_IN struct sockaddr_in
extern int open_listenfd(int);
extern int open_connectfd(char *, int);
extern int open_socketfd();
extern SOCKADDR_IN mk_sockaddr(char *, int);
#endif
