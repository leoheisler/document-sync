#ifndef SERVERCOMMANAGER_H
#define SERVERCOMMANAGER_H
#include <string>
#include <vector>
#include <sstream>

class serverComManager
{
    private: 
        // empty
    public:
        // Constructor Methods
        serverComManager(/* args */);

        // Communication Methods
        int connect_server_to_client(int argc, char* argv[]);
};
#endif