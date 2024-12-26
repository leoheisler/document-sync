#include "serverFileManager.h"
#include "packet.h"
#include "fileTransfer.h"
#include <dirent.h>
#include <sys/stat.h>

namespace fs = std::filesystem;
//CONSTRUCTOR
serverFileManager::serverFileManager(){};
//PUBLIC
void serverFileManager::create_server_sync_dir(std::string username){
    fs::path sync_path = "../src/server/userDirectories/sync_dir_" + username;
    try
    {
        if(fs::create_directory(sync_path)){
            std::cout << "Diretório criado com sucesso no servidor para o user: " + username + "\n";
        }else{
            std::cout << "Diretório não foi criado para o user: " + username  + "\n";
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "Não consegui criar o diretorio" << e.what() << '\n';
    }
    
}

std::string serverFileManager::delete_file(std::string file_path) {
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

// Function to retrieve all paths inside user synchronized directory
std::vector<std::string> serverFileManager::get_sync_dir_paths(const std::string& username) {
  std::vector<std::string> paths; // vector to be returned.
  std::string baseDir = "../src/server/userDirectories/sync_dir_" + username;

  // Check if directory exists and iterate over files
  if (fs::exists(baseDir) && fs::is_directory(baseDir)) {
    for (const auto& entry : fs::recursive_directory_iterator(baseDir)) {
      if (fs::is_regular_file(entry)) {
        paths.push_back(entry.path().string()); // Add file path to vector
      }
    }
  } else {
    std::cerr << "Directory does not exist: " << baseDir << std::endl;
  }

  return paths; // Return the vector of file paths
}

std::vector<std::string> serverFileManager::get_sync_dir_files_in_directory(const std::string& directory_path)
{
    std::vector<std::string> file_paths;

    // Open the directory
    DIR* dir = opendir(directory_path.c_str());
    if (dir == nullptr) {
        std::cerr << "Failed to open directory: " << directory_path << std::endl;
        return file_paths;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // Skip current (.) and parent (..) directories
        if (entry->d_name[0] == '.') continue;

        std::string full_path = directory_path + "/" + entry->d_name;

        struct stat statbuf;
        if (stat(full_path.c_str(), &statbuf) == 0) {
            // If it's a regular file, add it to the list
            if (S_ISREG(statbuf.st_mode)) {
                file_paths.push_back(full_path);
            }
            // If it's a directory, recurse into it
            else if (S_ISDIR(statbuf.st_mode)) {
                std::vector<std::string> subdirectory_files = get_sync_dir_files_in_directory(full_path);
                file_paths.insert(file_paths.end(), subdirectory_files.begin(), subdirectory_files.end());
            }
        }
    }

    closedir(dir);
    return file_paths;
}

void serverFileManager::receive_sync_dir_files(int socket)
{
    int client_socket = socket;

    while (true) {
        // Receive a packet
        Packet received_packet = Packet::receive_packet(client_socket);

        // Extract the payload
        std::string payload(received_packet.get_payload(), received_packet.get_length());

        // Split the payload to extract path, total paths, and index
        std::istringstream payload_stream(payload);
        std::string path;
        int total_paths = 0, index = 0;

        if (std::getline(payload_stream, path, '\n') &&
            payload_stream >> total_paths && 
            payload_stream >> index) {
            // Log received information
            // std::cout << "Received path: " << path << " (Index " << index << " of " << total_paths << ")" << std::endl;

            // Receive the file using the extracted path
            FileTransfer::receive_file(path, client_socket);

            // Check if all paths are received
            if (index + 1 == total_paths) {
                //std::cout << "All files received." << std::endl;
                break;
            }
        } else {
            std::cerr << "Error: Invalid payload format." << std::endl;
            break;
        }
    }
    return;
}