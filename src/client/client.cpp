#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <thread>

#include "clientFileManager.h"
#include "clientComManager.h"
#include "commandStatus.h"
#include "packet.h"

using namespace std;
#define PORT 1909
class client
{
    private:
        clientFileManager *file_manager;
        clientComManager communication_manager;

        void command_input_interface(){
            while(true){
                cout << "\n\nEscolha uma das opções abaixo:\n";
                cout << "1. upload <path/filename.ext> - Envia o arquivo para o servidor e sincroniza com os dispositivos.\n";
                cout << "2. download <filename.ext> - Baixa uma cópia não sincronizada do arquivo do servidor.\n";
                cout << "3. delete <filename.ext> - Exclui o arquivo do diretório \"sync_dir\" local.\n";
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
                    case 6: command = Command::EXIT; break;
                    case 7: command = Command::GET_SYNC_DIR; break;
                    default: command = Command::NO_COMMAND; break;
                }
                communication_manager.execute_command(command);
            }
        }
        /*
            THIS FUNCTION BINDS THE LISTENING SOCKET FOR REVERSE CONNECTION

        */
        void bind_client_socket(int* listening_socket){
            struct sockaddr_in cli_addr;
            if ((*listening_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                throw std::runtime_error("ERRO ABRINDO O SOCKET");
            }
                
            cli_addr.sin_family = AF_INET;
            cli_addr.sin_port = htons(PORT);
            cli_addr.sin_addr.s_addr = INADDR_ANY;
            bzero(&(cli_addr.sin_zero), 8);     
            if (bind(*listening_socket, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0){
                throw std::runtime_error("ERRO BINDANDO O SOCKET");
            } 
        }

        void upload_to_server()
        {
            // Inotify for client->server sync
            while(true){
                file_manager->check_dir_updates();
            }
        }

        void download_from_server()
        {
            // Receive loop for server->client sync
            while(true){
                communication_manager.await_sync();
            }
        }

        /*
            THIS FUNCTION ACCEPTS ALL CONNECTION ON PORT 1909,
            it >>WILL<< change the sockets once it listens to 
            >>ANY<< comunnication
        */
        void accept_connections(){
            int listening_socket;
            int first_contact_socket, second_contact_socket, third_contact_socket;
            struct sockaddr_in client_address;
            socklen_t client_len = sizeof(struct sockaddr_in);
            
            try{
                bind_client_socket(&listening_socket);
            }catch(const std::exception& e){
                std::cerr << e.what() << '\n';
            }
            
            listen(listening_socket,3);

            while(true){
                first_contact_socket = accept(listening_socket,(struct sockaddr*)&client_address,&client_len);
                if(first_contact_socket >= 0){
                    communication_manager.close_sockets();
                    try{
                        second_contact_socket = accept(listening_socket,(struct sockaddr*)&client_address,&client_len);
                        third_contact_socket = accept(listening_socket,(struct sockaddr*)&client_address,&client_len);
                        communication_manager.set_sock_cmd(first_contact_socket);
                        communication_manager.set_sock_upload(second_contact_socket);
                        communication_manager.set_sock_fetch(third_contact_socket);
                    }catch(const std::exception& e){
                        std::cerr << e.what() << '\n';
                    }
                }
            }
        }

    public:
        client(){
            file_manager = new clientFileManager;// Configurando o file_manager no communication_manager
        };
        void start(int argc, char* argv[]){
            //create sync_dir and connect sockets
            file_manager->create_client_sync_dir();
            communication_manager.set_file_manager(file_manager);
            communication_manager.connect_client_to_server(argc,argv);
            cout << flush;

            // create threads
            thread command_thread(&client::command_input_interface,this);
            thread upload_thread(&client::upload_to_server,this);
            thread download_thread(&client::download_from_server,this);
            thread accept_thread(&client::accept_connections,this);

            // wait for all threads to finish so main can finish
            command_thread.join();
            upload_thread.join();
            download_thread.join();
            accept_thread.join();
        }
};

int main(int argc, char* argv[]){
    
    client client;
    client.start(argc,argv);   
}

