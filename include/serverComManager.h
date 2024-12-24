#ifndef SERVER_COM_MANAGER_H
#define SERVER_COM_MANAGER_H
#include <string>
#include <vector>
#include <tuple>
#include <sstream>
#include <mutex>
#include <sys/stat.h>
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
#include "clientList.h"
#include "serverList.h"
#include "packet.h"
#include "serverStatus.h"
#include "serverFileManager.h"
#include "commandStatus.h"
#include "fileTransfer.h"

class serverComManager
{
    private: 
        ClientList* client_list;
        ServerList* server_list;
        serverFileManager file_manager;
        std::string username = "";
        bool is_backup_server = false;
        int client_cmd_socket = -1;
        int client_upload_socket= -1; 
        int client_fetch_socket = -1;

        // Communication Methods
        void start_communications();
        void end_communications(bool* exit);
        void get_sync_dir();
        void download(Packet command_packet);
        void list_server();
        void upload(Packet command_packet);
        void delete_server_file(Packet command_packet);
        
    public:
        // Constructor Method
        serverComManager(ClientList* client_list);
        void await_command_packet();
        serverStatus bind_client_sockets(int server_socket, int first_comm_socket);
        std::string get_username();
};
#endif