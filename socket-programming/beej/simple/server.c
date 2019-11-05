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

void *get_in_addr(struct sockaddr *sa) {
    if(sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
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

    while(1) {
        // accept connections
        int sin_size = sizeof their_addr;
        n = newsockfd = accept(sockfd, (struct sockaddr *) &(their_addr), &sin_size);
        if(n == -1) {
            perror("accept");
            exit(1);
        }

        // print connection
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), s, sizeof s);
        printf("Connected to %s!\n", s);

        if(!fork()) { // child sends msg to client
            close(sockfd);
            n = send(newsockfd, "Hello World!", 13, 0);
            if(n == -1) {
                perror("send");
                exit(1);
            }
            exit(0);
        }
        else { // server listens
            close(newsockfd);
        }
    }

    return 0;
}
