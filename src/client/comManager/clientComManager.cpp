#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

#include "clientComManager.h" 
#include "commandStatus.h"
#include "packet.h"
#include "fileTransfer.h"
#include "clientParser.h" 

using namespace std;

// CONSTRUCTOR
clientComManager::clientComManager(/* args */){};

// PRIVATE METHODS

// asks for the file that will be deleted and sends request with file and username
// inotify will probably listen to changes in server and update local directories on clients..
void clientComManager::send_delete_request(std::string file_name)
{
    // Send packet signaling to server what file it wants to delete.
    Packet delete_command = Packet(Packet::CMD_PACKET, Command::DELETE, 1, (file_name + "\n").c_str(), file_name.length());
    delete_command.send_packet(this->sock_cmd);
}


void clientComManager::get_sync_dir()
{
    // Send packet signaling server to execute get_sync_dir with client info (username and socket)
    string client_info = (get_username() + "\n" + to_string(this->sock_cmd));
    Packet get_sync_command = Packet(Packet::CMD_PACKET, Command::GET_SYNC_DIR, 1, client_info.c_str(), client_info.length());
    get_sync_command.send_packet(this->sock_cmd);
}

void clientComManager::receive_sync_dir_files() {
    int client_socket = this->sock_cmd;
    FileTransfer receiver;
    while (true) {
        // Receive a packet
        Packet received_packet = Packet::receive_packet(client_socket);
        if (received_packet.get_type() != Packet::DATA_PACKET) {
            cout << this->username + " sync dir is empty" << endl;
            break;
        }

        // Extract the payload
        std::string payload(received_packet.get_payload(), received_packet.get_length());

        // Split the payload to extract path, total paths, and index
        std::istringstream payload_stream(payload);
        std::string path;
        int total_paths = 0, index = 0;

        if (std::getline(payload_stream, path, '\n') && 
            payload_stream >> total_paths && 
            payload_stream >> index) {
            // Log received information
            std::cout << "Received path: " << path << " (Index " << index << " of " << total_paths << ")" << std::endl;

            size_t last_slash = path.find_last_of("/\\");
            std::string filename = (last_slash != std::string::npos) ? path.substr(last_slash) : path;
            // Receive the file using the extracted path
            cout << "will store at: ../src/client/sync_dir" + filename << endl;
            receiver.receive_file("../src/client/sync_dir" + filename, client_socket);

            // Check if all paths are received
            if (index + 1 == total_paths) {
                std::cout << "All files received." << std::endl;
                break;
                return;
            }
        } else {
            std::cerr << "Error: Invalid payload format." << std::endl;
            break;
        }
    }
    return;
}

void clientComManager::download(std::string file_name)
{
    // Send packet signaling to server what file it wants to download
    Packet download_command = Packet(Packet::CMD_PACKET, Command::DOWNLOAD, 1, (file_name + "\n").c_str(), file_name.length());
    download_command.send_packet(this->sock_cmd);
    FileTransfer::receive_file("../" + file_name, this->sock_cmd);
}

void clientComManager::start_sockets(){

    if ((this->sock_cmd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        cout << "ERROR opening cmd socket\n";
    
    if ((this->sock_upload = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        cout << "ERROR opening upload socket\n";

    if ((this->sock_fetch = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        cout << "ERROR opening fetch socket\n";
    
                    
}

void clientComManager::connect_sockets(int port, hostent* server){

    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(port);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);  

    if (connect(this->sock_cmd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        cout << "ERROR connecting cmd socket\n";
    else
        cout <<"cmd socket connected\n";

    Packet handshake_packet = Packet::receive_packet(this->sock_cmd);
    if(handshake_packet.get_type() == Packet::COMM_PACKET){
        if (connect(this->sock_upload,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
            cout << "ERROR connecting upload socket\n";
        else
            cout <<"upload socket connected\n";

        if (connect(this->sock_fetch,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
            cout << "ERROR connecting fetch socket\n";
        else
            cout <<"fetch socket connected\n";

    }
    
    
}

void clientComManager::close_sockets(){
    close(this->sock_cmd);
    close(this->sock_fetch);
    close(this->sock_upload);
    cout << "All sockets closed,";
}
// PUBLIC METHODS

// This is the interface on client that will delegate each method based on commands.
std::string clientComManager::execute_command(Command command) {
    FileTransfer sender_reciever_client;
    switch (command) {
        case Command::GET_SYNC_DIR:
            try {
                get_sync_dir();
                receive_sync_dir_files(); 
                return "Everything ok.";
            } catch (const std::exception& e) {
                return std::string("Something went wrong: ") + e.what();
            } 
        case Command::LIST_CLIENT:
            return "NOT IMPLEMENTED YET";
        case Command::LIST_SERVER:
            return "NOT IMPLEMENTED YET";
        case Command::UPLOAD:
            return "NOT IMPLEMENTED YET";
        case Command::DOWNLOAD:
            try {           
                // Recebe nome do arquivo a ser baixado na linha de comando 
                string file_name;
                cout << "\nInsira o nome do arquivo desejado: ";
                cin >> file_name;

                // Executa o comando/método de download
                download(file_name);
                return "Everything ok.";
            } catch (const std::exception& e) {
                return std::string("Something went wrong: ") + e.what();
            } 
        case Command::DELETE:
            try {
                string file_name;
                cout << "\nInsira o nome do arquivo desejado: ";
                cin >> file_name;

                send_delete_request(file_name);
                return "Everything ok.";
            } catch (const std::exception& e) {
                return std::string("Something went wrong: ") + e.what();
            }
        case Command::EXIT:{ 
            Packet exit_command = Packet(Packet::CMD_PACKET, Command::EXIT, 0, "", 0);
            exit_command.send_packet(this->sock_cmd);
            this->close_sockets();
            std::cout << " shutting down... bye bye" << std::endl;
            break;
        }
        default:
            return "UNKNOWN COMMAND! The code shouldn't arrive here, check get_command_from_string and valid_command_status for debugging, code: " + std::to_string(static_cast<int>(command));
    }
}

void clientComManager::await_sync()
{
    // Receive a packet
    Packet received_packet = Packet::receive_packet(this->sock_fetch);
    if (received_packet.get_type() != Packet::DATA_PACKET) {
        return;
    }
    return;
}

int clientComManager::connect_client_to_server(int argc, char* argv[])
{
    int  port;
    struct hostent *server;
    
	set_username(argv[1]);
	server = gethostbyname(argv[2]);
    port = atoi(argv[3]);

	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    // SOCKET
    start_sockets();

    // CONNECT
	connect_sockets(port,server);
    execute_command(Command::GET_SYNC_DIR);

    return 0;
}

// GETTERS & SETTERS
std::string clientComManager::get_username(){ return this->username; }
void clientComManager::set_username(std::string username){ this->username = username; }