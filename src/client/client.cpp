#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "clientComManager.h"
#include "clientParser.h" 
#include "commandStatus.h"

class client
{
    private:
        std::string username;
        void checkCommandStatus(CommandStatus status){
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
        client(/* args */){};
        void start(int argc, char* argv[]){
            clientParser parser;
            clientComManager communicationManager;
            std::string responseString,commandString;
            CommandStatus commandStatus;
            bool exit = false;     

            communicationManager.connectClientToServer(argc,argv);
            do{
                checkCommandStatus(commandStatus);
                std::cin >> commandString;
                commandStatus = parser.verifyClientCommand(commandString);
            }while(!exit);
    
        }
};

int main(int argc, char* argv[]){
    client client;
    client.start(argc,argv);   
}

