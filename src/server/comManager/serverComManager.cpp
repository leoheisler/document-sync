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
#include "commandStatus.h"
#include "fileTransfer.h"
#include "packet.h"


#define PORT 4000

// CONSTRUCTOR
serverComManager::serverComManager(){};

// PRIVATE METHODS

void serverComManager::create_sync_dir(Packet sync_dir_packet, ClientList* client_device_list)
{
	char* payload = sync_dir_packet.get_payload();

	if(payload != nullptr){
		// Extract username and client socket from packet payload
		std::string username = strtok(payload, "\n");
		this->username = username;

		//try to add client to device list
		try
		{
			client_device_list->add_device(username,this->client_cmd_socket);
			client_device_list->display_clients();


		    std::string file_path1 = "../src/server/userDirectories/socket_cmd.txt";
    		FileTransfer::send_file(file_path1, this->client_cmd_socket);

			std::string file_path2 = "../src/server/userDirectories/socket_upload.txt";
    		FileTransfer::send_file(file_path2, this->client_upload_socket);

		    std::string file_path3 = "../src/server/userDirectories/socket_fetch.txt";
    		FileTransfer::send_file(file_path3, this->client_fetch_socket);

		}
		//if couldnt connect, send and error packet
		catch(const std::string& e)
		{
			std::cout<< e << '\n';
			Packet error_packet = Packet();
			error_packet.send_packet(this->client_cmd_socket);
		}
	}
}

// PUBLIC METHODS
void serverComManager::await_command_packet(ClientList* client_device_list)
{
	while(true){
		// Wait to receive a command packet from client
		Packet command_packet = Packet::receive_packet(this->client_cmd_socket);

		// Determine what to do based on the command packet received
		switch(command_packet.get_seqn()){
			case Command::GET_SYNC_DIR:
				cout << "Received get_sync_dir packet" << endl;
				create_sync_dir(command_packet,client_device_list);
				break;
	
		}
	}
}

serverStatus serverComManager::bind_client_sockets(int* server_socket){
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(struct sockaddr_in);

	if ((this->client_cmd_socket = accept(*server_socket, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
        printf("ERROR on accept cmd socket\n");
        return serverStatus::FAILED_TO_ACCEPT_CMD_SOCKET; // Retorna o erro apropriado
    }

    if ((this->client_upload_socket = accept(*server_socket, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
        printf("ERROR on accept upload socket\n");
        return serverStatus::FAILED_TO_ACCEPT_UPLOAD_SOCKET; // Retorna o erro apropriado
    }

    if ((this->client_fetch_socket = accept(*server_socket, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
        printf("ERROR on accept fetch socket\n");
        return serverStatus::FAILED_TO_ACCEPT_FETCH_SOCKET; // Retorna o erro apropriado
    }

	return serverStatus::OK;
		
}
