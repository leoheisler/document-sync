#ifndef SERVER_FILEMANAGER_H
#define SERVER_FILEMANAGER_H
#include <string>
#include <filesystem>
#include <vector> 
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>

class serverFileManager
{
    private: 

    public:
        serverFileManager();
        static void create_server_sync_dir(std::string username);

        // Function to retrieve all paths inside user synchronized directory
        static std::vector<std::string> get_sync_dir_paths(const std::string& username);
        static std::string delete_file(std::string file_path);
};  
#endif