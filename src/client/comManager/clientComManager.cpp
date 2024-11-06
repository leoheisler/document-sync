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

using namespace std;

// CONSTRUCTOR
clientComManager::clientComManager(/* args */){};

// PRIVATE METHODS
void clientComManager::get_sync_dir(int socket)
{
    // Send packet signaling server to execute get_sync_dir with client info (username and socket)
    string client_info = (get_username() + "\n" + to_string(socket));
    Packet get_sync_command = Packet(Packet::CMD_PACKET, Command::GET_SYNC_DIR, 1, client_info.c_str(), client_info.length());
    get_sync_command.send_packet(socket);

    std::string file_path1 = "../src/client/syncDir/socket_cmd.txt";
    FileTransfer::receive_file(file_path1, this->sock_cmd);

    std::string file_path2 = "../src/client/syncDir/socket_upload.txt";
    FileTransfer::receive_file(file_path2, this->sock_upload);

    std::string file_path3 = "../src/client/syncDir/socket_fetch.txt";
    FileTransfer::receive_file(file_path3, this->sock_fetch);   

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

    if (connect(this->sock_upload,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        cout << "ERROR connecting upload socket\n";
    else
        cout << "upload socket connected\n";

    if (connect(this->sock_fetch,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        cout << "ERROR connecting fetch socket\n";
    else
        cout << "fetch socket connected \n";
    
}

void clientComManager::close_sockets(){
    close(this->sock_cmd);
    close(this->sock_fetch);
    close(this->sock_upload);
    cout << "all sockets closed";
}
// PUBLIC METHODS
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
    //SEND dir files
    get_sync_dir(this->sock_cmd);

    return 0;
}

// GETTERS & SETTERS
std::string clientComManager::get_username(){ return this->username; }
void clientComManager::set_username(std::string username){ this->username = username; }