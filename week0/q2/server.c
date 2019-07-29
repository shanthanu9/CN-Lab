#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>    // contains defs of system calls. Used in next header files 
#include <sys/socket.h>   // defs and structures for sockets
#include <netinet/in.h>   // constants and structures for internet domain addresses
#include <time.h>         // to output current date

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

	int sockfd, newsockfd, portno, clilen, n;
	char buffer[BUFFER_SIZE]= {0};
	struct sockaddr_in serv_addr, cli_addr;
	
	// get port number
	portno = atoi(argv[1]);
	
	// creating a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
        exit(1);    
    }
    
    // set server address
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno); // host byte order to network byte order
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    
    // bind socket to server address
    if(bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
        exit(1);
    }
    
    // allows the process to listen from socket
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
   
    printf("Waiting for client to connect...\n");

    // accept a connection (block until any connection occurs)
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if(newsockfd < 0) {
        error("ERROR on accept");
        exit(1);
    }
    
    // connection established with client!
    
    // read from client
    n = read(newsockfd, buffer, BUFFER_SIZE-1);
    if(n < 0) error("ERROR reading from socket");
   
    // output client message
    printf("Client: %s\n", buffer);

    // create date string

    char date[BUFFER_SIZE];
    time_t t = time(NULL);
    struct tm timeinfo = *localtime(&t);
    n = snprintf(date, sizeof(date), "now %d-%d-%d %d:%d:%d", timeinfo.tm_mday, timeinfo.tm_mon, timeinfo.tm_year, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
	
	// send back ACK to client
	n = write(newsockfd, date, sizeof(date));
    if (n < 0) error("ERROR writing to socket");
    
	return 0;
}
