#ifndef CLIENTCOMMANAGER_H
#define CLIENTCOMMANAGER_H
#include <string>
#include <vector>
#include <sstream>
#include <netdb.h>
#include <clientFileManager.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <filesystem>



#include "commandStatus.h"
#include "fileTransfer.h"
#include "packet.h"

class clientComManager
{
    private: 
        std::string username;
        std::string hostname;
        int sock_cmd = -1;
        int sock_upload = -1;
        int sock_fetch = -1;

        // filemanager reference
        clientFileManager* file_manager = nullptr;

        //Sockets
        void start_sockets();
        void connect_sockets(int port, hostent* server);
        void close_sockets();

        //sync_dir funcs
        void get_sync_dir();
        void receive_sync_dir_files();

        //list_server
        void list_server();
        void receive_list_server_times();

        // other commands
        void upload();
        void download();
        void delete_file();
        void list_client();
        void exit_client();
    
    public:
        // Constructor Method
        clientComManager();

        // Delegate method based on command requested by user
        void execute_command(Command command);

        // Communication Methods
        int connect_client_to_server(int argc, char* argv[]);
        void await_sync();
        
        // Getters & Setters
        std::string get_username();
        void set_username(std::string username);
        std::string get_hostname();
        void set_hostname(std::string hostname);


        void send_delete_request(std::string file_name);

        // Sett client file handler
        void set_file_manager(clientFileManager* fm); 

};
#endif