#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <iostream>

#include "serverComManager.h" 
#include "packet.h"
#include "fileTransfer.h"

#define PORT 4000

// CONSTRUCTOR
serverComManager::serverComManager(/* args */){};

// PRIVATE METHODS

// PUBLIC METHODS
int serverComManager::connectServerToClient(int argc, char* argv[])
{
	int sockfd, newsockfd, n;
	//sockfd is the listening socket for accepting new connections.
	//newsockfd is a separate socket used for actual communication with the connected client.
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	
	// SOCKET
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket");
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);     
    
	// BIND
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		printf("ERROR on binding");
	
	// LISTEN
	listen(sockfd, 5);
	
	// ACCEPT
	clilen = sizeof(struct sockaddr_in);
	if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1) 
		printf("ERROR on accept");
	
	bzero(buffer, 256);
	
	//Packet received_packet = Packet::receive_packet(newsockfd);
	//printf("%s\n", received_packet.getPayload()); // No need for c_str() since it returns char*


	// Receive the file
    std::string output_file_path ="../src/server/syncDir/teste.jpeg";
    FileTransfer::receive_file(output_file_path, newsockfd);

	// Open the received file and print its contents
    std::ifstream file(output_file_path);
    if (!file.is_open()) {
        std::cerr << "ERROR opening file: " << output_file_path << std::endl;
        return -1; // Exit with error code
    }

    std::string line;
    while (std::getline(file, line)) {
        std::cout << line << std::endl; // Print each line to the terminal
    }

    file.close(); // Close the file




	close(newsockfd);
	close(sockfd);
	return 0; 
}