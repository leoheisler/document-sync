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
        bool valid_command_status(CommandStatus status){
            switch (status) {
                case CommandStatus::VALID:
                    return true;
                case CommandStatus::INVALID_COMMAND:
                    std::cout << "Invalid command.\n";
                    return false;
                case CommandStatus::TOO_MANY_ARGS:
                    std::cout << "Too many arguments.\n";
                    return false;
                case CommandStatus::TOO_FEW_ARGS:
                    std::cout << "Too few arguments.\n";
                    return false;
                case CommandStatus::NO_COMMAND:
                    return false;
                default:
                    return false;
            }
        }

    public:
        client(){};
        void start(int argc, char* argv[]){
            clientFileManager file_manager;
            clientParser parser;
            clientComManager communication_manager;
            std::string response_string,command_string;
            CommandStatus command_status = CommandStatus::NO_COMMAND;
            Command command;

            file_manager.create_client_sync_dir();
            bool exit = false;     
            communication_manager.connect_client_to_server(argc,argv);
            do{
                if (valid_command_status(command_status)){
                    // call parser to identify command 
                    command = parser.get_command_from_string(command_string);
                    // call communication manager to request from server
                    std::cout << communication_manager.send_request_to_server(command) << std::endl;
                }
                std::cin >> command_string;
                command_status = parser.verify_client_command(command_string);
            }while(!exit);
    
        }
};

int main(int argc, char* argv[]){
    client client;
    client.start(argc,argv);   
}

