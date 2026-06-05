#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "banner.h"
#include "command.h"
#include "history.h"
#include "output.h"
#include "shell.h"
#include "shell_parser.h"
#include "prompt.h"
#include "session.h"

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
    char command[SHELL_MAX_LINE + 1];
    char redirect_target[SHELL_MAX_LINE + 1];
    char *cursor;
    char *argv[SHELL_MAX_ARGS];
    FILE *redirect_file;
    int argc;
    int should_exit;

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

        cursor = line;
        should_exit = 0;
        while (shell_split_next_command(&cursor, command, sizeof(command))) {
            if (shell_extract_redirection(command, redirect_target, sizeof(redirect_target)) != 0) {
                continue;
            }

            argc = shell_parse_line(command, argv, SHELL_MAX_ARGS);
            if (argc == 0) {
                continue;
            }

            if (strcmp(argv[0], "exit") == 0) {
                should_exit = 1;
                break;
            }

            if (strcmp(argv[0], "echo") != 0) {
                shell_unquote_args(argv, argc);
            }

            redirect_file = 0;
            if (redirect_target[0] != '\0') {
                redirect_file = fopen(redirect_target, "w");
                if (redirect_file == 0) {
                    printf("redirect: cannot write %s\n", redirect_target);
                    continue;
                }
                output_set(redirect_file);
            }

            commands_dispatch(argc, argv);

            if (redirect_file != 0) {
                output_set(stdout);
                fclose(redirect_file);
            }
        }

        if (should_exit) {
            session_cleanup();
            return;
        }
    }
}
