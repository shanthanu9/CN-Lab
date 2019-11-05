#include "chat.h"

int END_COM = 0;
int TALK = 1, HEAR = 1;
char RECV_NAME[BUFFER_LEN];

void *hear(void *addr_pack) {
    int n;
    int sockfd = ((struct pack_addr *)addr_pack)->sockfd;
    struct sockaddr* addr = ((struct pack_addr *)addr_pack)->addr;
    int addrlen = ((struct pack_addr *)addr_pack)->addrlen;
    char buffer[BUFFER_LEN];
    while(END_COM == 0) {
        n = recvfrom(sockfd, buffer, BUFFER_LEN, 0, addr, &addrlen);
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

void *talk(void *addr_pack) {
    int n;
    int sockfd = ((struct pack_addr *)addr_pack)->sockfd;
    struct sockaddr* addr = ((struct pack_addr *)addr_pack)->addr;
    int addrlen = ((struct pack_addr *)addr_pack)->addrlen;
    char buffer[BUFFER_LEN];
    while(END_COM == 0) {
        fgets(buffer, BUFFER_LEN, stdin);
        n = sendto(sockfd, buffer, BUFFER_LEN, 0, addr, addrlen);
        if(n == -1) {
            perror("send");
        }
        if(strncmp(buffer, "#END", 4) == 0) {
            END_COM = 1;
        }
    }
    TALK = 0;
}

void chat(int sockfd, struct sockaddr* addr, int addrlen, char recv[], int recv_len) {

    strncpy(RECV_NAME, recv, recv_len);

    pthread_t pt_hear, pt_talk;

    struct pack_addr a = {sockfd, addr, addrlen};

    pthread_create(&pt_hear, NULL, hear, (void*) &a);
    pthread_create(&pt_talk, NULL, talk, (void*) &a);

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
