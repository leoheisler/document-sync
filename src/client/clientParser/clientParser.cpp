#include "clientParser.h" 
#include "commandStatus.h"

//CONSTRUCTOR
clientParser::clientParser(/* args */){};


//PRIVATE FUNCTIONS
std::vector<std::string> clientParser::commandSplit(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream tokenStream(str);
    std::string token;

    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}


//PUBLIC FUNCTIONS
CommandStatus clientParser::verifyClientCommand(int argc, char* argv[]){
    CommandStatus status;

    if(argc == 3){
        status = CommandStatus::VALID;
    }else if(argc < 3){
        status = CommandStatus::TOO_MANY_ARGS;
    }else if(argc > 3){
        status = CommandStatus::TOO_MANY_ARGS;
    }

    return status;
}

CommandStatus clientParser::verifyClientCommand(std::string command){
    std::vector<std::string> comArgs = commandSplit(command,' ');
    std::string comm = comArgs[0];
    
    // verify if the command is valid
    if (comm != "list_server" && comm != "list_client" &&
        comm != "get_sync_dir" && comm != "exit" &&
        comm != "download" && comm != "upload" &&
        comm != "delete") {
        return CommandStatus::INVALID_COMMAND;
    }

    // verify if the command has too many arguments
    if (comArgs.size() > 2) {
        return CommandStatus::TOO_MANY_ARGS;
    }

    // verify if the command has just one argument besides the function
    if ((comm == "download" || comm == "upload" || comm == "delete") && comArgs.size() < 2) {
        return CommandStatus::TOO_MANY_ARGS;
    }

    // verify single commands
    if ((comm == "list_server" || comm == "list_client" || 
         comm == "get_sync_dir" || comm == "exit") && 
         comArgs.size() > 1) {
        return CommandStatus::TOO_MANY_ARGS;
    }

    return CommandStatus::VALID;
}
