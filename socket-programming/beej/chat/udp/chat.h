#ifndef _INCLUDE_CHAT
#define _INCLUDE_CHAT

#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>

#define TIMEOUT 2
#define BUFFER_LEN 256

// #END signals end of communication.

struct pack_addr {
    int sockfd;
    struct sockaddr* addr;
    int addrlen;
};

void *hear(void *addr_pack);
void *talk(void *addr_pack);
void chat(int sockfd, struct sockaddr *addr, int addrlen, char recv[], int recv_len);

#endif
