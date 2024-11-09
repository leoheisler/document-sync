// CommandStatus.h

#ifndef COMMAND_STATUS_H
#define COMMAND_STATUS_H

enum class CommandStatus {
    VALID,
    INVALID_COMMAND,
    TOO_MANY_ARGS,
    TOO_FEW_ARGS,
    NO_COMMAND
};

enum Command {
    GET_SYNC_DIR = 0x0001,
    LIST_SERVER = 0x0002,
    LIST_CLIENT = 0x0003,
    EXIT = 0x0004,
    DOWNLOAD = 0x0005,
    UPLOAD = 0x0006,
    DELETE = 0x0007
};

#endif // COMMAND_STATUS_H
