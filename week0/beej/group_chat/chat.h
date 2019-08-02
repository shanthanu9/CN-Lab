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

// client side
void *hear(void *sock_fd);
void *talk(void *sock_fd);
void chat(int sockfd);

#endif
