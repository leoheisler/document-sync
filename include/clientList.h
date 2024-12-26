#ifndef CLIENTLIST_H
#define CLIENTLIST_H

#include <iostream>
#include <string>
#include <tuple>

using namespace std;

class ClientNode {
    private:
        string username;                        // Username of the client user
        string device1_hostname;               // Hostname for device 1
        string device2_hostname;               // Hostname for device 2
        tuple<int, int, int> device1_sockets;  // Sockets for device 1 (cmd, upload, download)
        tuple<int, int, int> device2_sockets;  // Sockets for device 2 (cmd, upload, download)
        ClientNode* next;                      // Pointer to the next node

    public:
        // Constructor with default values for sockets and hostnames
        ClientNode(const std::string& uname,
                   const std::string& dev1_hostname = "",
                   const std::tuple<int, int, int>& dev1_sockets = {0, 0, 0},
                   const std::string& dev2_hostname = "",
                   const std::tuple<int, int, int>& dev2_sockets = {0, 0, 0})
            : username(uname),
              device1_hostname(dev1_hostname),
              device1_sockets(dev1_sockets),
              device2_hostname(dev2_hostname),
              device2_sockets(dev2_sockets),
              next(nullptr) {}

        // Getters for username and socket information
        const string& get_username() const { return username; }

         // Getters and Setters for device 1 hostname
        const string& get_device1_hostname() const { return device1_hostname; }
        void set_device1_hostname(const string& hostname) { device1_hostname = hostname; }

        // Getters and Setters for device 2 hostname
        const string& get_device2_hostname() const { return device2_hostname; }
        void set_device2_hostname(const string& hostname) { device2_hostname = hostname; }

        // Getters and Setters for device 1 sockets
        std::tuple<int, int, int> get_device1_sockets() const { return device1_sockets; }
        void set_device1_sockets(int cmd, int upload, int download) { 
            device1_sockets = std::make_tuple(cmd, upload, download); 
        }
        int get_device1_cmd_socket() const { return std::get<0>(device1_sockets); }
        int get_device1_upload_socket() const { return std::get<1>(device1_sockets); }
        int get_device1_download_socket() const { return std::get<2>(device1_sockets); }
        void set_device1_cmd_socket(int socket) { std::get<0>(device1_sockets) = socket; }
        void set_device1_upload_socket(int socket) { std::get<1>(device1_sockets) = socket; }
        void set_device1_download_socket(int socket) { std::get<2>(device1_sockets) = socket; }

        // Getters and Setters for device 2 sockets
        std::tuple<int, int, int> get_device2_sockets() const { return device2_sockets; }
        void set_device2_sockets(int cmd, int upload, int download) { 
            device2_sockets = std::make_tuple(cmd, upload, download); 
        }
        int get_device2_cmd_socket() const { return std::get<0>(device2_sockets); }
        int get_device2_upload_socket() const { return std::get<1>(device2_sockets); }
        int get_device2_download_socket() const { return std::get<2>(device2_sockets); }
        void set_device2_cmd_socket(int socket) { std::get<0>(device2_sockets) = socket; }
        void set_device2_upload_socket(int socket) { std::get<1>(device2_sockets) = socket; }
        void set_device2_download_socket(int socket) { std::get<2>(device2_sockets) = socket; }

        // Getters and Setters for next pointer
        ClientNode* get_next() const { return next; }
        void set_next(ClientNode* next_node) { next = next_node; }

        // Display client info (updated to include hostnames)
        void display() const {
            std::cout << "Client: " << username << std::endl
                    << " Device 1: HOSTNAME=" << device1_hostname
                    << ", CMD=" << std::get<0>(device1_sockets)
                    << ", UPLOAD=" << std::get<1>(device1_sockets)
                    << ", DOWNLOAD=" << std::get<2>(device1_sockets) << std::endl
                    << " Device 2: HOSTNAME=" << device2_hostname
                    << ", CMD=" << std::get<0>(device2_sockets)
                    << ", UPLOAD=" << std::get<1>(device2_sockets)
                    << ", DOWNLOAD=" << std::get<2>(device2_sockets) << std::endl;
        }
};

class ClientList {
    private:
        ClientNode* head;  // Head of the linked list
        const tuple<int, int, int> empty_device{0, 0, 0};

    public:
        ClientList() : head(nullptr) {}

        // Method to add a new device to the list
        bool add_device(const std::string& uname, const std::string& hostname, const std::tuple<int, int, int>& device_sockets) {
            ClientNode* current = head;

            // Search for the client node by username
            while (current != nullptr) {
                if (current->get_username() == uname) {
                    // Check if there is a free slot for the new device
                    if (current->get_device1_sockets() == empty_device) {
                        current->set_device1_sockets(std::get<0>(device_sockets),
                                                    std::get<1>(device_sockets),
                                                    std::get<2>(device_sockets));
                        current->set_device1_hostname(hostname); // Set device 1 hostname
                    } else if (current->get_device2_sockets() == empty_device) {
                        current->set_device2_sockets(std::get<0>(device_sockets),
                                                    std::get<1>(device_sockets),
                                                    std::get<2>(device_sockets));
                        current->set_device2_hostname(hostname); // Set device 2 hostname
                    } else {
                        std::cout << "CLIENT FULL" << std::endl;
                        return true;
                    }
                    return false;
                }
                current = current->get_next();
            }

            // If we reach here, the client was not found, so add a new client
            ClientNode* new_client = new ClientNode(uname, hostname, device_sockets);

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
            return false;
        }


        ClientNode* get_first_client()
        {
            return head;
        }

        // Method to remove a device by username and socket triple
        void remove_device(const std::string& uname, const std::tuple<int, int, int>& device_sockets) {
            ClientNode* current = head;
            ClientNode* prev = nullptr;

            while (current != nullptr) {
                if (current->get_username() == uname) {
                    // Check and remove the corresponding device sockets
                    if (current->get_device1_sockets() == device_sockets) {
                        current->set_device1_sockets(0, 0, 0); // Clear device 1 sockets
                        current->set_device1_hostname("");    // Clear device 1 hostname
                    } else if (current->get_device2_sockets() == device_sockets) {
                        current->set_device2_sockets(0, 0, 0); // Clear device 2 sockets
                        current->set_device2_hostname("");    // Clear device 2 hostname
                    } else {
                        std::cout << "DEVICE NOT FOUND FOR USER" << std::endl;
                        return;
                    }

                    // Remove the client if both devices are disconnected
                    if (current->get_device1_sockets() == empty_device &&
                        current->get_device2_sockets() == empty_device) {
                        if (prev == nullptr) {  // Removing the first node
                            head = current->get_next();
                        } else {  // Removing an intermediate or last node
                            prev->set_next(current->get_next());
                        }
                        delete current;  // Free the memory
                    }
                    return;
                }
                prev = current;
                current = current->get_next();
            }

            std::cout << "CLIENT NOT FOUND" << std::endl;
        }

        // Method to retrieve a client node by username
        ClientNode* get_client(const std::string& uname) const {
            ClientNode* current = head;
            
            while (current != nullptr) {
                if (current->get_username() == uname) {
                    return current;  // Return the pointer to the matching client node
                }
                current = current->get_next();  // Move to the next node
            }
            // If the username is not found
            cout << "CLIENT NOT FOUND";
            return nullptr;
        }

        // Display all clients in the list (for debugging)
        void display_clients() const {
            ClientNode* current = head;
            cout << endl;
            while (current != nullptr) {
                current->display();
                current = current->get_next();
            }
        }
};

#endif // CLIENTLIST_H
