#ifndef PACKET
#define PACKET

#include <array>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h>

constexpr int MAX_PAYLOAD_SIZE = 1024; 
constexpr const char* INVALID_SOCKET_ERROR_MSG = "Invalid socket number";
constexpr const char* FAILED_WRITE_ERROR_MSG = "Failed to write data into socket";
constexpr const char* FAILED_READ_ERROR_MSG = "Failed to read data from socket";

class FileTransfer;
class Packet {
    private:
        uint8_t type;                                          // Packet Type (p.ex. DATA | CMD)
        std::array<unsigned char, MAX_PAYLOAD_SIZE> payload;   // Packet data

    public:
        enum PacketType : uint8_t {
            ERR_PACKET,          // Error or invalid packet
            DATA_PACKET,         // Data transmission packet
            CMD_PACKET,          // Command packet
            COMM_PACKET,         // Communication initialization packet
            CLIENTINFO_PACKET,   // Packet with client information
            DELETEDEVICE_PACKET, // Request to delete a client device
            SERVERINFO_PACKET,   // Packet with server information
            DELETESERVER_PACKET, // Request to delete a server
            HEARTBEAT_PACKET,    // Keep-alive or heartbeat packet
            ELECTION_PACKET,     // Election process packet
            EOT_PACKET,          // End-of-transmission packet
            SUCCESS,             // Success acknowledgment
            ELECTED_PACKET,      // Notification of election result
            UNKNOWN              // Default value for unknown packet types
        };
        static const int PACKET_SIZE = sizeof(type) + MAX_PAYLOAD_SIZE;

        /**
         * @brief Constructs a packet with a specific type and payload.
         *
         * This constructor initializes a `Packet` object by setting the packet's type and 
         * the associated payload data.
         *
         * @param t The type of the packet, represented as a `uint8_t`. 
         *          It determines the purpose or category of the packet (e.g., data, command).
         * @param payload The payload data to be associated with the packet.
         *                It can be up to `MAX_PAYLOAD_SIZE` in size.
        */
        Packet(uint8_t t, const std::array<unsigned char, MAX_PAYLOAD_SIZE>& payload)
            : type(t), payload(payload) {}
        Packet(int sock_add) : type(PacketType::COMM_PACKET), payload{} {}

        /**
         * @brief Constructs an error packet with default values.
         *
         * This constructor initializes a `Packet` object with the type set to `ERR_PACKET`,
         * which typically indicates an error or invalid packet. The payload is set to an empty state.
        */
        Packet() : type(PacketType::ERR_PACKET), payload{} {}

        /**
         * @brief Sends a data packet to the specified socket using a TCP connection.
         *
         * This function serializes the packet data (header and payload) into a stream buffer
         * and sends it over the specified socket using a blocking `write` call. The function ensures
         * that the entire packet is sent by tracking the number of bytes successfully written.
         *
         * @param socket The file descriptor of the socket used for communication.
         *               Must be a valid and open socket for writing.
         * @throws std::invalid_argument If the socket is not valid (<= 0).
         * @throws std::runtime_error If sending the data fails during the write process.
        */
        void send_packet(int socket) {
            if (socket <= 0) {
                throw std::invalid_argument(INVALID_SOCKET_ERROR_MSG);
            }
            
            char *stream = new char[PACKET_SIZE];
            int offset = 0;
            
            // Serializing packet headers + data into stream
            memcpy(stream + offset, &type, sizeof(type));
            offset += sizeof(type);
            memcpy(stream + offset, &payload, get_payload_length());

            // Sending the stream (packet headers + data) to the specified socket
            int bytes_sent, total_bytes_sent = 0;
            do {
                bytes_sent = write(
                    socket, stream + total_bytes_sent, PACKET_SIZE - total_bytes_sent
                );
                if (bytes_sent < 0) {
                    delete[] (stream); 
                    throw std::runtime_error(FAILED_READ_ERROR_MSG);
                } 
                total_bytes_sent += bytes_sent;
            }while(total_bytes_sent < PACKET_SIZE);

            delete[] (stream);
            return;
        }

        /**
         * @brief Receives a data packet from the specified socket using a TCP connection.
         *
         * This function blocks until the entire packet is received from the socket. It waits for data to
         * become available on the socket for a specified time period. Once data is available, it reads the
         * stream buffer and deserializes the packet's header and payload.
         *
         * @param socket The file descriptor of the socket used for communication.
         *               Must be a valid and open socket for reading.
         * @param timeout_seconds Time limit the functions awaits for data to arrive into socket.
         * @throws std::invalid_argument If the socket is not valid (<= 0).
         * @throws std::runtime_error If receiving the data fails during the read process.
         * @return A `Packet` object containing the deserialized header and payload. If the timeout occurs
         *         without receiving data, an error `Packet` is returned.
        */        
        static Packet receive_packet(int socket, int timeout_seconds = 300) {
            if (socket <= 0) {
                throw std::invalid_argument(INVALID_SOCKET_ERROR_MSG);
            }

            fd_set read_fds;
            struct timeval timeout;
            timeout.tv_sec = timeout_seconds;
            timeout.tv_usec = 0;

            FD_ZERO(&read_fds);
            FD_SET(socket, &read_fds);

            // Wait for data to become available on the socket (return empty packet if timeout occurs)
            int select_result = select(socket + 1, &read_fds, nullptr, nullptr, &timeout);
            if (select_result < 0) {
                throw std::runtime_error(FAILED_READ_ERROR_MSG);
            }else if(select_result == 0){
                return Packet();
            }

            // Receiving data into stream from the specified socket (if data is available)
            char* stream = new char[PACKET_SIZE];
            int bytes_received, total_bytes_received = 0;
            do {
                bytes_received = read(socket, stream + total_bytes_received, PACKET_SIZE - total_bytes_received);
                if (bytes_received < 0) {
                    delete[] (stream);
                    throw std::runtime_error(FAILED_READ_ERROR_MSG);
                }
                total_bytes_received += bytes_received;
            } while (total_bytes_received < PACKET_SIZE);

            // Deserialize stream into packet headers + data
            int offset = 0;
            uint8_t type;
            std::array<unsigned char, MAX_PAYLOAD_SIZE> payload;

            memcpy(&type, stream + offset, sizeof(type));
            offset += sizeof(type);
            memcpy(&payload, stream + offset, MAX_PAYLOAD_SIZE);

            delete[] (stream);
            return Packet(type, payload);
        }

        // Getters & Setters
        uint8_t get_type() const{
            return type;
        }
        std::array<unsigned char, MAX_PAYLOAD_SIZE> get_payload() const{
            return this->payload;
        }
        int get_payload_length() const{
            return this->payload.size();
        }

    // Declare FileTransfer as a friend class
    friend class FileTransfer; 
};

#endif
