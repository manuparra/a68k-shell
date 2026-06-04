#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "banner.h"
#include "command.h"
#include "history.h"
#include "shell.h"
#include "prompt.h"
#include "session.h"

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

static int expand_history(char *line, int line_size)
{
    int number;
    int i;
    const char *entry;

    if (line[0] != '!') {
        return 0;
    }

    i = 1;
    while (line[i] >= '0' && line[i] <= '9') {
        i++;
    }

    if (i == 1 || (line[i] != '\0' && line[i] != '\n' && line[i] != '\r')) {
        printf("%s: invalid history reference\n", line);
        return 1;
    }

    number = atoi(line + 1);

    entry = history_get_display_number(number);
    if (entry == 0) {
        printf("!%d: event not found\n", number);
        return 1;
    }

    strncpy(line, entry, line_size - 1);
    line[line_size - 1] = '\0';
    puts(line);

    return 0;
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
            session_cleanup();
            return;
        }

        if (expand_history(line, sizeof(line)) != 0) {
            continue;
        }

        history_add(line);

        argc = parse_line(line, argv, SHELL_MAX_ARGS);
        if (argc == 0) {
            continue;
        }

        if (strcmp(argv[0], "exit") == 0) {
            session_cleanup();
            return;
        }

        commands_dispatch(argc, argv);
    }
}
