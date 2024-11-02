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
        int connectClientToServer(int argc, char* argv[]);
        void getSyncDir();

        // Getters & Setters
        std::string getUserName();
        void setUserName(std::string username);

};
#endif