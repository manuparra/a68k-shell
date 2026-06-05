#include <stdio.h>

#include "commands/echo.h"

#define QUOTED_ARG_MARKER '\001'

int command_echo(int argc, char **argv)
{
    char *value;

    if (argc != 2 || argv[1][0] != QUOTED_ARG_MARKER) {
        puts("usage: echo \"<string>\"");
        return 1;
    }

    value = argv[1] + 1;
    fputs(value, stdout);
    putchar('\n');
    return 0;
}
