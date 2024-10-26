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
    //constructor
    clientParser(/* args */);

    //command verification functions
    CommandStatus verifyClientCommand(int argc, char* argv[]);
    CommandStatus verifyClientCommand(std::string command);
};
#endif