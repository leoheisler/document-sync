#ifndef COMMANAGER_H
#define COMMANAGER_H
#include <string>
#include <vector>
#include <sstream>

class comManager
{
    private: 
        // empty
    public:
        // Constructor Methods
        comManager(/* args */);

        // Communication Methods
        int connectClientToServer(int argc, char* argv[]);
};
#endif