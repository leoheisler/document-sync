#include "clientFileManager.h"

namespace fs = std::filesystem;
using namespace std;

//CONSTRUCTOR
clientFileManager::clientFileManager(){};
//PRIVATE

 void clientFileManager::print_paths(){
    if (this->paths.empty()) {
        std::cout << "Nenhum caminho disponível." << std::endl;
        return;
    }

    std::cout << "Lista de caminhos:" << std::endl;
    for (const auto& path : paths) {
        std::cout << "- " << path << std::endl;
    }
}

//PUBLIC
void clientFileManager::check_dir_updates(){
    char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event)))); // inotify buffer with events
    const struct inotify_event *event; // variable to extract inotify event from buffer
    ssize_t num_read; // number of bytes

    while (true) {
        // reading inotify buffer for events
        num_read = read(this->inotifyFd, buf, sizeof(buf));

        if (num_read <= 0) {
            //std::cout << "incorrect read inotify!" <<endl;
            break;
        }

        // event handling
        for (char *ptr = buf; ptr < buf + num_read; ) {
            event = (const struct inotify_event *) ptr;

            switch(event->mask)
            {
                // created/changed files event
                case IN_CLOSE_WRITE: {
                    string path = event->name;
                    path = "/" + path;

                    if(contains_path(path)){
                        cout << "caminho na lista:" + path << endl;
                    }
                    break;
                }
                
                // deleted files event
                case IN_DELETE: {
                    string path = event->name;
                    path = "/" + path;
                    cout << "deleted file: " + path;
                    break;
                }
            }

            // pointer skips to next event from buffer
            ptr += sizeof(struct inotify_event) + event->len;
        }
    }
}

void clientFileManager::create_client_sync_dir(){
    fs::path sync_path = "../src/client/sync_dir";
    try
    {
        if(fs::create_directory(sync_path)){
            std::cout << "Diretório criado com sucesso\n";
        }else{
            std::cout << "Diretório ja existente\n";
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "Não consegui criar o diretorio" << e.what() << '\n';
    }
    
}

std::string clientFileManager::erase_dir(std::string path){
    try {
        // Delete old sync dir from client to create new sync dir
        for (const auto& entry : fs::directory_iterator(path)) {
            if (fs::is_regular_file(entry)) {
                fs::remove(entry);
            }
        }
        return "Erased older directory.";
    } catch (const fs::filesystem_error& e) {
        return "Error when erasing directory.";
    }
}

std::string clientFileManager::delete_file(std::string file_path) {
    cout << "CHEGUEI AQUI NO DELETE FILE, com o file:" + file_path <<endl;
    try {
        if (fs::remove(file_path)) {
            return "File deleted successfully.\n";
        } else {
            return "File not found or unable to delete.\n";
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return "Something went wrong";
    }
}

void clientFileManager::add_path(string path){
    this->paths.push_back(path);
}

void clientFileManager::remove_path(string path){
    this->paths.erase(std::remove(this->paths.begin(), this->paths.end(), path), this->paths.end());
}

bool clientFileManager::contains_path(const std::string path){
    // Usa std::find para verificar se o caminho está na lista
    return std::find(this->paths.begin(),this-> paths.end(), path) != this->paths.end();
}
// Function: list_files
// Purpose: This function retrieves a list of file names from the "sync_dir" directory
//          and prints their modification, access, and change times (MAC times) where available.
//
// Returns:
//     A vector of strings containing the names of the files in the "sync_dir" directory.

std::vector<std::string> clientFileManager::list_files() {
    std::vector<std::string> files; // Vector to store the file names.
    std::string sync_dir = "../src/client/sync_dir"; // Directory path to scan for files.

    try {
        // Iterate over the entries in the specified directory.
        for (const auto& entry : std::filesystem::directory_iterator(sync_dir)) {
            // Check if the entry is a regular file.
            if (entry.is_regular_file()) {
                // Extract the file name and add it to the list.
                std::string filename = entry.path().filename().string();
                files.push_back(filename);

                // Get the modification time (mtime) of the file.
                auto ftime = entry.last_write_time();
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
                );

                // Convert the modification time to a time_t format for formatting.
                std::time_t mtime = std::chrono::system_clock::to_time_t(sctp);

                // Print the file name and its modification time.
                std::cout << "- " << filename << "\n";
                std::cout << "  mtime (Modification Time): " << std::put_time(std::localtime(&mtime), "%Y-%m-%d %H:%M:%S") << "\n";

                // For Unix systems, attempt to retrieve and print access time (atime)
                // and change time (ctime) using the `stat` system call.
                #ifdef __unix__
                struct stat file_stat;
                if (stat(entry.path().c_str(), &file_stat) == 0) {
                    std::time_t atime = file_stat.st_atime; // Access Time.
                    std::time_t ctime = file_stat.st_ctime; // Change Time.

                    // Print the access and change times.
                    std::cout << "  atime (Access Time): " << std::put_time(std::localtime(&atime), "%Y-%m-%d %H:%M:%S") << "\n";
                    std::cout << "  ctime (Change Time): " << std::put_time(std::localtime(&ctime), "%Y-%m-%d %H:%M:%S") << "\n";
                }
                #endif
            }
        }
    } 
    // Handle any errors that occur while accessing the directory.
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error accessing sync_dir: " << e.what() << std::endl;
    }

    // Return the list of file names.
    return files;
}
