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
class clientFileManager
{
    private: 
        const char *SYNC_DIR= "../src/client/sync_dir";
        int inotifyFd = inotify_init(); // inotify buffer stores warnings from watchman
        int watchman = inotify_add_watch(inotifyFd, SYNC_DIR, IN_CLOSE_WRITE | IN_DELETE ); // watchman warns inotify when changes occur
    public:
        clientFileManager();
        void create_client_sync_dir();
        std::vector<std::string> list_files();
        static std::string erase_dir(std::string path);
        void check_dir_updates();
};  
#endif

