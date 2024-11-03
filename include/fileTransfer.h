#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

#include "packet.h"
#include <fstream>
#include <iostream>

class FileTransfer {
public:
    static void send_file(const std::string& file_path, int socket) {
        std::ifstream file(file_path, std::ios::binary | std::ios::ate);
        
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file " << file_path << std::endl;
            return;
        }

        std::streamsize file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        // (file_size + MAX_PAYLOAD_SIZE - 1) => used to round up the integer division, to account for when there are leftovers bytes
        uint32_t total_packets = (file_size + MAX_PAYLOAD_SIZE - 1) / MAX_PAYLOAD_SIZE;
        uint16_t seq_num = 0;

        char buffer[MAX_PAYLOAD_SIZE];
        while (file.read(buffer, MAX_PAYLOAD_SIZE) || file.gcount() > 0) {
            uint16_t payload_size = static_cast<uint16_t>(file.gcount());
            Packet packet(1, seq_num++, total_packets, buffer);
            packet.send_packet(socket);
            std::memset(buffer, 0, MAX_PAYLOAD_SIZE);
        }

        file.close();
        std::cout << "File sent successfully!" << std::endl;
    }

    static void receive_file(const std::string& output_path, int socket) {
        std::ofstream file(output_path, std::ios::binary);
        
        if (!file.is_open()) {
            std::cerr << "Error: Cannot create file " << output_path << std::endl;
            return;
        }

        uint32_t total_packets = 0;
        uint16_t seq_num = 0;

        while (true) {
            Packet packet = Packet::receive_packet(socket);

            if (packet.getPayload()[0] == '\0') { // Assuming a packet with null payload indicates end of transmission
                break; // Exit loop if no more packets are to be received
            }

            total_packets = packet.total_size; // Capture total number of packets

            // Write the received payload to the file
            file.write(packet.getPayload(), packet.length);

            // Optional: You can add checks for seq_num here if needed
            seq_num++;
        }

        file.close();
        std::cout << "File received successfully!" << std::endl;
    }
};

#endif
