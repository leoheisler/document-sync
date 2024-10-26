#include "clientParser.h" 
#include "commandStatus.h"

#include <iostream>
class myClient
{
    private:
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
        myClient(/* args */){};
        void start(int argc, char* argv[]){
        
            clientParser parser;
            std::string responseString,commandString;
            CommandStatus commandStatus;
            bool exit = false;     

            commandStatus = parser.verifyClientCommand(argc,argv);
            do{
                checkCommandStatus(commandStatus);
                std::cin >> commandString;
                commandStatus = parser.verifyClientCommand(commandString);
            }while(!exit);
    
        }
};

int main(int argc, char* argv[]){
    myClient client;
    client.start(argc,argv);   
}

