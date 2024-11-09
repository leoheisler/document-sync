#ifndef CLIENTPARSER_H
#define CLIENTPARSER_H
#include <string>
#include <vector>
#include <sstream>
#include <commandStatus.h>

class clientParser
{
    private: 
        std::string command;
        std::vector<std::string> command_split(const std::string& str, char delimiter);
    public:
        // Constructor Method
        clientParser(/* args */);

        // Command Methods
        CommandStatus verify_client_command(std::string command);
        bool execute_command(std::string command);
        Command get_command_from_string(const std::string& commandStr);
};
#endif