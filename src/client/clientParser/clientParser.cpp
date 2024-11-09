#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "clientParser.h" 
#include "commandStatus.h"

// CONSTRUCTOR
clientParser::clientParser(/* args */){};


// PRIVATE METHODS
std::vector<std::string> clientParser::command_split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream token_stream(str);
    std::string token;

    while (std::getline(token_stream, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

// PUBLIC METHODS
CommandStatus clientParser::verify_client_command(std::string command){
    std::vector<std::string> com_args = command_split(command,' ');
    std::string comm = com_args[0];
    
    // verify if the command is valid
    if (comm != "list_server" && comm != "list_client" &&
        comm != "get_sync_dir" && comm != "exit" &&
        comm != "download" && comm != "upload" &&
        comm != "delete") {
        return CommandStatus::INVALID_COMMAND;
    }

    // verify if the command has too many arguments
    if (com_args.size() > 2) {
        return CommandStatus::TOO_MANY_ARGS;
    }

    // verify if the command has just one argument besides the function
    if ((comm == "download" || comm == "upload" || comm == "delete") && com_args.size() < 2) {
        return CommandStatus::TOO_MANY_ARGS;
    }

    // verify single commands
    if ((comm == "list_server" || comm == "list_client" || 
         comm == "get_sync_dir" || comm == "exit") && 
         com_args.size() > 1) {
        return CommandStatus::TOO_MANY_ARGS;
    }

    this->command = command;
    return CommandStatus::VALID;
}

// gets command from a string
Command clientParser::get_command_from_string(const std::string& commandStr) {
    if (commandStr == "get_sync_dir") {
        return Command::GET_SYNC_DIR;
    } else if (commandStr == "list_server") {
        return Command::LIST_SERVER;
    } else if (commandStr == "list_client") {
        return Command::LIST_CLIENT;
    } else if (commandStr == "exit") {
        return Command::EXIT;
    } else if (commandStr == "download") {
        return Command::DOWNLOAD;
    } else if (commandStr == "upload") {
        return Command::UPLOAD;
    } else if (commandStr == "delete") {
        return Command::DELETE;
    }

    // Default case (if no matching command is found)
    return static_cast<Command>(0);
}

