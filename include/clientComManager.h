#ifndef CLIENTCOMMANAGER_H
#define CLIENTCOMMANAGER_H
#include <string>
#include <vector>
#include <sstream>
#include <netdb.h>
#include <clientFileManager.h>
#include "commandStatus.h"
class clientComManager
{
    private: 
        std::string username;
        int sock_cmd = -1;
        int sock_upload = -1;
        int sock_fetch = -1;

        void get_sync_dir();
        void start_sockets();
        void connect_sockets(int port, hostent* server);
        void close_sockets();
    public:
        // Constructor Method
        clientComManager(/* args */);

        // Send packet based on user request
        std::string send_request_to_server(Command command);

        // Communication Methods
        int connect_client_to_server(int argc, char* argv[]);
        
        // Getters & Setters
        std::string get_username();
        void set_username(std::string username);
        void receive_sync_dir_files();

};
#endif