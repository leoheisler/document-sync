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
void clientComManager::get_sync_dir(int socket)
{
    // Send packet signaling server to execute get_sync_dir with client info (username and socket)
    string client_info = (get_username() + "\n" + to_string(socket));
    Packet get_sync_command = Packet(Packet::CMD_PACKET, Command::GET_SYNC_DIR, 1, client_info.c_str(), client_info.length());
    get_sync_command.send_packet(socket);
}

// PUBLIC METHODS
int clientComManager::connect_client_to_server(int argc, char* argv[])
{
    int sockfd, n, port;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];

	set_username(argv[1]);
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

    //SEND THE FILE
    // std::string file_path = "../src/client/syncDir/test.mp4";
    // FileTransfer::send_file(file_path, sockfd);
    get_sync_dir(sockfd);

	close(sockfd);
    return 0;
}

// GETTERS & SETTERS
std::string clientComManager::get_username(){ return this->username; }
void clientComManager::set_username(std::string username){ this->username = username; }