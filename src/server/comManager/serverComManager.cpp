#include "serverComManager.h" 


#define PORT 4000
std::mutex dell_user;
// CONSTRUCTOR
serverComManager::serverComManager(ClientList* client_list){ this->client_list = client_list;};

// PRIVATE METHODS
void serverComManager::upload(Packet command_packet)
{
	// construct file path for client sync dir from packet payload and username
	string file_path = strtok(command_packet.get_payload(), "\n");
	size_t last_slash = file_path.find_last_of("/\\");
    string file_name = (last_slash != std::string::npos) ? file_path.substr(last_slash) : file_path;
	string sync_dir_path = "../src/server/userDirectories/sync_dir_" + this->username;
	string local_file_path = sync_dir_path + "/" + file_name;

	// receive file from client upload socket
	FileTransfer::receive_file(local_file_path, this->client_upload_socket);

	// propagate file to both client devices (sync)
	ClientNode* client_devices = client_list->get_client(this->username);
	int device1_socket = client_devices->get_device1_download_socket();
	int device2_socket = client_devices->get_device2_download_socket();

	Packet file_name_packet(Packet::DATA_PACKET, 1, 1, file_name.c_str(), file_name.size());
	if(device1_socket != 0){
		file_name_packet.send_packet(device1_socket);
		FileTransfer::send_file(local_file_path, device1_socket);	
	}
	if(device2_socket != 0){
		file_name_packet.send_packet(device2_socket);
		FileTransfer::send_file(local_file_path, device2_socket);	
	}
}


void serverComManager::download(Packet command_packet)
{
	// construct file path from packet payload and username
	string file_name = strtok(command_packet.get_payload(), "\n");
	string sync_dir_path = "../src/server/userDirectories/sync_dir_" + this->username;
	string file_path = sync_dir_path + "/" + file_name;

	// send file to client for downloading
	FileTransfer::send_file(file_path, this->client_cmd_socket);
	return;
}

void serverComManager::delete_server_file(Packet command_packet)
{
	// construct file path from packet payload and username
	string file_name = strtok(command_packet.get_payload(), "\n");
	string sync_dir_path = "../src/server/userDirectories/sync_dir_" + this->username;
	string file_path = sync_dir_path + "/" + file_name;

	// delete the file in file_path path.
	cout << serverFileManager::delete_file(file_path) << endl;
}

void serverComManager::list_server() 
{    
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

void serverComManager::end_communications(bool *exit)
{
	// Remove client device from server device list
	dell_user.lock();
    this->client_list->remove_device(
        this->username, 
        tuple<int,int,int>{this->client_cmd_socket, this->client_upload_socket, this->client_fetch_socket}
    );
	dell_user.unlock();

	// close sockets
	close(this->client_cmd_socket);
	close(this->client_fetch_socket);
	close(this->client_upload_socket);
	cout << "All sockets closed for user:" + this->username <<std::endl;

	(*exit) = true;
}

void serverComManager::get_sync_dir()
{
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
		std::string payload = paths[i] + "\n" + std::to_string(total_paths) + "\n" + std::to_string(i);

		// Create and send packet with file infos
		Packet get_sync_command(Packet::DATA_PACKET, 1, 1, payload.c_str(), payload.size());
		get_sync_command.send_packet(this->client_cmd_socket);
		std::cout << "Sent path: " << paths[i] << std::endl;

		// Send file
		FileTransfer::send_file(paths[i], this->client_cmd_socket);
	}
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
		this->client_list->add_device(
			this->username, 
			tuple<int,int,int>{this->client_cmd_socket, this->client_upload_socket, this->client_fetch_socket}
		);
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
