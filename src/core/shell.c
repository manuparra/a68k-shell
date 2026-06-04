#include <stdio.h>
#include <string.h>

#include "banner.h"
#include "command.h"
#include "shell.h"
#include "prompt.h"

static int parse_line(char *line, char **argv, int max_args)
{
    int argc;
    char *token;

    argc = 0;
    token = strtok(line, " \t\r\n");

    while (token != 0 && argc < max_args) {
        argv[argc++] = token;
        token = strtok(0, " \t\r\n");
    }

    return argc;
}

void shell_run(void)
{
    char line[SHELL_MAX_LINE + 1];
    char *argv[SHELL_MAX_ARGS];
    int argc;

    banner_print();

    for (;;) {
        prompt_print();

        if (fgets(line, sizeof(line), stdin) == 0) {
            putchar('\n');
            return;
        }

        argc = parse_line(line, argv, SHELL_MAX_ARGS);
        if (argc == 0) {
            continue;
        }

        if (strcmp(argv[0], "exit") == 0) {
            return;
        }

        commands_dispatch(argc, argv);
    }
}
