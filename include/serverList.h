#ifndef SERVERLIST_H
#define SERVERLIST_H

#include <iostream>

class ServerNode {
private:
    int socket;             // Socket number of the server
    ServerNode* next;       // Pointer to the next node

public:
    // Constructor
    ServerNode(int sock) : socket(sock), next(nullptr) {}

    // Getters and Setters
    int get_socket() const { return socket; }
    void set_socket(int sock) { socket = sock; }

    ServerNode* get_next() const { return next; }
    void set_next(ServerNode* next_node) { next = next_node; }

    // Display server info (for debugging)
    void display() const {
        std::cout << "Server Socket: " << socket << std::endl;
    }
};

class ServerList {
private:
    ServerNode* head;  // Head of the linked list

public:
    // Constructor
    ServerList() : head(nullptr) {}

    // Method to add a server to the list
    void add_server(int socket) {
        ServerNode* new_server = new ServerNode(socket);
        if (head == nullptr) {
            head = new_server;  // Add as the first node
        } else {
            ServerNode* temp = head;
            while (temp->get_next() != nullptr) {
                temp = temp->get_next();  // Traverse to the end of the list
            }
            temp->set_next(new_server);  // Link the new server at the end
        }
    }

    // Method to remove a server by its socket number
    void remove_server(int socket) {
        ServerNode* current = head;
        ServerNode* prev = nullptr;

        while (current != nullptr) {
            if (current->get_socket() == socket) {
                // Found the server to remove
                if (prev == nullptr) {  // Removing the first node
                    head = current->get_next();
                } else {  // Removing an intermediate or last node
                    prev->set_next(current->get_next());
                }
                delete current;  // Free the memory
                return;
            }
            prev = current;
            current = current->get_next();
        }

        std::cout << "SERVER NOT FOUND" << std::endl;
    }

    // Method to return the first server of the list
    ServerNode* get_first_server() const {
        return head;
    }

    // Method to find a server by its socket number
    ServerNode* find_server(int socket) const {
        ServerNode* current = head;
        while (current != nullptr) {
            if (current->get_socket() == socket) {
                return current;  // Return the matching server node
            }
            current = current->get_next();
        }
        std::cout << "SERVER NOT FOUND" << std::endl;
        return nullptr;
    }

    // Display all servers in the list (for debugging)
    void display_servers() const {
        ServerNode* current = head;
        while (current != nullptr) {
            current->display();
            current = current->get_next();
        }
    }

    // Destructor to free all nodes
    ~ServerList() {
        ServerNode* current = head;
        while (current != nullptr) {
            ServerNode* next = current->get_next();
            delete current;
            current = next;
        }
    }
};

#endif // SERVERLIST_H
