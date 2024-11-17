#include "serverFileManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>



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