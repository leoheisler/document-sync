#ifndef CLIENTCOMMANAGER_H
#define CLIENTCOMMANAGER_H
#include <string>
#include <vector>
#include <sstream>

class clientComManager
{
    private: 
        std::string username;
    public:
        // Constructor Method
        clientComManager(/* args */);

        // Communication Methods
        int connect_client_to_server(int argc, char* argv[]);
        void get_sync_dir();

        // Getters & Setters
        std::string get_username();
        void set_username(std::string username);

};
#endif