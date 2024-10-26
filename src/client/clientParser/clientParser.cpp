
#include "clientParser.h" 

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
std::string clientParser::verifyClientCommand(int argc, char* argv[]){
    std::string returnString;
    if(argc == 3){
        returnString = "ok";
    }else if(argc < 3){
        returnString = "too few arguments\n";
    }else if(argc > 3){
        returnString = "too many arguments\n";
    }

    return returnString;
}

std::string clientParser::verifyClientCommand(std::string command){
    std::vector<std::string> comArgs = commandSplit(command,' ');
    std::string comm = comArgs[0];
    
    // verify if the command is valid
    if (comm != "list_server" && comm != "list_client" &&
        comm != "get_sync_dir" && comm != "exit" &&
        comm != "download" && comm != "upload" &&
        comm != "delete") {
        return "Invalid command.\n";
    }

    // verify if the command has too many arguments
    if (comArgs.size() > 2) {
        return "Too many arguments.\n";
    }

    // verify if the command has just one argument besides the function
    if ((comm == "download" || comm == "upload" || comm == "delete") && comArgs.size() < 2) {
        return "Too few arguments.\n";
    }

    // verify single commands
    if ((comm == "list_server" || comm == "list_client" || 
         comm == "get_sync_dir" || comm == "exit") && 
         comArgs.size() > 1) {
        return "Too many arguments.\n";
    }

    return "ok";
   
}





