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

        std::streamsize file_size = file.tellg(); // Get the size of the file
        file.seekg(0, std::ios::beg); // Move the cursor back to the beginning of the file

        // (file_size + MAX_PAYLOAD_SIZE - 1) => used to round up the integer division, to account for when there are leftovers bytes
        uint32_t total_packets = (file_size + MAX_PAYLOAD_SIZE - 1) / MAX_PAYLOAD_SIZE;
        uint16_t seq_num = 0;

        char buffer[MAX_PAYLOAD_SIZE]; // Buffer to hold file data
        while (file.read(buffer, MAX_PAYLOAD_SIZE) || file.gcount() > 0) {
            uint16_t payload_size = static_cast<uint16_t>(file.gcount()); // Get the size of the data read
            Packet packet(1, seq_num++, total_packets, buffer, payload_size); // Create a packet with the data
            packet.send_packet(socket); // Send the packet over the socket
            std::memset(buffer, 0, MAX_PAYLOAD_SIZE); // Clear the buffer for the next read
             // Print packet length to terminal
            printf("Packet length file sent: %d\n", packet.getLength());
        }

        // Send an end-of-transmission packet
        Packet end_packet(0, 0, 0, "\0",0); // Create a packet indicating the end of the transmission
        end_packet.send_packet(socket); // Send the end packet to the receiver


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
            Packet packet = Packet::receive_packet(socket); // Receive a packet from the socket

            // Print packet length to terminal
            printf("Packet length file received1: %d\n", packet.getLength());


            // Check for end-of-transmission signal (a packet with null payload)
            if (packet.getPayload()[0] == '\0' && packet.length == 0) {
                std::cout << "Received end of packet from client" << std::endl;
                break; // Exit the loop if the end packet is received
            }

            total_packets = packet.total_size; // Capture total number of packets

            // Write the received payload to the file
            file.write(packet.getPayload(), packet.getLength());

            // Print packet length to terminal
            printf("Packet length file received2: %d\n", packet.getLength());

            // Optional: You can add checks for seq_num here if needed
            seq_num++;
        }

        file.close();
        std::cout << "File received successfully!" << std::endl;
    }
};

#endif
