#ifndef SERVER_FILEMANAGER_H
#define SERVER_FILEMANAGER_H
#include <string>
#include <filesystem>
#include <iostream>
class serverFileManager
{
    private: 

    public:
        serverFileManager();
        void create_server_sync_dir(std::string username);
};  
#endif