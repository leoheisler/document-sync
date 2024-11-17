#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "clientComManager.h"
#include "clientParser.h" 
#include "commandStatus.h"
#include "clientFileManager.h"

using namespace std;

class client
{
    private:
        bool valid_command_status(CommandStatus status){
            switch (status) {
                case CommandStatus::VALID:
                    return true;
                case CommandStatus::TOO_MANY_ARGS:
                    cout << "Too many arguments.\n";
                    return false;
                case CommandStatus::TOO_FEW_ARGS:
                    cout << "Too few arguments.\n";
                    return false;
                default:
                    return false;
            }
        }

        Command command_input_interface(){
            cout << "\nEscolha uma das opções abaixo:\n";
            cout << "1. upload <path/filename.ext> - Envia o arquivo para o servidor e sincroniza com os dispositivos.\n";
            cout << "2. download <filename.ext> - Baixa uma cópia não sincronizada do arquivo do servidor.\n";
            cout << "3. delete <filename.ext> - Exclui o arquivo do diretório \"sync_dir\" no servidor.\n";
            cout << "4. list_server - Lista os arquivos salvos no servidor associados ao usuário.\n";
            cout << "5. list_client - Lista os arquivos salvos no diretório \"sync_dir\" local.\n";
            cout << "6. exit - Fecha a sessão com o servidor.\n";
            cout << "7. get_sync_dir" <<std::endl;

            int option;
            cout << "\nDigite o número da opção desejada: ";
            cin >> option;
            switch(option){
                case 1: return Command::UPLOAD; 
                case 2: return Command::DOWNLOAD;
                case 3: return Command::DELETE;
                case 4: return Command:: LIST_SERVER;
                case 5: return Command::LIST_CLIENT;
                case 6: return Command::EXIT;
                case 7: return Command::GET_SYNC_DIR;
                default: return Command::EXIT;
            }
        }

    public:
        client(){};
        void start(int argc, char* argv[]){
            clientFileManager file_manager;
            clientParser parser;
            clientComManager communication_manager;
            string response_string,command_string;
            Command command;

            file_manager.create_client_sync_dir();
            bool exit = false;     
            communication_manager.connect_client_to_server(argc,argv);
            while(true){
                command = command_input_interface();
                cout << communication_manager.execute_command(command) << endl;
            }
        }
};

int main(int argc, char* argv[]){
    client client;
    client.start(argc,argv);   
}

