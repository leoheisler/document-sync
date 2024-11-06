#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>    
#include <unistd.h>   
#include <sys/socket.h>
#include <netinet/in.h>
#include "fileTransfer.h"
#include "fileManager.h"
#include "serverComManager.h" 
#include "serverStatus.h"

#define PORT 4000

serverStatus bind_server_socket(int* server_socket){
	struct sockaddr_in serv_addr;

	if ((*server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("ERROR opening socket");
		return serverStatus::FAILED_TO_CREATE_SOCKET;
	}
        
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);     
	if (bind(*server_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		cout << ("ERROR on binding");
		return serverStatus::FAILED_TO_BIND_SOCKET;
	} 
		

	return serverStatus::OK;
}

serverStatus bind_client_sockets(int* client_cmd_socket, int* client_upload_socket, int* client_fetch_socket,int* server_socket){
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(struct sockaddr_in);

	if ((*client_cmd_socket = accept(*server_socket, (struct sockaddr *) &cli_addr, &clilen)) == -1) 
			printf("ERROR on accept cmd socket");

	if ((*client_upload_socket = accept(*server_socket, (struct sockaddr *) &cli_addr, &clilen)) == -1) 
			printf("ERROR on accept upload socket");

	if ((*client_fetch_socket = accept(*server_socket, (struct sockaddr *) &cli_addr, &clilen)) == -1) 
			printf("ERROR on accept fetch socket");

	return serverStatus::OK;
		
}


int main(int argc, char *argv[])
{
	//SERVER SOCKETS
	int server_socket, client_cmd_socket, client_upload_socket, client_fetch_socket;
	//SERVER MODULES
	serverComManager serverCommunicationManager;
	fileManager serverFileManager;
	
	//BIND MAIN SOCKET
	serverStatus isBinded = bind_server_socket(&server_socket);
		if(isBinded != serverStatus::OK){
		cout << toString(isBinded);
		return -1;
	}
	
	// LISTEN
	listen(server_socket, 5);
	cout << "================================\n SERVER LISTENING ON PORT 4000\n================================\n";
	//ACCEPT
	isBinded = bind_client_sockets(&client_cmd_socket,&client_fetch_socket,&client_upload_socket,&server_socket);
		if(isBinded != serverStatus::OK){
		cout<<toString(isBinded);
		return -1;
	}
		
	//await for commands
	//serverCommunicationManager.await_command_packet(client_socket);
	
	return 0;
}

