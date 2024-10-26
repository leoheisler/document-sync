// CommandStatus.h

#ifndef COMMAND_STATUS_H
#define COMMAND_STATUS_H

enum class CommandStatus {
    VALID,
    INVALID_COMMAND,
    TOO_MANY_ARGS,
    TOO_FEW_ARGS
};

#endif // COMMAND_STATUS_H
