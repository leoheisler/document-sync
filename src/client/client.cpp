#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include "clientComManager.h"
#include "clientParser.h" 
#include "commandStatus.h"
#include "clientFileManager.h"
#include "packet.h"

using namespace std;

class client
{
    private:
        clientFileManager file_manager;
        clientParser parser;
        clientComManager communication_manager;
        bool exit = false;
        
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

        void command_input_interface(){
            while(!exit){
                cout << "\n\nEscolha uma das opções abaixo:\n";
                cout << "1. upload <path/filename.ext> - Envia o arquivo para o servidor e sincroniza com os dispositivos.\n";
                cout << "2. download <filename.ext> - Baixa uma cópia não sincronizada do arquivo do servidor.\n";
                cout << "3. delete <filename.ext> - Exclui o arquivo do diretório \"sync_dir\" no servidor.\n";
                cout << "4. list_server - Lista os arquivos salvos no servidor associados ao usuário.\n";
                cout << "5. list_client - Lista os arquivos salvos no diretório \"sync_dir\" local.\n";
                cout << "6. exit - Fecha a sessão com o servidor.\n";

                Command command;
                int option;
                cout << "\nDigite o número da opção desejada: ";
                cin >> option;
                switch(option){
                    case 1: command = Command::UPLOAD; break;  
                    case 2: command = Command::DOWNLOAD; break;
                    case 3: command = Command::DELETE; break;
                    case 4: command = Command::LIST_SERVER; break;
                    case 5: command = Command::LIST_CLIENT; break;
                    case 6: command = Command::EXIT; exit = true; break;
                    case 7: command = Command::GET_SYNC_DIR; break;
                    default: command = Command::NO_COMMAND; break;
                }
                cout << communication_manager.execute_command(command) << endl;
            }
        }

        void upload_to_server()
        {
            // inotify for sync
            while(!exit){

            }
        }

        void download_from_server()
        {
            while(!exit){
                communication_manager.await_sync();
            }
        }

    public:
        client(){
            // Configurando o file_manager no communication_manager
            communication_manager.set_file_manager(&file_manager);
        };
        void start(int argc, char* argv[]){
            //create sync_dir and connect sockets
            file_manager.create_client_sync_dir();
            communication_manager.connect_client_to_server(argc,argv);
            cout << flush;

            // create threads
            thread command_thread(&client::command_input_interface, this);
            thread upload_thread(&client::upload_to_server, this);
            thread download_thread(&client::download_from_server, this);

            // wait for all threads to finish so main can finish
            command_thread.join();
            upload_thread.join();
            download_thread.join();
        }
};

int main(int argc, char* argv[]){
    client client;
    client.start(argc,argv);   
}

