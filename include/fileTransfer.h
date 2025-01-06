#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

#include "packet.h"
#include <fstream>
#include <iostream>
#include <filesystem>

class FileTransfer {
    public:
        static void send_file(const std::string& file_path, int socket) {

            ifstream file(file_path, std::ios::binary | std::ios::ate);

            // File initialization
            if (!file.is_open()) {
                // If can't open file, send a warning packet to client to not wait for reception of file
                cerr << "Error: Cannot open file " << file_path << std::endl;
                Packet dont_receive_file = Packet();
                dont_receive_file.send_packet(socket);
                return;
            }
            std::streamsize file_size = file.tellg(); 
            file.seekg(0, std::ios::beg);

            // (file_size + MAX_PAYLOAD_SIZE - 1) => used to round up the integer division, to account for when there are leftovers bytes
            uint32_t total_packets = (file_size + MAX_PAYLOAD_SIZE - 1) / MAX_PAYLOAD_SIZE;
            uint16_t seq_num = 0;

            // Sending file in fragments of MAX_PAYLOAD_SIZE bytes
            char buffer[MAX_PAYLOAD_SIZE];
            file.read(buffer, MAX_PAYLOAD_SIZE);
            while(file.gcount() > 0) {
                int payload_size = file.gcount(); 
                Packet packet(Packet::DATA_PACKET, seq_num++, total_packets, buffer, payload_size);
                packet.send_packet(socket);
                memset(buffer, 0, MAX_PAYLOAD_SIZE); 
                //printf("Packet length file sent: %d\n", packet.getLength());
                file.read(buffer, MAX_PAYLOAD_SIZE);
            }

            file.close();
            //cout << "File sent successfully!" << std::endl;
        }

        static void receive_file(const std::string& output_path, int socket) {
            // Ensure the directory exists
            std::filesystem::path dir = std::filesystem::path(output_path).parent_path();
            if (!std::filesystem::exists(dir)) {
                if (!std::filesystem::create_directories(dir)) {
                    std::cerr << "Error: Cannot create directory" << dir << std::endl;
                    return;
                }
            }
            // std::cout << "*" << std::endl;

            std::ofstream file(output_path, std::ios::binary);
            
            // std::cout << "**" << std::endl;
            // File initialization
            if (!file.is_open()) {
                cerr << "Error: Cannot create file " << output_path << std::endl;
                return;
            }

            uint32_t total_packets = 0;
            uint16_t seq_num = 0;
            // std::cout << "***" << std::endl;
            // Receiving file in fragments of MAX_PAYLOAD_SIZE bytes
            while (true) { 
                // std::cout << "****" << std::endl;       
                Packet packet = Packet::receive_packet(socket);
                if (packet.type == Packet::ERR) 
                {
                    file.close();
                    remove(output_path.c_str()); 
                    cerr << "Error: Received error packet, file transmission aborted." << std::endl;
                    return;
                }
                total_packets = packet.total_size; 
                // std::cout << total_packets << std::endl;     
                // Write the received payload to the file
                file.write(packet.get_payload(), packet.get_length());

                seq_num++;
                // Check for end-of-transmission signal 
                if (seq_num == total_packets) {
                    //cout << "Received end of transmission packet from client" << std::endl;
                    break;
                }
            }

            file.close();
            //cout << "File received successfully!" << std::endl;
        }
};

#endif
