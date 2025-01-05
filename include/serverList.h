#ifndef SERVERLIST_H
#define SERVERLIST_H

#include <iostream>

class ServerNode {
    private:
        int socket;             // Socket number of the server, will be used as the id for the election as well
        std::string hostname;   // Hostname of the server
        ServerNode* next;       // Pointer to the next node
        bool is_leader;         // bool that says if it is the Leader

    public:
        // Constructor
        ServerNode(int sock, const std::string& host = "")
            : socket(sock), hostname(host), next(nullptr), is_leader(false)  {}

        // Getters and Setters
        int get_socket() const { return socket; }
        void set_socket(int sock) { socket = sock; }
        bool get_is_leader() const { return is_leader; }
        void set_is_leader(bool leader_status) { is_leader = leader_status; }

        const std::string& get_hostname() const { return hostname; }
        void set_hostname(const std::string& host) { hostname = host; }

        ServerNode* get_next() const { return next; }
        void set_next(ServerNode* next_node) { next = next_node; }

        // Display server info (for debugging)
        void display() const {
            std::cout << "Server Socket: " << socket
                    << ", Hostname: " << hostname << std::endl;
        }
};

class ServerList {
    private:
        ServerNode* head;  // Head of the linked list

    public:
        // Constructor
        ServerList() : head(nullptr) {}

        // Method to add a server to the list
        void add_server(int socket, const std::string& hostname) {
            ServerNode* new_server = new ServerNode(socket, hostname);
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

        ServerNode* find_next_server(const std::string& hostname) const {
            if (head == nullptr) {
                return nullptr; // Empty list
            }

            ServerNode* current = head;

            while (current != nullptr) {
                if (current->get_hostname() == hostname) {
                    // If it is the last one, return the first node of the list (ring connection)
                    return (current->get_next() != nullptr) ? current->get_next() : head;
                }
                current = current->get_next();
            }

            // If none has the host name, return null
            std::cout << "HOSTNAME NOT FOUND" << std::endl;
            return nullptr;
        }

        // Display all servers in the list (for debugging)
        void display_servers() const {
            ServerNode* current = head;
            cout << endl;
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
