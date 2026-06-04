#include <stdio.h>
#include <string.h>

#include "command.h"
#include "commands/cd.h"
#include "commands/date.h"
#include "commands/echo.h"

static const struct Command commands[] = {
    {"echo", command_echo},
    {"date", command_date},
    {"cd", command_cd}
};

const struct Command *commands_get_all(void)
{
    return commands;
}

int commands_count(void)
{
    return sizeof(commands) / sizeof(commands[0]);
}

int commands_dispatch(int argc, char **argv)
{
    int i;

    if (argc == 0) {
        return 0;
    }

    for (i = 0; i < commands_count(); ++i) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            return commands[i].handler(argc, argv);
        }
    }

    printf("%s: command not found\n", argv[0]);
    return 1;
}
