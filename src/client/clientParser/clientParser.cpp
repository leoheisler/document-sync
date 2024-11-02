#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "clientParser.h" 
#include "commandStatus.h"

// CONSTRUCTOR
clientParser::clientParser(/* args */){};


// PRIVATE METHODS
std::vector<std::string> clientParser::commandSplit(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream tokenStream(str);
    std::string token;

    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

// PUBLIC METHODS
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

    this->command = command;
    return CommandStatus::VALID;
}
