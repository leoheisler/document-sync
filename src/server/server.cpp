#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>    
#include <unistd.h>   
#include <sys/socket.h>
#include <netinet/in.h>
#include "fileTransfer.h"
#include <serverComManager.h>
#include <thread>
#include <mutex>


#define PORT 4000
std::mutex connect_hand;

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

void connection_handler(int server_socket, int first_contact_socket,ClientList* client_device_list){
	//create a server comm manager for the connection
	serverComManager server_comm(client_device_list);

	//send a handshake to client
	Packet handshake_packet(first_contact_socket);
	handshake_packet.send_packet(first_contact_socket);
	
	//connect the other sockets
	server_comm.bind_client_sockets(server_socket,first_contact_socket);
	
	//let other threads be created
	connect_hand.unlock();
	
	//command loop
	server_comm.await_command_packet();
	cout << "liberating memory and ending thread for user: " + server_comm.get_username() << std::endl;
}


int main(int argc, char *argv[])
{
	//SERVER SOCKETS
	int server_socket;
	int first_contact_socket;
	struct sockaddr_in client_address;
	socklen_t client_len = sizeof(struct sockaddr_in);

	// Linked list to store client infos 
    ClientList client_device_list;
	ServerList server_list;

	//BIND MAIN SOCKET
	serverStatus isBinded = bind_server_socket(&server_socket);
		if(isBinded != serverStatus::OK){
		cout << to_string(isBinded);
		return -1;
	}
	
	// LISTEN
	listen(server_socket, 6);
	cout << "================================\n SERVER LISTENING ON PORT 4000\n================================\n";
	
	// loop de criação de threads
	while(true){
		//pega o mutex para si, não deixando o while seguir aceitando outros clientes
		connect_hand.lock();

		//cria uma thread pra primeira conexão do cliente
		first_contact_socket = accept(server_socket,(struct sockaddr*)&client_address,&client_len);
		if(first_contact_socket >= 0){
			std::thread t(connection_handler,server_socket,first_contact_socket,&client_device_list);
			t.detach();
		}
		
	}

	return 0;
}

