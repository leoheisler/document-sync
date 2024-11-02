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
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	
    char buffer[256];
    if (argc < 2) {
		fprintf(stderr,"usage %s hostname\n", argv[0]);
		exit(0);
    }
	
	server = gethostbyname(argv[1]);
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket\n");
    
	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(PORT);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);     
	
    
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        printf("ERROR connecting\n");

    printf("Enter the message: ");
    bzero(buffer, 256);
    fgets(buffer, 256, stdin);
    
	/* write in the socket */
	n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) 
		printf("ERROR writing to socket\n");

    bzero(buffer,256);
	
	/* read from the socket */
    n = read(sockfd, buffer, 256);
    if (n < 0) 
		printf("ERROR reading from socket\n");

    printf("%s\n",buffer);
    
	close(sockfd);
    return 0;
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
