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
        std::vector<std::string> commandSplit(const std::string& str, char delimiter);
    public:
        // Constructor Method
        clientParser(/* args */);

        // Command Methods
        CommandStatus verifyClientCommand(std::string command);
        bool executeCommand(std::string command);
};
#endif