#include "clientFileManager.h"


namespace fs = std::filesystem;
//CONSTRUCTOR
clientFileManager::clientFileManager(){};

//PUBLIC
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