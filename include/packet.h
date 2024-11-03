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
        // Constant for total packet size in bytes
        static const int PACKET_SIZE = sizeof(type) + sizeof(seqn) + sizeof(total_size) + sizeof(length) + MAX_PAYLOAD_SIZE;

        // Packet Constructor Method
        Packet(uint16_t t, uint16_t s, uint32_t ts, const char* payload) 
            : type(t), seqn(s), total_size(ts), length(min(static_cast<int>(strlen(payload)), MAX_PAYLOAD_SIZE)) {
            strncpy(_payload, payload, length); 
            _payload[length] = '\0'; // Ensure null termination
        }

        // Send packet method
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
                perror("ERROR sending packet"); // Print error message if send failed
            } else if (bytes_sent < PACKET_SIZE) {
                printf("Warning: Only %zd out of %d bytes sent\n", bytes_sent, PACKET_SIZE);
            } else {
                printf("Packet sent successfully: %zd bytes\n", bytes_sent);
            }
            free(stream);
        }

        // Método para receber o pacote
        static Packet receive_packet(int socket) {
            uint16_t type, seqn, length;
            uint32_t total_size;
            char payload[MAX_PAYLOAD_SIZE];
            char* stream = (char*)malloc(PACKET_SIZE);

            // Receiving the stream (packet headers and data) from the specified socket
            long int bytes_received = read(socket, stream, PACKET_SIZE);
            if(bytes_received <= 0){
                printf("Error receiving packet.\n");
                return Packet(0, 0, 0, ""); // Return an empty packet on error
            }else if (bytes_received < PACKET_SIZE) {
                printf("Warning: Only %zd out of %d bytes received\n", bytes_received, PACKET_SIZE);
            } else {
                printf("Packet received successfully: %zd bytes\n", bytes_received);
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
            memcpy(payload, stream+offset, MAX_PAYLOAD_SIZE);

            Packet pkt(type, seqn, total_size, payload);
            free(stream);

            return pkt;
        }


        // Getters & Setters
        char* getPayload(){
            return _payload;
        }

    // Declare FileTransfer as a friend class
    friend class FileTransfer; 
};

#endif
