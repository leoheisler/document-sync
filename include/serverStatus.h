#ifndef SERVER_STATUS_H
#define SERVER_STATUS_H

#include <string>

// Definindo o enum class ServerStatus
enum class serverStatus {
    OK,
    FAILED_TO_CREATE_SOCKET,
    FAILED_TO_BIND_SOCKET,
    FAILED_TO_ACCEPT_CMD_SOCKET,
    FAILED_TO_ACCEPT_UPLOAD_SOCKET,
    FAILED_TO_ACCEPT_FETCH_SOCKET
};

// Função para converter ServerStatus para string
std::string inline to_string(serverStatus status) {
    switch (status) {
        case serverStatus::OK:
            return "OK";
        case serverStatus::FAILED_TO_CREATE_SOCKET:
            return "FAILED TO CREATE SOCKET\n";
        case serverStatus::FAILED_TO_BIND_SOCKET:
            return "FAILED TO BIND SOCKET\n";
        case serverStatus::FAILED_TO_ACCEPT_CMD_SOCKET:
            return "FAILED TO ACCEPT CMD SOCKET\n";
        case serverStatus::FAILED_TO_ACCEPT_UPLOAD_SOCKET:
            return "FAILED TO ACCEPT UPLOAD SOCKET\n";
        case serverStatus::FAILED_TO_ACCEPT_FETCH_SOCKET:
            return "FAILED TO ACCEPT FETCH SOCKET\n";
        default:
            return "Unknown Status";  // Valor default para casos inesperados
    }
}

#endif // SERVER_STATUS_H
