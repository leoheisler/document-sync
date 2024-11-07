#include "serverFileManager.h"


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