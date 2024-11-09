#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>    
#include <unistd.h>   
#include <sys/socket.h>
#include <netinet/in.h>
#include "fileTransfer.h"
#include <serverComManager.h>


#define PORT 4000

serverStatus bind_server_socket(int* server_socket){

	struct sockaddr_in serv_addr;

	if ((*server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("ERROR opening socket\n");
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




int main(int argc, char *argv[])
{
	//SERVER SOCKETS
	int server_socket;
	// Linked list to store client infos 
    ClientList client_device_list;
	// Handler of communications bewteen a single client and server
	serverComManager server_com_manager(&client_device_list);

	//BIND MAIN SOCKET
	serverStatus isBinded = bind_server_socket(&server_socket);
		if(isBinded != serverStatus::OK){
		cout << to_string(isBinded);
		return -1;
	}
	
	// LISTEN
	listen(server_socket, 6);
	cout << "================================\n SERVER LISTENING ON PORT 4000\n================================\n";
	
	//ACCEPT
	/* AQUI VAI FICAR O LOOP DE CRIAR NOVAS THREADS*/
	isBinded = server_com_manager.bind_client_sockets(&server_socket);
		if(isBinded != serverStatus::OK){
		cout << to_string(isBinded);
		return -1;
	}
		
	//await for commands
	server_com_manager.await_command_packet();
	
	return 0;
}

