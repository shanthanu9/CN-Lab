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
    
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;       // defines a host computer on the Internet
    char buffer[BUFFER_SIZE] = {0};
    
    // creating a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
        exit(1);
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
    serv_addr.sin_port = htons(portno);
    
    // connect to server
    if(connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
        exit(1);
    }
    
    // connection established to server!
    
    // send message to server
    printf("Please enter the message: ");
    fgets(buffer, BUFFER_SIZE-1, stdin);
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    
    // get ACK from server
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0) 
         error("ERROR reading from socket");
         
    // output ACK
    printf("%s\n", buffer);
           
    return 0;
}
