#include <stdio.h>
#include <string.h>

#include "command.h"
#include "commands/cat.h"
#include "commands/cd.h"
#include "commands/date.h"
#include "commands/df.h"
#include "commands/echo.h"
#include "commands/head.h"
#include "commands/history.h"
#include "commands/ls.h"
#include "commands/mkdir.h"
#include "commands/ps.h"
#include "commands/pwd.h"
#include "commands/rm.h"
#include "commands/tail.h"

static const struct Command commands[] = {
    {"echo", command_echo},
    {"date", command_date},
    {"df", command_df},
    {"cd", command_cd},
    {"ls", command_ls},
    {"history", command_history},
    {"head", command_head},
    {"tail", command_tail},
    {"pwd", command_pwd},
    {"mkdir", command_mkdir},
    {"rm", command_rm},
    {"cat", command_cat},
    {"ps", command_ps}
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
