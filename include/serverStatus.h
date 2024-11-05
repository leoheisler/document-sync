#ifndef SERVER_STATUS_H
#define SERVER_STATUS_H

#include <string>

// Definindo o enum class ServerStatus
enum class serverStatus {
    OK,
    FAILED_TO_CREATE_SOCKET,
    FAILED_TO_BIND_SOCKET,
    FAILED_TO_ACCEPT_SOCKET
};

// Função para converter ServerStatus para string
std::string toString(serverStatus status) {
    switch (status) {
        case serverStatus::OK:
            return "OK";
        case serverStatus::FAILED_TO_CREATE_SOCKET:
            return "FAILED TO CREATE SOCKET";
        case serverStatus::FAILED_TO_BIND_SOCKET:
            return "FAILED TO BIND SOCKET";
        case serverStatus::FAILED_TO_ACCEPT_SOCKET:
            return "FAILED TO ACCEPT SOCKET";
        default:
            return "Unknown Status";  // Valor default para casos inesperados
    }
}

#endif // SERVER_STATUS_H
