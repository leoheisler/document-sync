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

        // Ponteiro para file_manager, inicialmente nulo
        clientFileManager* file_manager = nullptr;

        void get_sync_dir();
        void receive_sync_dir_files();
        void list_server();
        void receive_list_server_times();
        void download(std::string file_name);
        void start_sockets();
        void connect_sockets(int port, hostent* server);
        void close_sockets();
    public:
        // Constructor Method
        clientComManager(/* args */);

        // Send packet based on user request
        std::string execute_command(Command command);

        // Communication Methods
        int connect_client_to_server(int argc, char* argv[]);
        void await_sync();
        
        // Getters & Setters
        std::string get_username();
        void set_username(std::string username);

        // Setter, para poder chamar o list_files dentro do ComManager
        void set_file_manager(clientFileManager* fm); 

};
#endif