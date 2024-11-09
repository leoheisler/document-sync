#ifndef CLIENTLIST_H
#define CLIENTLIST_H

#include <string>
#include <iostream>
#include <tuple>
using namespace std;

class ClientNode {
    private:
        std::string username;  // Username of client user
        int socket_device1;    // Socket for device 1
        int socket_device2;    // Socket for device 2
        ClientNode* next;      // Pointer to the next node

    public:
        // Constructor with default values for sockets
        ClientNode(const string& uname, int sock1 = 0, int sock2 = 0)
            : username(uname), socket_device1(sock1), socket_device2(sock2), next(nullptr) {}

        // Getters for username and socket information
        const string& get_username() const { return username; }
        int get_socket_device1() const { return socket_device1; }
        int get_socket_device2() const { return socket_device2; }

        void set_socket_device1(int socket) { socket_device1 = socket; }
        void set_socket_device2(int socket) { socket_device2 = socket; }
        ClientNode* get_next() const { return next; }
        void set_next(ClientNode* next_node) { next = next_node; }

        // Display client info (for debugging)
        void display() const {
            std::cout << "Client: " << username << endl
                    << " Device 1 Socket: " << socket_device1 << endl
                    << " Device 2 Socket: " << socket_device2 << endl;
        }
};

class ClientList {
    private:
        ClientNode* head;  // Head of the linked list

    public:
        ClientList() : head(nullptr) {}

        // Method to add a new device to the list
        void add_device(const string& uname, int device_sock) {
            ClientNode* current = head;
            bool isConnected = true;
            // Search for the client node by username
            while (current != nullptr) {
                if (current->get_username() == uname) {
                    // Check if there is a free socket for the new device
                    if (current->get_socket_device1() == 0) {
                        current->set_socket_device1(device_sock);                      
                    } else if (current->get_socket_device2() == 0) {
                        current->set_socket_device2(device_sock);  
                    } else {
                        throw std::string("CLIENT FULL");
                    }
                    return;
                }
                current = current->get_next();  // Move to the next node
            }

            // If we reach here, the client was not found, so add a new client
            ClientNode* new_client = new ClientNode(uname, device_sock);

            // Insert the new client node at the end of the list
            if (head == nullptr) {
                head = new_client;  
            } else {
                ClientNode* temp = head;
                while (temp->get_next() != nullptr) {
                    temp = temp->get_next();  // Move to the end of the list
                }
                temp->set_next(new_client);  // Link the new client at the end
            }
        }

        // Display all clients in the list (for debugging)
        void display_clients() const {
            ClientNode* current = head;
            while (current != nullptr) {
                current->display();
                current = current->get_next();
            }
        }
};

#endif // CLIENTLIST_H
