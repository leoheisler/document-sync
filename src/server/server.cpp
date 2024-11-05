#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "serverComManager.h" 

#define PORT 4000

int main(int argc, char *argv[])
{
	serverComManager serverCommunicationManager;

	serverCommunicationManager.connect_server_to_client(argc,argv);
	return 0;
}

