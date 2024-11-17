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

