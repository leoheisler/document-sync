#ifndef CLIENTCOMMANAGER_H
#define CLIENTCOMMANAGER_H
#include <string>
#include <vector>
#include <sstream>

class clientComManager
{
    private: 
        // empty
    public:
        // Constructor Methods
        clientComManager(/* args */);

        // Communication Methods
        int connectClientToServer(int argc, char* argv[]);
};
#endif