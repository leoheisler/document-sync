#ifndef CLIENT_FILEMANAGER_H
#define CLIENT_FILEMANAGER_H
#include <string>
#include <filesystem>
#include <iostream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sys/stat.h>
#include <sys/types.h>

class clientFileManager
{
    private: 

    public:
        clientFileManager();
        void create_client_sync_dir();
        std::vector<std::string> list_files();
};  
#endif

