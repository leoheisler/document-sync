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

std::vector<std::string> clientFileManager::list_files() {
    std::vector<std::string> files;
    std::string sync_dir = "../src/client/sync_dir";

    try {
        for (const auto& entry : std::filesystem::directory_iterator(sync_dir)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                files.push_back(filename);

                // Obter tempos MAC
                auto ftime = entry.last_write_time();
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
                );

                std::time_t mtime = std::chrono::system_clock::to_time_t(sctp);

                std::cout << "- " << filename << "\n";
                std::cout << "  mtime (Modification Time): " << std::put_time(std::localtime(&mtime), "%Y-%m-%d %H:%M:%S") << "\n";

                // Em sistemas Unix, ctime e atime são acessíveis via system calls.
                #ifdef __unix__
                struct stat file_stat;
                if (stat(entry.path().c_str(), &file_stat) == 0) {
                    std::time_t atime = file_stat.st_atime; // Access Time
                    std::time_t ctime = file_stat.st_ctime; // Change Time

                    std::cout << "  atime (Access Time): " << std::put_time(std::localtime(&atime), "%Y-%m-%d %H:%M:%S") << "\n";
                    std::cout << "  ctime (Change Time): " << std::put_time(std::localtime(&ctime), "%Y-%m-%d %H:%M:%S") << "\n";
                }
                #endif
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error accessing sync_dir: " << e.what() << std::endl;
    }

    return files;
}