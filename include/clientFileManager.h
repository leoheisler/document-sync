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
#include <sys/inotify.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <algorithm> 
#include "packet.h"
#include "commandStatus.h"
#include "fileTransfer.h"

class clientFileManager
{
    private: 
        std::vector<std::string> paths;
        int sock_cmd = -1, sock_upload = -1, sock_fetch = -1;
        const char *SYNC_DIR= "../src/client/sync_dir";
        int inotifyFd = inotify_init(); // inotify buffer stores warnings from watchman
        int watchman = inotify_add_watch(inotifyFd, SYNC_DIR, IN_CLOSE_WRITE | IN_DELETE ); // watchman warns inotify when changes occur
        void print_paths();
    public:
        clientFileManager();
        void create_client_sync_dir();
        std::vector<std::string> list_files();
        std::string erase_dir(std::string path);
        std::string delete_file(std::string file_path);
        void check_dir_updates();
        void add_path(std::string path);
        void remove_path(std::string path);
        bool contains_path(const std::string path);
        void set_sockets(int sock_cmd, int sock_upload, int sock_fetch){this->sock_cmd = sock_cmd; this->sock_fetch = sock_fetch; this->sock_upload = sock_upload;};
        
};  
#endif

