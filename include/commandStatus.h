// CommandStatus.h

#ifndef COMMAND_STATUS_H
#define COMMAND_STATUS_H

enum class CommandStatus {
    VALID,
    INVALID_COMMAND,
    TOO_MANY_ARGS,
    TOO_FEW_ARGS
};

enum Command {
    GET_SYNC_DIR = 0x0001,
};

#endif // COMMAND_STATUS_H
