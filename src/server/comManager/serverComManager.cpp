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
#include "serverFileManager.h"




#define PORT 4000

// CONSTRUCTOR
serverComManager::serverComManager(ClientList* client_list){ this->client_list = client_list;};

// PRIVATE METHODS

void serverComManager::start_communications()
{
	Packet starter_packet = Packet::receive_packet(this->client_cmd_socket);
	char* payload = starter_packet.get_payload();
	std::string file_path;

	if(payload != nullptr){
		// Extract username and client socket from packet payload
		std::string username = strtok(payload, "\n");
		this->username = username;

		//try to add client to device list
		try
		{
			this->client_list->add_device(username,this->client_cmd_socket);
			this->file_manager.create_server_sync_dir(username);
			this->client_list->display_clients();
			
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

// This is the interface that will get commands from user and delegate through different methods
void serverComManager::await_command_packet()
{
	serverFileManager file_manager; 
	while(true){
		// Wait to receive a command packet from client
		Packet command_packet = Packet::receive_packet(this->client_cmd_socket);
		FileTransfer sender_reciever;
		// Determine what to do based on the command packet received
		switch(command_packet.get_seqn()){
			case Command::GET_SYNC_DIR:
				std::vector<std::string> paths = file_manager.get_sync_dir_paths(this->username);
				int total_paths = paths.size();
				for(size_t i = 0; i < total_paths; ++i) {
						// Construct the packet payload string
						std::string payload = paths[i] + "\n" + std::to_string(total_paths) + "\n" + std::to_string(i);

						// Create a packet for sending
						Packet get_sync_command(Packet::DATA_PACKET, 1, 1, payload.c_str(), payload.size());

						// Send the packet
						get_sync_command.send_packet(this->client_fetch_socket);

						// Optional logging
						std::cout << "Sent path: " << paths[i] << std::endl;

						// Send the file using sender_reciever
						sender_reciever.send_file(paths[i], this->client_fetch_socket);
				}
			break;
		}
	}
}

serverStatus serverComManager::bind_client_sockets(int server_socket, int first_comm_socket){
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(struct sockaddr_in);

	this->client_cmd_socket = first_comm_socket;

    if ((this->client_upload_socket = accept(server_socket, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
        printf("ERROR on accept upload socket\n");
        return serverStatus::FAILED_TO_ACCEPT_UPLOAD_SOCKET; // Retorna o erro apropriado
    }

    if ((this->client_fetch_socket = accept(server_socket, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
        printf("ERROR on accept fetch socket\n");
        return serverStatus::FAILED_TO_ACCEPT_FETCH_SOCKET; // Retorna o erro apropriado
    }
	start_communications();

	return serverStatus::OK;
		
}
