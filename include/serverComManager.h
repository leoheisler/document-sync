#ifndef SERVERCOMMANAGER_H
#define SERVERCOMMANAGER_H
#include <string>
#include <vector>
#include <sstream>

#include "clientList.h"
#include "packet.h"

class serverComManager
{
    private: 
        // Linked list to store client infos 
        ClientList clientDeviceList;

        // Aux methods
        void evaluate_command(Packet command_packet, int socket);

        // Communication Methods
        void await_packets(int socket);
        void create_sync_dir(Packet sync_dir_packet, int socket);
    public:
        // Constructor Method
        serverComManager();

        // Start Connection Method
        int connect_server_to_client(int argc, char* argv[]);
};
#endif