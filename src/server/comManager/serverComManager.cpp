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
#include <mutex>




#define PORT 4000
std::mutex dell_user;
// CONSTRUCTOR
serverComManager::serverComManager(ClientList* client_list){ this->client_list = client_list;};

// PRIVATE METHODS
void serverComManager::get_sync_dir()
{
	cout << "received get sync dir command from user: " + this->username <<std::endl;
	
	std::vector<std::string> paths = serverFileManager::get_sync_dir_paths(this->username);
	int total_paths = paths.size();
	if(total_paths == 0){
		// If user sync dir is empty, warns client to not wait for file reception
		Packet dont_receive_files(Packet::ERR, 1, 1, "", 0);
		dont_receive_files.send_packet(this->client_fetch_socket);
		return;
	}
	
	for(size_t i = 0; i < total_paths; ++i){
		// Construct the packet payload string
		std::string payload = paths[i] + "\n" + std::to_string(total_paths) + "\n" + std::to_string(i);

		// Create a packet for sending
		Packet get_sync_command(Packet::DATA_PACKET, 1, 1, payload.c_str(), payload.size());

		// Send the packet
		get_sync_command.send_packet(this->client_fetch_socket);

		// Optional logging
		std::cout << "Sent path: " << paths[i] << std::endl;

		// Send the file using sender_reciever
		FileTransfer::send_file(paths[i], this->client_fetch_socket);
	}
}

void serverComManager::end_communications(){
	close(this->client_cmd_socket);
	close(this->client_fetch_socket);
	close(this->client_upload_socket);
	cout << "All sockets closed for user:" + this->username <<std::endl;
}

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
		this->client_list->add_device(username,this->client_cmd_socket);
		this->file_manager.create_server_sync_dir(username);
		this->client_list->display_clients();
		get_sync_dir();
	}
	
}


// PUBLIC METHODS

// This is the interface on server that will delegate each method based on commands.
void serverComManager::await_command_packet()
{
	bool exit = false;
	while(!exit) {
		// Wait to receive a command packet from client
		Packet command_packet = Packet::receive_packet(this->client_cmd_socket);
		// Determine what to do based on the command packet received
		switch(command_packet.get_seqn()){

			case Command::DOWNLOAD: {
				cout << "received download command from user: " + this->username << std::endl;
				string file_name = strtok(command_packet.get_payload(), "\n");
				string sync_dir_path = "../src/server/userDirectories/sync_dir_" + this->username;
				string file_path = sync_dir_path + "/" + file_name;
				FileTransfer::send_file(file_path, this->client_fetch_socket);
				break;
			  }

			case Command::GET_SYNC_DIR:{
				get_sync_dir();
				break;
        	}
			case Command::EXIT:{
				cout << "recieve exit command from user: " + this->username <<std::endl;

				//remove from server list this clients' connection
				dell_user.lock();
				this->client_list->remove_device(this->username,this->client_cmd_socket);
				dell_user.unlock();

				end_communications();
				exit = true;
				break;
			}
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

std::string serverComManager::get_username(){
	return this->username;
}
