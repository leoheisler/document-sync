#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "clientComManager.h"
#include "clientParser.h" 
#include "commandStatus.h"
#include "clientFileManager.h"
class client
{
    private:
        void check_command_status(CommandStatus status){
            switch (status) {
                case CommandStatus::VALID:
                    break;
                case CommandStatus::INVALID_COMMAND:
                    std::cout << "Invalid command.\n";
                    break;
                case CommandStatus::TOO_MANY_ARGS:
                    std::cout << "Too many arguments.\n";
                    break;
                case CommandStatus::TOO_FEW_ARGS:
                    std::cout << "Too few arguments.\n";
                    break;
            }
        }

    public:
        client(){};
        void start(int argc, char* argv[]){
            clientFileManager file_manager;
            clientParser parser;
            clientComManager communication_manager;
            std::string response_string,command_string;
            CommandStatus command_status;

            file_manager.create_client_sync_dir();
            bool exit = false;     
            communication_manager.connect_client_to_server(argc,argv);
            do{
                check_command_status(command_status);
                std::cin >> command_string;
                command_status = parser.verify_client_command(command_string);
            }while(!exit);
    
        }
};

int main(int argc, char* argv[]){
    client client;
    client.start(argc,argv);   
}

