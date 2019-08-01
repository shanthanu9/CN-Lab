#include "chat.h"

int END_COM = 0;
int TALK = 1, HEAR = 1;
char RECV_NAME[BUFFER_LEN];

void *hear(void *sock_fd) {
    int n;
    int sockfd = *((int *)sock_fd);
    char buffer[BUFFER_LEN];
    while(END_COM == 0) {
        n = recv(sockfd, buffer, BUFFER_LEN, 0);
        if(n == -1) {
            perror("recv");
        }
        if(strncmp(buffer, "#END", 4) == 0) {
            END_COM = 1;
        }
        printf("%s: %s", RECV_NAME, buffer);
    }
    HEAR = 0;
}

void *talk(void *sock_fd) {
    int n;
    int sockfd = *((int *)sock_fd);
    char buffer[BUFFER_LEN];
    while(END_COM == 0) {
        fgets(buffer, BUFFER_LEN, stdin);
        n = send(sockfd, buffer, BUFFER_LEN, 0);
        if(n == -1) {
            perror("send");
        }
        if(strncmp(buffer, "#END", 4) == 0) {
            END_COM = 1;
        }
    }
    TALK = 0;
}

void chat(int sockfd, char recv[], int recv_len) {

    strncpy(RECV_NAME, recv, recv_len);

    pthread_t pt_hear, pt_talk;

    pthread_create(&pt_hear, NULL, hear, (void*) &sockfd);
    pthread_create(&pt_talk, NULL, talk, (void*) &sockfd);

    while(1) {
        if(!HEAR && TALK) {
            pthread_kill(pt_talk, SIGTERM);
            break;
        }
        if(!TALK && HEAR) {
            pthread_kill(pt_hear, SIGTERM);
            break;
        }
        sleep(TIMEOUT); // check every TIMEOUT seconds
    }

    pthread_join(pt_talk, NULL);
    pthread_join(pt_hear, NULL);

}
