#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include "chat.h"

#define MAX_NUM_CLIENT 10

void *get_in_addr(struct sockaddr *sa) {
    if(sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

struct client {
    int id;
    int sockfd;
} clients[MAX_NUM_CLIENT];

pthread_mutex_t shouting;
int client_id = 0;

void shout(int cli_id, char buffer[], int buffer_len) {
    int msg_len = buffer_len+10;
    char *msg = (char*) malloc(msg_len * sizeof(char)); // "CLIENT: "
    memset(msg, 0, msg_len);
    for(int i = 0; i < client_id; i++) {
        if(i != cli_id) {
            snprintf(msg, msg_len, "CLIENT%d: %s", cli_id, buffer);
            send(clients[i].sockfd, msg, msg_len, 0);
            memset(msg, 0, msg_len);
        }
    }
}

void *hear_all(void *cli) {
    int sockfd = ((struct client*)cli)->sockfd;
    int id = ((struct client*)cli)->id;
    int n;
    char buffer[BUFFER_LEN];
    while(1) {
        n = recv(sockfd, buffer, BUFFER_LEN, 0);
        if(n == -1) {
            perror("recv");
            exit(1);
        }
        pthread_mutex_lock(&shouting);
        shout(id, buffer, strlen(buffer));
        pthread_mutex_unlock(&shouting);
    }
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd;
    struct addrinfo hints, *res;
    struct sockaddr_storage their_addr;
    char s[INET6_ADDRSTRLEN];
    int n;
    int yes = 1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // get addr info
    n = getaddrinfo(NULL, argv[1], &hints, &res);
    if(n != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(n));
		return 1;
    }

    // create socket
    n = sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(n == -1) {
        perror("server: socket");
        exit(1);
    }

    // set socket options
    n = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if(n == -1) {
        perror("setsockopt");
        exit(1);
    }

    // bind the socket
    n = bind(sockfd, (res->ai_addr), res->ai_addrlen);
    if(n == -1) {
        perror("server: bind");
        exit(1);
    }

    freeaddrinfo(res);

    // listen on the socket for connections
    listen(sockfd, 5);
    if(n == -1) {
        perror("listen");
        exit(1);
    }

    printf("Waiting for connections...\n");

    n = pthread_mutex_init(&shouting, 0);
    if(n != 0) {
        perror("mutex");
        exit(1);
    }

    while(1) {
        int sin_size = sizeof their_addr;
        n = newsockfd = accept(sockfd, (struct sockaddr *) &(their_addr), &sin_size);
        if(n == -1) {
            perror("accept");
            exit(1);
        }

        // print connection
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), s, sizeof s);
        printf("Connected to %s!\n", s);

        // server assigns client names
        clients[client_id].sockfd = newsockfd;
        clients[client_id].id = client_id;

        char auth[BUFFER_LEN];
        snprintf(auth, BUFFER_LEN, "You are Client%d\n", client_id);
        send(newsockfd, auth, strlen(auth), 0);

        pthread_t pt_client;
        pthread_create(&pt_client, NULL, hear_all, (void*) &clients[client_id]);

        client_id++;
    }

    pthread_mutex_destroy(&shouting);

    return 0;
}
