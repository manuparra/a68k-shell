#include <stdio.h>

#include "commands/echo.h"

int command_echo(int argc, char **argv)
{
    int i;

    for (i = 1; i < argc; ++i) {
        if (i > 1) {
            putchar(' ');
        }
        fputs(argv[i], stdout);
    }

    putchar('\n');
    return 0;
}

