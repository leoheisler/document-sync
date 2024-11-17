#ifndef SERVER_FILEMANAGER_H
#define SERVER_FILEMANAGER_H
#include <string>
#include <filesystem>
#include <vector> 
#include <iostream>
class serverFileManager
{
    private: 

    public:
        serverFileManager();
        void create_server_sync_dir(std::string username);

        // Function to retrieve all paths inside user synchronized directory
        std::vector<std::string> get_sync_dir_paths(const std::string& username);
};  
#endif