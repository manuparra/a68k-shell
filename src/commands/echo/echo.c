#include <stdio.h>

#include "commands/echo.h"
#include "output.h"

#define QUOTED_ARG_MARKER '\001'

int command_echo(int argc, char **argv)
{
    char *value;

    if (argc != 2 || argv[1][0] != QUOTED_ARG_MARKER) {
        output_puts("usage: echo \"<string>\"");
        return 1;
    }

    value = argv[1] + 1;
    output_fputs(value);
    output_putchar('\n');
    return 0;
}
