// CommandStatus.h

#ifndef COMMAND_STATUS_H
#define COMMAND_STATUS_H

enum class CommandStatus {
    VALID,
    TOO_MANY_ARGS,
    TOO_FEW_ARGS,
};

enum Command {
    UPLOAD = 0x0001,
    DOWNLOAD = 0x0002,
    DELETE = 0x0003,
    LIST_SERVER = 0x0004,
    LIST_CLIENT = 0x0005,
    EXIT = 0x0006,
    GET_SYNC_DIR = 0x0007,
    NO_COMMAND = 0x0008
};

#endif // COMMAND_STATUS_H
