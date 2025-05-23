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

std::mutex connect_hand;

serverStatus bind_main_server_socket(int* server_socket, int port){
	struct sockaddr_in serv_addr;

	if ((*server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("ERROR opening socket\n");
		return serverStatus::FAILED_TO_CREATE_SOCKET;
	}
        
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);     
	if (bind(*server_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		cout << ("ERROR on binding");
		return serverStatus::FAILED_TO_BIND_SOCKET;
	} 

	return serverStatus::OK;
}

void main_server_connection_handler(int server_socket, int first_contact_socket, ClientList* client_device_list, ServerList* backup_server_list){
	// Create a server comm manager for the connection
	serverComManager server_comm(client_device_list, backup_server_list);

	// Send a handshake to client
	Packet handshake_packet(first_contact_socket);
	handshake_packet.send_packet(first_contact_socket);

	Packet ack_packet = Packet::receive_packet(first_contact_socket);
	if(ack_packet.get_seqn()){
		// Client is an user device
		server_comm.bind_client_sockets(server_socket, first_contact_socket);
		connect_hand.unlock();
		server_comm.await_command_packet();
	}else{
		// Client is a backup server
		connect_hand.unlock();
		string server_hostname = strtok(ack_packet.get_payload(), "\n");
		server_comm.add_backup_server(first_contact_socket, server_hostname);
		while(true);
	}
	
	cout << "liberating memory and ending thread for user: " + server_comm.get_username() << std::endl;
}

serverStatus setup_backup_server_socket(int port, string hostname, hostent* server, int* server_socket){
	struct sockaddr_in serv_addr;

	if ((*server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("ERROR opening socket\n");
		return serverStatus::FAILED_TO_CREATE_SOCKET;
	}

	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(port);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);  

    if (connect(*server_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        cout << "ERROR connecting backup server socket\n";
		return serverStatus::FAILED_TO_CREATE_SOCKET;
	}else
        cout <<"backup server socket connected\n";

	Packet handshake_packet = Packet::receive_packet(*server_socket);
	
	// Send packet to communicate to main server that it is a backup server
    Packet ack_packet(Packet::COMM_PACKET, 0, 1, (hostname + "\n").c_str(), (hostname + "\n").size());
    ack_packet.send_packet(*server_socket);

	return serverStatus::OK;
}

int main(int argc, char *argv[])
{
	// SERVER SOCKETS
	int server_socket;
	int connection_socket;
	int first_contact_socket;
	struct sockaddr_in client_address;
	socklen_t client_len = sizeof(struct sockaddr_in);

	// Linked list to store client and server infos 
    ClientList client_device_list;
	ServerList server_list;

	if(argc == 1 || argc == 2){
		cout << "Starting main server... \n";
		int port = 4000;  // Default port
		if (argc == 2) {
			port = stoi(argv[1]);  // Convert the argument to an integer (port)
		}

		// BIND MAIN SOCKET
		serverStatus isBinded = bind_main_server_socket(&server_socket, port);
		if(isBinded != serverStatus::OK){
			cout << to_string(isBinded);
			return -1;
		}

		// START HEARTBEAT THREAD
		std::thread heartbeat_thread(serverComManager::heartbeat_protocol, &server_list);
        heartbeat_thread.detach();

		// LISTEN
		listen(server_socket, 6);
		if(argc == 1){
			cout << "================================\n SERVER LISTENING ON PORT 4000\n================================\n";
		}else
			cout << "================================\n SERVER LISTENING ON PORT " + string(argv[1]) + "\n================================\n";


		// loop de criação de threads
		while(true){
			//pega o mutex para si, não deixando o while seguir aceitando outros clientes
			connect_hand.lock();

			//cria uma thread pra primeira conexão do cliente
			first_contact_socket = accept(server_socket,(struct sockaddr*)&client_address,&client_len);
			if(first_contact_socket >= 0){
				std::thread t(main_server_connection_handler, server_socket, first_contact_socket, &client_device_list, &server_list);
				t.detach();
			}
		}
	}else if(argc == 3){
		cout << "Starting backup server...\n";
		serverComManager com_manager(&client_device_list, &server_list);
		int  port;
    	struct hostent *server;
		char self_hostname[256];
		bool elected = false;
		server = gethostbyname(argv[1]);
		port = atoi(argv[2]);
		gethostname(self_hostname, sizeof(self_hostname));
		setup_backup_server_socket(port, self_hostname, server, &connection_socket);

		// Data structures and file syncs
		com_manager.receive_server_list(connection_socket);
		com_manager.receive_client_list(connection_socket);
		serverFileManager::receive_sync_dir_files(connection_socket);

		// Initialize sockets for the election
		com_manager.start_election_sockets();
		com_manager.bind_incoming_election_socket();
		
		// Create listener thread for election connections on port 3999
        thread listener_thread(&serverComManager::accept_election_connection, &com_manager);
		listener_thread.detach();

		// Infinite loop awaiting syncronizations from main server
		com_manager.await_sync(&connection_socket, &elected);

		// ============================================================ // 
		// IF LOOP WAS BROKEN, BACKUP SERVER WAS ELECTED AS MAIN SERVER //
        // ============================================================ //

		// BIND MAIN SOCKET
		serverStatus isBinded = bind_main_server_socket(&server_socket, 4000);
		if(isBinded != serverStatus::OK){
			cout << to_string(isBinded);
			return -1;
		}

		// START HEARTBEAT THREAD
		std::thread heartbeat_thread(serverComManager::heartbeat_protocol, &server_list);
        heartbeat_thread.detach();

		// LISTEN
		listen(server_socket, 6);
		cout << "================================\n SERVER LISTENING ON PORT 4000\n================================\n";

		while(true){
			//pega o mutex para si, não deixando o while seguir aceitando outros clientes
			connect_hand.lock();

			//cria uma thread pra primeira conexão do cliente
			first_contact_socket = accept(server_socket,(struct sockaddr*)&client_address,&client_len);
			if(first_contact_socket >= 0){
				std::thread t(main_server_connection_handler, server_socket, first_contact_socket, &client_device_list, &server_list);
				t.detach();
			}
		}
	}


	return 0;
}

