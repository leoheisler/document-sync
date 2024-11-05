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
void serverComManager::evaluate_command(Packet command_packet, int socket)
{
	int command_id = command_packet.get_seqn();
	switch(command_id){
		case Command::GET_SYNC_DIR:
			cout << "Received get_sync_dir packet" << endl;
			create_sync_dir(command_packet, socket);
			break;
	}
}



void serverComManager::create_sync_dir(Packet sync_dir_packet, int socket)
{
	char* payload = sync_dir_packet.get_payload();

	if(payload != nullptr){
		// Extract username and client socket from packet payload
		std::string username = strtok(payload, "\n");
		int socket = atoi(strtok(nullptr, "\0"));

		//try to add client to device list
		try
		{
			clientDeviceList.add_device(username,socket);

		}
		//if couldnt connect, send and error packet
		catch(const std::string& e)
		{
			std::cout<< e << '\n';
			Packet error_packet = Packet();
			error_packet.send_packet(socket);
		}


	}
}

// PUBLIC METHODS
void serverComManager::await_command_packet(int socket)
{
	while(true){
		// Wait to receive a command packet from client
		Packet command_packet = Packet::receive_packet(socket);

		// Determine what to do based on the command packet received
		switch(command_packet.get_type()){
			case Packet::CMD_PACKET:
				evaluate_command(command_packet, socket);
				break;
		}
	}
}
