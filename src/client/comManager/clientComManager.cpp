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

#include "clientComManager.h" 
#include "commandStatus.h"
#include "packet.h"
#include "fileTransfer.h"

using namespace std;

// CONSTRUCTOR
clientComManager::clientComManager(/* args */){};

// PRIVATE METHODS

// PUBLIC METHODS
int clientComManager::connectClientToServer(int argc, char* argv[])
{
    int sockfd, n, port;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];

	setUserName(argv[1]);
	server = gethostbyname(argv[2]);
    port = atoi(argv[3]);

	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    // SOCKET
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket\n");
    
	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(port);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);     
	
    // CONNECT
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        printf("ERROR connecting\n");

    //Packet test_packet(1,1,1,"Hello World!");
    //test_packet.send_packet(sockfd);


    //SEND THE FILE
    std::string file_path = "../src/client/syncDir/teste.jpeg";
    FileTransfer::send_file(file_path, sockfd);

    
	close(sockfd);
    return 0;
}

// GETTERS & SETTERS
std::string clientComManager::getUserName(){ return this->username; }
void clientComManager::setUserName(std::string username){ this->username = username; }