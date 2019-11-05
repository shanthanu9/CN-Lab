#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>    // contains defs of system calls. Used in next header files 
#include <sys/socket.h>   // defs and structures for sockets
#include <netinet/in.h>   // constants and structures for internet domain addresses

#define BUFFER_SIZE 256

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {

    // if no port provided	
	if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

	int sockfd, portno, clilen, n;
	char buffer[BUFFER_SIZE]= "Hello Client!";
	struct sockaddr_in serv_addr, cli_addr;
	
	// get port number
	portno = atoi(argv[1]);
	
	// creating a socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }
    
    // reset client address
    bzero((char *) &cli_addr, sizeof(cli_addr));
    clilen = sizeof(cli_addr);

    // set server address
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno); // host byte order to network byte order
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    
    // bind socket to server address
    if(bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }
  
    char abc[2];
    // recieve message from client (to get client address)
    n = recvfrom(sockfd, abc, 2, MSG_WAITALL, (struct sockaddr*) &cli_addr, &clilen);
    if(n < 0) {
        error("ERROR recieving message from client");
    }

	// send greeting to client client
    n = sendto(sockfd, buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr*) &cli_addr, clilen);
    if(n < 0) {
        error("ERROR sending greeting to client");
    }

    printf("Sent greeting to client!\n");

    // close socket
    close(sockfd);

	return 0;
}
