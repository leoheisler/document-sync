#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>    
#include <unistd.h>   
#include <sys/socket.h>
#include <netinet/in.h>

#include "fileManager.h"
#include "serverComManager.h" 
#include "serverStatus.h"

#define PORT 4000

serverStatus bindParentSocket(int* parent_socket){
	struct sockaddr_in serv_addr;

	if ((*parent_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("ERROR opening socket");
		return serverStatus::FAILED_TO_CREATE_SOCKET;
	}
        
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);     
	if (bind(*parent_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		printf("ERROR on binding");
		return serverStatus::FAILED_TO_BIND_SOCKET;
	

	return serverStatus::OK;
}

serverStatus bindClientSocket(int* client_socket,int* parent_socket){
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(struct sockaddr_in);
	if ((*client_socket = accept(*parent_socket, (struct sockaddr *) &cli_addr, &clilen)) == -1) 
			printf("ERROR on accept");
		
}


int main(int argc, char *argv[])
{
	serverComManager serverCommunicationManager;
	fileManager serverFileManager;

	int parent_socket, client_socket;
	//parent_socket is the listening socket for accepting new connections.
	//client_socket is a separate socket used for actual communication with the connected client.

	//prepare server socket
	serverStatus isBinded = bindParentSocket(&parent_socket);
	switch (isBinded)
	{
	case serverStatus::OK:
		// LISTEN
		listen(parent_socket, 5);

		//ACCEPT
		bindClientSocket(&client_socket,&parent_socket);
		
		//await for commands
		serverCommunicationManager.await_command_packet(client_socket);


		break;
	
	default:
		cout << toString(isBinded);
		break;
	}	

	
	close(client_socket);
	close(parent_socket);
	
	return 0;
}

