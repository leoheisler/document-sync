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

        // Method to add or update a server in the list
        void add_server(int socket, const std::string& hostname) {
            ServerNode* temp = head;

            // Search for a server with the same hostname
            while (temp != nullptr) {
                if (temp->get_hostname() == hostname) {
                    // If the hostname matches, overwrite the socket
                    temp->set_socket(socket);
                    return;  // Exit the method after updating
                }
                temp = temp->get_next();  // Move to the next node
            }

            // If no matching hostname is found, add a new server node
            ServerNode* new_server = new ServerNode(socket, hostname);
            if (head == nullptr) {
                head = new_server;  // Add as the first node
            } else {
                temp = head;
                while (temp->get_next() != nullptr) {
                    temp = temp->get_next();  // Traverse to the end of the list
                }
                temp->set_next(new_server);  // Link the new server at the end
            }
        }

        // Method to remove a server by its hostname
        void remove_server(string hostname) {
            ServerNode* current = head;
            ServerNode* prev = nullptr;

            while (current != nullptr) {
                if (current->get_hostname() == hostname) {
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
        ServerNode* find_current_server(const std::string& hostname) const {
            if (head == nullptr) {
                return nullptr; // Empty list
            }

            ServerNode* current = head;

            while (current != nullptr) {
                if (current->get_hostname() == hostname) {
                    // If it is the last one, return the first node of the list (ring connection)
                    return (current);
                }
                current = current->get_next();
            }

            // If none has the host name, return null
            std::cout << "HOSTNAME NOT FOUND" << std::endl;
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

        // Method to get the position (ID) of a server in the list based on its hostname
        int get_server_id(const std::string& hostname) const {
            ServerNode* current = head;
            int position = 0;

            while (current != nullptr) {
                if (current->get_hostname() == hostname) {
                    return position;  // Return the position of the server
                }
                current = current->get_next();
                position++;
            }

            // Server not found
            std::cout << "HOSTNAME NOT FOUND" << std::endl;
            return -1;
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
