#ifndef CLIENTPARSER_H
#define CLIENTPARSER_H
#include <string>
#include <vector>
#include <sstream>

class clientParser
{
private:
    std::vector<std::string> commandSplit(const std::string& str, char delimiter);
public:
    //constructor
    clientParser(/* args */);

    //command verification functions
    std::string verifyClientCommand(int argc, char* argv[]);
    std::string verifyClientCommand(std::string command);
};
#endif