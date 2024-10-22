#include "clientParser.h" 

#include <iostream>
class myClient
{
public:
    myClient(/* args */){};
    void start(int argc, char* argv[]){
       
        clientParser parser;
        std::string responseString,commandString;
        bool exit = false;

        responseString = parser.verifyClientCommand(argc,argv);
        do{
            if (responseString == "ok"){
                /* code */
            }else{
                std::cout<<responseString;
            }
            std::cin >> commandString;
            responseString = parser.verifyClientCommand(commandString);
        }while(!exit);
  
    }
};

int main(int argc, char* argv[]){
    myClient client;
    client.start(argc,argv);   
}

