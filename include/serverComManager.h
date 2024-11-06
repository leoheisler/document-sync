#ifndef SERVERCOMMANAGER_H
#define SERVERCOMMANAGER_H
#include <string>
#include <vector>
#include <sstream>

#include "clientList.h"
#include "packet.h"
#include "serverStatus.h"

class serverComManager
{
    private: 
        std::string username = "";
        int client_cmd_socket = -1;
        int client_upload_socket= -1; 
        int client_fetch_socket = -1;

        // Communication Methods
        void create_sync_dir(Packet sync_dir_packet,ClientList* client_device_list);
       
        
    public:
        // Constructor Method
        serverComManager();
        void await_command_packet(ClientList* client_device_list);
        serverStatus bind_client_sockets(int* server_socket);
        

};
#endif