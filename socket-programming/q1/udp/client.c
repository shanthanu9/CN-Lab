#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>      // for hostent

#define BUFFER_SIZE 256

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]) {
    
    // if no server name and port provided
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    
    int sockfd, portno, n, servlen;
    struct sockaddr_in serv_addr;
    struct hostent *server;       // defines a host computer on the Internet
    char buffer[BUFFER_SIZE] = "I am client";
    
    // creating a socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }
    
    // get server port no
    portno = atoi(argv[2]);
    
    // get server ip address
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    // set server address
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    // serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
  
    // printf("SERVER ADDR = %d\n", serv_addr.sin_addr.s_addr);

    // send message to server (to tell server to send back greeting)
    n = sendto(sockfd, "1", 2, MSG_CONFIRM, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    if(n < 0) {
        error("ERROR sending message to server");
    }

    // recieve greeting from server
    n = recvfrom(sockfd, buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr*) &serv_addr, (socklen_t*) &servlen);
    if(n < 0) {
        error("ERROR recieving greeting from server");
    }

    // output greeting by server
    printf("%s\n", buffer);
   
    // close socket
    close(sockfd);

    return 0;
}
