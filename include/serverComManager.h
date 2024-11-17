#ifndef SERVER_COM_MANAGER_H
#define SERVER_COM_MANAGER_H
#include <string>
#include <vector>
#include <sstream>

#include "clientList.h"
#include "packet.h"
#include "serverStatus.h"
#include "serverFileManager.h"

class serverComManager
{
    private: 
        ClientList* client_list;
        serverFileManager file_manager;
        std::string username = "";
        int client_cmd_socket = -1;
        int client_upload_socket= -1; 
        int client_fetch_socket = -1;

        // Communication Methods
        void start_communications();
        void end_communications();
       
        
    public:
        // Constructor Method
        serverComManager(ClientList* client_list);
        void await_command_packet();
        serverStatus bind_client_sockets(int server_socket, int first_comm_socket);
        std::string get_username();
        

};
#endif