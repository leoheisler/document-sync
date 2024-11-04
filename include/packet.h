#ifndef PACKET
#define PACKET
#include <math.h>
#include <string>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>

#define MAX_PAYLOAD_SIZE 1024

class FileTransfer; // Forward declaration

using namespace std;

class Packet {
    private:
        uint16_t type;                     // Packet Type (p.ex. DATA | CMD)
        uint16_t seqn;                     // Sequential Number
        uint32_t total_size;               // Total number of fragments
        uint16_t length;                   // Payload length
        char _payload[MAX_PAYLOAD_SIZE];   // Packet data

    public:
        // Enum to define packet types
        enum PacketType {
            ERR = 0x0000,
            DATA_PACKET = 0x0001,
            CMD_PACKET = 0x0002,
        };

        // Constant for total packet size in bytes
        static const int PACKET_SIZE = sizeof(type) + sizeof(seqn) + sizeof(total_size) + sizeof(length) + MAX_PAYLOAD_SIZE;

        // Packet Constructors Methods
        Packet(uint16_t t, uint16_t s, uint32_t ts, const char* payload,  uint16_t payload_length) 
            : type(t), seqn(s), total_size(ts), length(min(static_cast<int>(payload_length), MAX_PAYLOAD_SIZE)) {
            memcpy(_payload, payload, length); 
        }
        Packet() : type(ERR), seqn(0), total_size(0), length(0) {};

        // Packet Transmission Methods
        void send_packet(int socket) {
            char* stream = (char*)malloc(PACKET_SIZE);
            size_t offset = 0;

            // Breaking Packet content into stream
            memcpy(stream + offset, &type, sizeof(type));
            offset += sizeof(type);
            memcpy(stream + offset, &seqn, sizeof(seqn));
            offset += sizeof(seqn);
            memcpy(stream + offset, &total_size, sizeof(total_size));
            offset += sizeof(total_size);
            memcpy(stream + offset, &length, sizeof(length));
            offset += sizeof(length);
            memcpy(stream + offset, _payload, length);

            // Sending the stream (packet headers and data) to the specified socket
            long int bytes_sent = write(socket, stream, PACKET_SIZE);

            // Check if the send was successful
            if (bytes_sent < 0) {
                perror("ERROR sending packet"); 
            } else if (bytes_sent < PACKET_SIZE) {
                printf("Warning: Only %zd out of %d bytes sent\n", bytes_sent, PACKET_SIZE);
            } else {
                printf("Packet sent successfully: %zd bytes\n", bytes_sent);
            }

            // Print packet length to terminal
            printf("Packet length sent: %d\n", length);

            free(stream);
            clear(); 
            return;
        }

        static Packet receive_packet(int socket) {
            uint16_t type, seqn, length;
            uint32_t total_size;
            char payload[MAX_PAYLOAD_SIZE];
            
            // Allocate memory for receiving the full packet
            char* stream = (char*)malloc(PACKET_SIZE);
            if (!stream) {
                printf("Error: Failed to allocate memory for receiving packet.\n");
                return Packet();
            }

            // Receiving the stream (packet headers and data) from the socket
            long int bytes_received = read(socket, stream, PACKET_SIZE);
            if (bytes_received <= 0) {
                printf("Error receiving packet.\n");
                free(stream); 
                return Packet(); 
            } else if (bytes_received < PACKET_SIZE) {
                printf("Warning: Only %zd out of %d bytes received\n", bytes_received, PACKET_SIZE);
            } else {
                printf("Packet received successfully: %zd bytes\n", bytes_received);
            }

            // Check if the received data is enough to fill headers
            if (bytes_received < sizeof(type) + sizeof(seqn) + sizeof(total_size) + sizeof(length)) {
                printf("Error: Incomplete header received.\n");
                free(stream);
                return Packet(); 
            }

            // Remounting Packet from stream data
            size_t offset = 0;
            memcpy(&type, stream + offset, sizeof(type));
            offset += sizeof(type);
            memcpy(&seqn, stream + offset, sizeof(seqn));
            offset += sizeof(seqn);
            memcpy(&total_size, stream + offset, sizeof(total_size));
            offset += sizeof(total_size);
            memcpy(&length, stream + offset, sizeof(length));
            offset += sizeof(length);

            // Ensure length is within bounds to prevent buffer overflow
            if (length > MAX_PAYLOAD_SIZE) {
                printf("Error: Payload length %d exceeds maximum allowed size %d.\n", length, MAX_PAYLOAD_SIZE);
                free(stream);
                return Packet(); // Return empty packet if length is invalid
            }
            memcpy(payload, stream + offset, length);            

            Packet packet(type, seqn, total_size, payload, length);

            //printf("Packet length when received: %d\n", packet.getLength());
            free(stream); // Free allocated memory after use

            return packet;
        }


        // Getters & Setters
        char* getPayload(){
            return _payload;
        }

        uint16_t getLength(){
            return length;
        }

        void clear() {
            std::memset(_payload, 0, MAX_PAYLOAD_SIZE); // Clear payload
        }

    // Declare FileTransfer as a friend class
    friend class FileTransfer; 
};

#endif
