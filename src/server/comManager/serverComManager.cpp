#include "serverComManager.h" 

using namespace std;
namespace fs = std::filesystem;
#define CLIENT_PORT 1909
#define PORT 4000
#define ELECTION_PORT 3999

std::mutex access_device_list;
std::mutex access_server_list;
std::mutex access_heartbeat_time;
bool stop_heartbeat_thread = false;

// CONSTRUCTOR
serverComManager::serverComManager(ClientList* client_list, ServerList* server_list){ this->client_list = client_list; this->server_list = server_list;};
//getters & setters
std::string serverComManager::get_username()
{
	return this->username;
}
/*
======================================================================================
						MAIN SERVER SETTING
======================================================================================
*/
/*
	FUNCTION USED TO ACCEPTS CLIENT SOCKETS, FIRST FUNCT USED BY SERVER
*/
serverStatus serverComManager::bind_client_sockets(int server_socket, int first_comm_socket) {
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(struct sockaddr_in);
	this->client_cmd_socket = first_comm_socket;

	// Only primary servers need additional upload and fetch sockets
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
/*
	ACCEPTS CLIENTS AND PROPAGATE INFOS TO BACKUP_SERVERS
*/
void serverComManager::start_communications() {	
	Packet starter_packet = Packet::receive_packet(this->client_cmd_socket);
	char* payload = starter_packet.get_payload();
	std::string file_path;
	
	if(payload != nullptr){
		// Extract username and hostname from packet payload
		std::string username = strtok(payload, "\n");
		std::string hostname = strtok(nullptr, "\n");
		this->username = username;
		this->hostname = hostname;

		//try to add client to device list
		bool full_list;
		access_device_list.lock();
		{
			full_list = this->client_list->add_device(
				this->username, 
				this->hostname,
				tuple<int,int,int>{this->client_cmd_socket, this->client_upload_socket, this->client_fetch_socket}
			);
		}
		access_device_list.unlock();

		if(full_list){
			// If list of devices is fully occupied send error packet to signal client to exit
			Packet error_packet(Packet::ERR, Command::EXIT, 1, "", 0);
			error_packet.send_packet(this->client_cmd_socket);
		}else{
			// If list of devices has free space, allow client to connect
			this->file_manager.create_server_sync_dir(username);
			this->client_list->display_clients();

			access_server_list.lock();
			{
				// Propagate client info to all backup servers (sync)
				ServerNode* backup_server = server_list->get_first_server();
				string client_info = this->username + "\n" + this->hostname + "\n";

				while(backup_server != nullptr){
					int server_socket = backup_server->get_socket();
					Packet client_info_packet(Packet::CLIENTINFO_PACKET, 1, 1, client_info.c_str(), client_info.size());
					client_info_packet.send_packet(server_socket);
					backup_server = backup_server->get_next();
				}
			}
			access_server_list.unlock();

			// Send all client sync dir files
			get_sync_dir();	
		}
	}
}

/*
	INTERFACE THREAD THAT KEEPS ON LOOPING TILL THE SERVER IS DOWN
*/
void serverComManager::await_command_packet() {
	bool exit = false;
	while(!exit) {
		// Wait to receive a command packet from client
		Packet command_packet = Packet::receive_packet(this->client_cmd_socket);

		// Determine what to do based on the command packet received
		switch(command_packet.get_seqn()){

			case Command::UPLOAD: {
				cout << "received upload command from user: " + this->username << std::endl;
				upload(command_packet);
				break;
			}

			case Command::DOWNLOAD: {
				cout << "received download command from user: " + this->username << std::endl;
				download(command_packet);
				break;
			}

			case Command::DELETE:{
				cout << "received delete command from user: " + this->username << std::endl;
				delete_server_file(command_packet);
				break;
			}

			case Command::LIST_SERVER: {
				std::cout << "received list_server command from user: " + this->username << std::endl;
				list_server();
                break;
			}

			case Command::EXIT:{
				cout << "received exit command from user: " + this->username <<std::endl;
				end_communications(&exit);
				break;
			}

			case Command::GET_SYNC_DIR:{
				cout << "received get_sync_dir command from user: " + this->username << std::endl;
				get_sync_dir();
				break;
        	}
		}

		Packet ack_packet(Packet::SUCCESS, 1, 1, "", 0);
		ack_packet.send_packet(this->client_cmd_socket);
	}
}

/*
=====================================================================
					MAIN SERVER FUNCTIONS: CLIENT
=====================================================================
*/
void serverComManager::upload(Packet command_packet) {
	// Construct file path for client sync dir from packet payload and username
	string file_path = strtok(command_packet.get_payload(), "\n");
	fs::path path(file_path);
	string file_name = "/" + path.filename().string(); // Extracts only the file name
	string sync_dir_path = "../src/server/userDirectories/sync_dir_" + this->username;
	string local_file_path = sync_dir_path + file_name;

	// Receive file from client upload socket
	FileTransfer::receive_file(local_file_path, this->client_upload_socket);

	// Propagate file to both client devices (sync)
	ClientNode* client_devices = client_list->get_client(this->username);
	int device1_socket = client_devices->get_device1_download_socket();
	int device2_socket = client_devices->get_device2_download_socket();
	
	file_name = file_name + "\n";
	Packet file_name_packet_1(Packet::DATA_PACKET, 1, 1, file_name.c_str(), file_name.size());
	Packet file_name_packet_2(Packet::DATA_PACKET, 1, 1, file_name.c_str(), file_name.size());

	if(device1_socket != 0){
		file_name_packet_1.send_packet(device1_socket);
		FileTransfer::send_file(local_file_path, device1_socket);
	}
	if(device2_socket != 0){
		file_name_packet_2.send_packet(device2_socket);
		FileTransfer::send_file(local_file_path, device2_socket);
	}

	access_server_list.lock();
	{
		// propagate file to all backup servers (sync)
		ServerNode* backup_server = server_list->get_first_server();
		
		while(backup_server != nullptr){
			int server_socket = backup_server->get_socket();
			if(server_socket != 0){
				Packet file_path_packet(Packet::DATA_PACKET, 1, 1, (local_file_path + "\n").c_str(), (local_file_path + "\n").size());
				file_path_packet.send_packet(server_socket);
				FileTransfer::send_file(local_file_path, server_socket);
				backup_server = backup_server->get_next();
			}
		}
	}
	access_server_list.unlock();
}

void serverComManager::download(Packet command_packet) {
	// construct file path from packet payload and username
	string file_name = strtok(command_packet.get_payload(), "\n");
	string sync_dir_path = "../src/server/userDirectories/sync_dir_" + this->username;
	string file_path = sync_dir_path + file_name;

	// send file to client for downloading
	FileTransfer::send_file(file_path, this->client_cmd_socket);
	return;
}

void serverComManager::delete_server_file(Packet command_packet) {
	// Construct file path from packet payload and username
	string file_name = strtok(command_packet.get_payload(), "\n");
	string sync_dir_path = "../src/server/userDirectories/sync_dir_" + this->username;
	string file_path = sync_dir_path + file_name;
	
	// Delete the file in file_path path.
	string found_file = serverFileManager::delete_file(file_path);

	// Dont propagate if file has already been deleted from server
	if(found_file == "File not found or unable to delete.\n"){
		return;
	}

	// Propagate delete to both clients	
	ClientNode* client_devices = client_list->get_client(this->username);
	int device1_socket = client_devices->get_device1_download_socket();
	int device2_socket = client_devices->get_device2_download_socket();
	
	file_name = file_name + "\n";
	Packet file_name_packet_1(Packet::CMD_PACKET, Command::DELETE, 1, file_name.c_str(), file_name.size());
	Packet file_name_packet_2(Packet::CMD_PACKET, Command::DELETE, 1, file_name.c_str(), file_name.size());

	if(device1_socket != 0){
		file_name_packet_1.send_packet(device1_socket);
	}
	if(device2_socket != 0){
		file_name_packet_2.send_packet(device2_socket);
	}

	access_server_list.lock();
	{
		// Propagate delete to all backup servers (sync)
		ServerNode* backup_server = server_list->get_first_server();
		file_path += "\n";

		while(backup_server != nullptr){
			int server_socket = backup_server->get_socket();
			Packet file_name_packet(Packet::CMD_PACKET, Command::DELETE, 1, file_path.c_str(), file_path.size());
			file_name_packet.send_packet(server_socket);
			backup_server = backup_server->get_next();
		}
	}
	access_server_list.unlock();
}

void serverComManager::list_server() {  
    // Gets the file paths in the user's sync directory
    std::vector<std::string> paths = serverFileManager::get_sync_dir_paths(this->username);
    int total_paths = paths.size();
    
    // If there are no files in the sync directory
    if (total_paths == 0) {
        // Inform the client that there are no files to list
        Packet no_files(Packet::ERR, 1, 1, "No files to list", 0);
        no_files.send_packet(this->client_cmd_socket);
        return;
    }

    // For each file, calculate the times and send them to the client
    for (size_t i = 0; i < total_paths; ++i) {
        // Get MAC times (modification time, access time, change/creation time)
        struct stat file_stat;
        if (stat(paths[i].c_str(), &file_stat) == -1) {
            // If unable to retrieve the file's times, send an error message
            std::string error_msg = "Error retrieving times for file: " + paths[i];
            Packet error_packet(Packet::ERR, 1, 1, error_msg.c_str(), error_msg.size());
            error_packet.send_packet(this->client_cmd_socket);
            continue;
        }

        // Format the times as strings
        std::ostringstream mtime_stream, atime_stream, ctime_stream;
        mtime_stream << std::put_time(std::localtime(&file_stat.st_mtime), "%Y-%m-%d %H:%M:%S");
        atime_stream << std::put_time(std::localtime(&file_stat.st_atime), "%Y-%m-%d %H:%M:%S");
        ctime_stream << std::put_time(std::localtime(&file_stat.st_ctime), "%Y-%m-%d %H:%M:%S");

        // Format the message to send
        std::string file_info = paths[i] + "\n" +
                                mtime_stream.str() + "\n" +
                                atime_stream.str() + "\n" +
                                ctime_stream.str() + "\n" +
								std::to_string(total_paths) + "\n" + std::to_string(i);

        // Create a packet with the file information
        Packet file_info_packet(Packet::DATA_PACKET, 1, 1, file_info.c_str(), file_info.size());

        // Send the packet to the client
        file_info_packet.send_packet(this->client_cmd_socket);

        // Optional log for monitoring
        //std::cout << "Sent file info: " << file_info << std::endl;
    }
}

void serverComManager::end_communications(bool *exit) {
	// Remove client device from server device list
	access_device_list.lock();
    this->client_list->remove_device(
        this->username, 
        tuple<int,int,int>{this->client_cmd_socket, this->client_upload_socket, this->client_fetch_socket}
    );
	access_device_list.unlock();

	access_server_list.lock();
	{
		// Propagate client device delete to all backup servers (sync)
		ServerNode* backup_server = server_list->get_first_server();
		string client_info = this->username + "\n" + this->hostname + "\n";

		while(backup_server != nullptr){
			int server_socket = backup_server->get_socket();
			Packet client_info_packet(Packet::DELETEDEVICE_PACKET, 1, 1, client_info.c_str(), client_info.size());
			client_info_packet.send_packet(server_socket);
			backup_server = backup_server->get_next();
		}
	}
	access_server_list.unlock();

	// close sockets
	close(this->client_cmd_socket);
	close(this->client_fetch_socket);
	close(this->client_upload_socket);
	cout << "\nAll sockets closed for user:" + this->username <<std::endl;

	(*exit) = true;
}

void serverComManager::get_sync_dir() {
	std::vector<std::string> paths = serverFileManager::get_sync_dir_paths(this->username);
	int total_paths = paths.size();
	if(total_paths == 0){
		// If user sync dir is empty, warns client to not wait for file reception
		Packet dont_receive_files(Packet::ERR, 1, 1, "", 0);
		dont_receive_files.send_packet(this->client_cmd_socket);
		return;
	}
	
	for(size_t i = 0; i < total_paths; ++i){
		// Packet payload -> file path, total files being transfered, index of current file being transfered
		std::string payload = paths[i] + "\n" + std::to_string(total_paths) + "\n" + std::to_string(i) + "\n";

		// Create and send packet with file infos
		Packet get_sync_command(Packet::DATA_PACKET, 1, 1, payload.c_str(), payload.size());
		get_sync_command.send_packet(this->client_cmd_socket);
		std::cout << "Sent path: " << paths[i] << std::endl;

		// Send file
		FileTransfer::send_file(paths[i], this->client_cmd_socket);
	}
}


/*
============================================================================
					MAIN SERVER FUNCTIONS: BACKUP_SERVER
============================================================================
*/
/*
	ADD_BACKUP_SERVER: SET CONNECTED BACKUP SERVER IN SERVER_LIST
*/
void serverComManager::add_backup_server(int backup_server_socket, string hostname){
	access_server_list.lock();
	{
		// Propagate backup server info to all OTHER backup servers (sync)
		ServerNode* backup_server = server_list->get_first_server();
		string server_info = hostname + "\n";

		while(backup_server != nullptr){
			int server_socket = backup_server->get_socket();
			Packet* server_info_packet = new Packet(Packet::SERVERINFO_PACKET, 1, 1, server_info.c_str(), server_info.size());
			server_info_packet->send_packet(server_socket);
			backup_server = backup_server->get_next();
		}

		// Add connecting backup server to server list
		this->server_list->add_server(backup_server_socket, hostname);

		// Backup current server list to connecting backup server
		backup_server_list(backup_server_socket);

		this->server_list->display_servers();
	}
	access_server_list.unlock();

	backup_client_list(backup_server_socket);
	backup_sync_dir(backup_server_socket);
}
/*
	BACKUP_SYNC_DIR: SEND ALL ARCHIVES TO BACKUP_SERVERS
*/
void serverComManager::backup_sync_dir(int socket){
	std::vector<std::string> all_paths = file_manager.get_sync_dir_files_in_directory("../src/server/userDirectories");	
	int total_paths = all_paths.size();

	if(total_paths == 0){
		// If user sync dir is empty, warns client to not wait for file reception
		Packet dont_receive_files(Packet::ERR, 1, 1, "", 0);
		dont_receive_files.send_packet(socket);
		return;
	}
	
	// Loop to send each path and file to the backup server
	for(size_t i = 0; i < total_paths; ++i){
		// Packet payload -> file path, total files being transfered, index of current file being transfered
		std::string payload = all_paths[i] + "\n" + std::to_string(total_paths) + "\n" + std::to_string(i) + "\n";

		// Create and send packet with file infos
		Packet get_sync_command(Packet::DATA_PACKET, 1, 1, payload.c_str(), payload.size());
		get_sync_command.send_packet(socket);

		// Send file
		FileTransfer::send_file(all_paths[i], socket);
	}
}
/*
	BACKUP_SERVER_LIST: SEND THE BACKUP_SERVER_LIST TO BACKUP_SERVERS
*/
void serverComManager::backup_server_list(int socket){
	// Propagate current backup server list to connecting backup server (sync)
    ServerNode* backup_server = this->server_list->get_first_server();
    std::string all_server_infos = "";
    int size = 0;

	while(backup_server != nullptr){
		all_server_infos += backup_server->get_hostname() + "\n";
		backup_server = backup_server->get_next();
		size++;
	}

	// Send packet with all backup server hostnames
	Packet server_total(Packet::SERVERINFO_PACKET, size, 1, all_server_infos.c_str(), all_server_infos.size());
	server_total.send_packet(socket);
}
/*
	BACKUP_CLIENT_LIST: SEND THE CLIENT_LIST TO BACKUP_SERVERS
*/
void serverComManager::backup_client_list(int socket){
	access_device_list.lock();
	{
		// Propagate current client list to the connecting backup server (sync)
		ClientNode* client = this->client_list->get_first_client();

		while (client != nullptr) {
			// Retrieve client information
			std::string device1_hostname = client->get_device1_hostname();
			std::string device2_hostname = client->get_device2_hostname();

			// Add marker if device hostname is empty
			if (device1_hostname.empty()) {
				device1_hostname = "EMPTY_DEVICE_1";
			}
			if (device2_hostname.empty()) {
				device2_hostname = "EMPTY_DEVICE_2";
			}

			// Prepare packet content
			std::string client_info = client->get_username() + "\n";
			client_info += device1_hostname + "\n";
			client_info += device2_hostname + "\n";

			// Create and send the packet
			Packet client_info_packet(Packet::CLIENTINFO_PACKET, 1, 1, client_info.c_str(), client_info.size());
			client_info_packet.send_packet(socket);
			client = client->get_next();
		}
	}
	access_device_list.unlock();

	Packet end_transmission(Packet::EOT_PACKET, 1, 1, "", 0);
	end_transmission.send_packet(socket);
}
/*
	HEARTBEAT_PROTOCOL: SEND HEARTBEAT SIGNALS TO BACKUP_SERVERS
*/
void serverComManager::heartbeat_protocol(ServerList* server_list){
	std::thread(election_timer);

	while(true){
		// Propagate heartbeat packet to all backup servers every 5 seconds
		std::this_thread::sleep_for(std::chrono::seconds(5)); 
		// cout << "sending heartbeat" << endl;

		ServerNode* backup_server = server_list->get_first_server();
		
		while(backup_server != nullptr){
			int server_socket = backup_server->get_socket();
			Packet* file_path_packet = new Packet(Packet::HEARTBEAT_PACKET, 1, 1, "", 1);
			file_path_packet->send_packet(server_socket);
			backup_server = backup_server->get_next();
		}
	}
}
/*
	ELECTION_TIMER: SIGNALS TO BACKUP_SERVERS TO INITIATE ELECTION WHEN MAIN SERVER IS KILLED
*/
void serverComManager::election_timer(time_t* last_heartbeat, bool* should_start_election, int socket){
	time_t current_time;

	while(!stop_heartbeat_thread){
		std::this_thread::sleep_for(std::chrono::seconds(6)); 
		current_time = time(NULL);

		access_heartbeat_time.lock();
		{
			double elapsed_seconds_after_heartbeat = difftime(current_time, *last_heartbeat);
			if(elapsed_seconds_after_heartbeat > 6){
				
        		std::cout << "More than 6 seconds have passed since last heartbeat!" << std::endl;

				//START THE ELECTION HERE, if delay 15 seconds, change the bool to true to make it start the election
				*should_start_election = true;
				close(socket);
				std::cout << "Fechei Socket" << std::endl;
				// close the thread because we dont have to count until election ends
				stop_heartbeat_thread = true;
				
			}
		}
		access_heartbeat_time.unlock();
	}
}

/*
================================================================================
	FROM THIS POINT ONWARD ALL FUNCTIONS PAY RESPECT TO 
	A BACKUP_SERVER
================================================================================
*/

/*
==================================================
				BACKUP_SERVER SETTING
==================================================
*/
// RECEIVE SERVER_LIST FROM MAIN SERVER
void serverComManager::receive_server_list(int socket){
	access_server_list.lock();
		Packet server_list_infos = Packet::receive_packet(socket);
		string server_hostname;
		int server_amount = server_list_infos.get_seqn();

		if (server_amount != 0) {
			char* payload = server_list_infos.get_payload();
			server_hostname = strtok(payload, "\n");
			this->server_list->add_server(0, server_hostname);

			for (int i = 1; i < server_amount; i++) {
				server_hostname = strtok(nullptr, "\n");
				this->server_list->add_server(0, server_hostname);
			}
		}
		this->server_list->display_servers();
	access_server_list.unlock();
}
// RECEIVE CLIENT LIST FROM MAIN SERVER
void serverComManager::receive_client_list(int socket) {
    access_device_list.lock();
    {
        // Start receiving client list data
        bool end_of_transmission = false;
        while (!end_of_transmission) {
            // Receive the client info packet
            Packet client_info_packet = Packet::receive_packet(socket);

            if (client_info_packet.get_type() == Packet::EOT_PACKET) {
                end_of_transmission = true;
            } else if (client_info_packet.get_type() == Packet::CLIENTINFO_PACKET) {
                // Extract client information
                char* client_info = client_info_packet.get_payload();

                string username;
                string device1_hostname;
                string device2_hostname;

				cout << string(client_info) << endl;

                // Read the client information from the stream
                username = strtok(client_info, "\n");
                device1_hostname = strtok(nullptr, "\n");
                device2_hostname = strtok(nullptr, "\n");
				
                // Add the client to the list
				if(device1_hostname != "EMPTY_DEVICE_1")
                	this->client_list->add_device(username, device1_hostname, tuple<int,int,int>(0,0,0));
				if(device2_hostname != "EMPTY_DEVICE_2")
					this->client_list->add_device(username, device2_hostname, tuple<int,int,int>(0,0,0));
            }
        }
		this->client_list->display_clients();
    }
    access_device_list.unlock();
}

// LOOP TO GET SYNC FUNCTIONS
void serverComManager::await_sync(int socket, bool* elected) {
	bool should_start_election = false;
	bool wait_election = false;
	time_t last_heartbeat = time(NULL);
	std::thread heartbeat_timeout(election_timer, &last_heartbeat, &should_start_election, socket);
	heartbeat_timeout.detach();

	while(!(*elected))
	{
		//cout << should_start_election << endl;
		if(should_start_election)
		{
			build_ring();
			start_ring_election(&wait_election);
			should_start_election = false;
			wait_election = true;
		}
		if(wait_election){
			handle_election(&wait_election, elected);
		}
		if(!wait_election)
		{
			// wait for only 15 seconds, timeout is important to unblock this thread
			Packet received_packet = Packet::receive_packet(socket, 15);

			if (!received_packet.is_empty()) {
				access_heartbeat_time.lock();
				{
					last_heartbeat = time(NULL);
				}
				access_heartbeat_time.unlock();

				switch(received_packet.get_type()){
					// CMD_PACKET == DELETE SYNC
					case Packet::CMD_PACKET:
					{
						string file_path = strtok(received_packet.get_payload(), "\n");        
						serverFileManager::delete_file(file_path);
						break;
					}

					// DATA_PACKET == DOWNLOAD SYNC
					case Packet::DATA_PACKET:
					{
						string file_path = strtok(received_packet.get_payload(), "\n");        
						FileTransfer::receive_file(file_path, socket);	
						break;
					}

					// CLIENTINFO_PACKET == ADD NEW CLIENT DEVICE TO CLIENT LIST
					case Packet::CLIENTINFO_PACKET:
					{
						string client_username = strtok(received_packet.get_payload(), "\n");
						string client_hostname = strtok(nullptr, "\n");
						this->client_list->add_device(client_username, client_hostname, tuple<int,int,int>(0,0,0));
						this->client_list->display_clients();
						break;
					}

					// DELETEDEVICE_PACKET == REMOVE CLIENT DEVICE FROM CLIENT LIST
					case Packet::DELETEDEVICE_PACKET:
					{
						string client_username = strtok(received_packet.get_payload(), "\n");
						string client_hostname = strtok(nullptr, "\n");
						this->client_list->remove_device(client_username, client_hostname);
						this->client_list->display_clients();
						break;
					}

					// SERVERINFO_PACKET == ADD NEW BACKUP SERVER TO SERVER LIST
					case Packet::SERVERINFO_PACKET:
					{
						string server_hostname = strtok(received_packet.get_payload(), "\n");
						this->server_list->add_server(0, server_hostname);
						this->server_list->display_servers();
						break;
					}

					// DELETESERVER_PACKET == DELETE BACKUP SERVER FROM SERVER LIST
					case Packet::DELETESERVER_PACKET:
					{
						string server_hostname = strtok(received_packet.get_payload(), "\n");
						this->server_list->remove_server(server_hostname);
						this->server_list->display_servers();
						break;
					}

					// HEARTBEAT_PACKET == MAIN SERVER HEARTBEAT
					case Packet::HEARTBEAT_PACKET:
					{
						//cout << "received heartbeat..." << endl;
						break;
					}
				}
			}

			//cout << 'received_packet: ' << endl;

		}
	}
}

/*
====================================================================================================
                           				ELECTION FUNCTIONS
====================================================================================================
*/
// Start sockets to create the ring
void serverComManager::start_election_sockets() {
	//listening_socket used by backup server to connect to the next backup server in the ring
    if ((this->listening_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        cout << "ERROR opening listening_socket" << endl;
	//outgoing_election_socket used by backup server to connect to the next backup server in the ring
    if ((this->outgoing_election_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        cout << "ERROR opening outgoing_election_socket" << endl;;

	cout << "startei election sockets" << endl;
  
}

// Bind de election ring sockets
void serverComManager::bind_incoming_election_socket(){
	struct sockaddr_in cli_addr;
	cli_addr.sin_family = AF_INET;
	cli_addr.sin_port = htons(ELECTION_PORT);
	cli_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(cli_addr.sin_zero), 8);     
	if (bind(this->listening_socket, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0){
		throw std::runtime_error("ERRO BINDANDO O LISTENING SOCKET");
	}

	cout << "bindei incoming election socket" << endl;; 

	
	// Setting the ID for the election
	char self_hostname[256];
	gethostname(self_hostname, sizeof(self_hostname));

	this->id = this->server_list->get_server_id(self_hostname);

	cout << "MINHA ID EH:" << this->id << endl ;
}

//send ring connection and wait for a link
void serverComManager::build_ring(){
	cout << "building ring..." << endl;
	struct hostent *server;
	char self_hostname[256];
	gethostname(self_hostname, sizeof(self_hostname));
	ServerNode* next_backup_s = this->server_list->find_next_server(self_hostname);

	server = gethostbyname(next_backup_s->get_hostname().c_str());

	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}

	connect_election_sockets(server);

	while( this->incoming_election_socket == -1){}
}
// send connection
void serverComManager::connect_election_sockets(hostent* backup_server) {
	struct sockaddr_in serv_addr;

	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(ELECTION_PORT);    
	serv_addr.sin_addr = *((struct in_addr *)backup_server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);  

	if (connect(this->outgoing_election_socket,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		cout << "ERROR connecting outgoing_election_socket" << endl;
	else{}
		// cout << "outgoing_election_socket connected" << endl;

}

// accepts ring connections
void serverComManager::accept_election_connection(){
	int listening_socket;
	int first_contact_socket;
	struct sockaddr_in previous_backup_address;
    socklen_t previous_backup_len = sizeof(struct sockaddr_in);

	listen(this->listening_socket, 1);

	this->incoming_election_socket = accept(this->listening_socket,(struct sockaddr*)&previous_backup_address,&previous_backup_len);
	// cout << "incoming election connection accepted" << endl;
}

void serverComManager::start_ring_election(bool* wait_election) {

	if(not(this->participant)){
		int outgoing_socket = this->outgoing_election_socket;

		std::string my_id = std::to_string(this->id)+ "\n";

		Packet election_packet = Packet(Packet::ELECTION_PACKET, 1, 1, my_id.c_str(), my_id.size());
		election_packet.send_packet(outgoing_socket);

		this->participant = true;

		// cout << "MANDEI PACKET para outgoing socket!!" << endl;
	}	
}

// Receive incomming election packet and execute ring logic and send outgoing packet.
void serverComManager::handle_election(bool* wait_election, bool* elected) {
	
	this->participant = true;
	//====================== HANDLING PACKET  ====================

	int my_id = this->id;  // Definindo inicialmente o maior identificador
	int max_id;

	// recieves incoming election packet from previous server
	Packet received_packet = Packet::receive_packet(this->incoming_election_socket);

	// Extract the payload
	std::string received_id = strtok(received_packet.get_payload(), "\n");

	// cout << "RECEBI UM PACOTE Q DENTRO TINHA:" << received_id << endl;

	int payload_id = std::stoi(received_id);

	// Sets to where you send the packet
	int outgoing_socket = this->outgoing_election_socket;

	// The servers can send two types of messages, either an ELECTION or an ELECTED, id they recieve an ELECTED message, then...
	if (received_packet.get_type() == Packet::ELECTED_PACKET) {
		// Find the server with the elected id, set it as the elected server
		if (my_id == payload_id) {
			// cout << "I WAS ELECTED!!" << endl;
			*elected =true;
			close_old_connections();
			evolve_into_main();
			return;
		} else {
			// cout << "SOMEONE WAS ELECTED!!!!" << endl;
			*wait_election = false;
			Packet elected_leader_packet = Packet(Packet::ELECTED_PACKET, 1, 1, received_id.c_str(), received_id.size());
			elected_leader_packet.send_packet(outgoing_socket);
			close_old_connections();
			return;
		}
	}

	if (received_packet.get_type() != Packet::ELECTION_PACKET)
	{
		cerr << "ERROR: wrong packet type during election!"<< endl;
	}
	
	if (my_id < payload_id){
		max_id = payload_id;
	} else if (my_id > payload_id) {
		if (this->participant) {
			return;
		} else {
			max_id = payload_id; 		
		}
	} else if (my_id == payload_id){
		// This means that the server is the elected one.
		cout << "KENJI WAS ELECTED!!!!" << endl;
		Packet elected_leader_packet = Packet(Packet::ELECTED_PACKET, 1, 1, received_id.c_str(), received_id.size());
		elected_leader_packet.send_packet(outgoing_socket);
		return;
		
	}

	// Here is the election packet
	std::string string_max_id = std::to_string(max_id) + "\n";
	Packet election_packet = Packet(Packet::ELECTION_PACKET, 1, 1, string_max_id.c_str(), string_max_id.size());

	election_packet.send_packet(outgoing_socket);
}

void serverComManager::close_old_connections(){
	close(this->incoming_election_socket);
	close(this->outgoing_election_socket);
}

/*
==============================================================
				BACKUP_SERVER TO MAIN_SERVER
=============================================================
*/
//SET TRANSFORMS BACKUP_SERVER INTO MAIN_SERVER
void serverComManager::evolve_into_main() {
	char self_hostname[256];
    gethostname(self_hostname, sizeof(self_hostname));

	access_server_list.lock();
	{
		this->server_list->remove_server(self_hostname);
		this->server_list->display_servers();
		this->client_list->display_clients();
	}
	access_server_list.unlock();

	string teste_client_hostname = this->client_list->get_first_client()->get_device1_hostname();
	connect_to_hostname(const_cast<char*>(teste_client_hostname.c_str()));
}

//=========================================================
//  BACKUP_SERVER TO MAIN: CLIENT REVERSE CONNECTION
//=========================================================
/*
	GIVEN A STRING HOSTNAME, IT CONNECTS THE SERVER_COM SOCKETS TO
	THE ADDRESS FOUND, IF IT COULDN'T FIND ANY IT EXITS W/0
*/
void serverComManager::connect_to_hostname(char* hostname){
	string teste_client_hostname = this->client_list->get_first_client()->get_device1_hostname();
	string teste_client_username = this->client_list->get_first_client()->get_username();
	struct hostent *client_host; 
	client_host = gethostbyname(hostname);

	if(client_host == NULL){
		cout << "NAO CONSEGUI ENCONTRAR O ENDERECO" << endl;
		exit(0);
	}
	start_sockets();
	connect_sockets(CLIENT_PORT, client_host);
	this->username = teste_client_username;
	this->hostname = teste_client_hostname;

	// cout << "CONECTADO AO USER:" << this->username << endl;
	await_command_packet();
}
//aux functs
/*
	START SOCKETS: start sockets to client connection
*/
void serverComManager::start_sockets() {

    //sock_cmd used for the client to send commands like: download, upload, delete, list_server, list_server, exit, two-way communication
    if ((this->client_cmd_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        cout << "ERROR opening cmd socket" << endl;
    
    //sock_upload used for the client to upload files from inotify events syncs into the server
    if ((this->client_upload_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        cout << "ERROR opening upload socket" << endl;

    //sock_fetch used for the client to download files from the server if synchronization needed
    if ((this->client_fetch_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        cout << "ERROR opening fetch socket" << endl;
}

/*
	CONNECT_SOCKETS: SEND TCP CONNECTIONS TO CLIENT_HOST
*/
void serverComManager::connect_sockets(int port, hostent* client_host) {
    struct sockaddr_in client_addr;

    client_addr.sin_family = AF_INET;     
	client_addr.sin_port = htons(port);    
	client_addr.sin_addr = *((struct in_addr *)client_host->h_addr);
	bzero(&(client_addr.sin_zero), 8);  

    if (connect(this->client_cmd_socket,(struct sockaddr *) &client_addr,sizeof(client_addr)) < 0) 
        cout << "ERROR connecting cmd socket" << endl;
        // cout <<"cmd socket connected" << endl;

    if (connect(this->client_upload_socket,(struct sockaddr *) &client_addr,sizeof(client_addr)) < 0) 
        cout << "ERROR connecting upload socket" << endl;
        // cout <<"upload socket connected" << endl;

    if (connect(this->client_fetch_socket,(struct sockaddr *) &client_addr,sizeof(client_addr)) < 0) 
        cout << "ERROR connecting fetch socket" << endl;
        // cout <<"fetch socket connected" << endl;
    
}
