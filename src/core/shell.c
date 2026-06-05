#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "banner.h"
#include "command.h"
#include "history.h"
#include "output.h"
#include "shell.h"
#include "prompt.h"
#include "session.h"

#define QUOTED_ARG_MARKER '\001'

static int is_space(char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

static int parse_line(char *line, char **argv, int max_args)
{
    int argc;
    char *read;
    char *write;

    argc = 0;
    read = line;
    write = line;

    while (*read != '\0' && argc < max_args) {
        while (is_space(*read)) {
            read++;
        }

        if (*read == '\0') {
            break;
        }

        if (*read == '"') {
            argv[argc++] = write;
            *write++ = QUOTED_ARG_MARKER;
            read++;
            while (*read != '\0' && *read != '"') {
                *write++ = *read++;
            }
            if (*read == '"') {
                read++;
            }
        } else {
            argv[argc++] = write;
            while (*read != '\0' && !is_space(*read)) {
                *write++ = *read++;
            }
        }

        if (*read != '\0') {
            read++;
        }
        *write++ = '\0';
    }

    return argc;
}

static int split_next_command(char **cursor, char *command, int command_size)
{
    char *read;
    int write;
    int in_quote;

    read = *cursor;
    write = 0;
    in_quote = 0;

    while (*read != '\0' && is_space(*read)) {
        read++;
    }

    if (*read == '\0') {
        *cursor = read;
        return 0;
    }

    while (*read != '\0') {
        if (*read == '"') {
            in_quote = !in_quote;
        }

        if (*read == ';' && !in_quote) {
            read++;
            break;
        }

        if (write < command_size - 1) {
            command[write++] = *read;
        }
        read++;
    }

    command[write] = '\0';
    *cursor = read;
    return 1;
}

static int arg_is_quoted(char *arg)
{
    return arg != 0 && arg[0] == QUOTED_ARG_MARKER;
}

static void unquote_args(char **argv, int argc)
{
    int i;

    for (i = 0; i < argc; ++i) {
        if (arg_is_quoted(argv[i])) {
            argv[i]++;
        }
    }
}

static void trim_token(char *text)
{
    char *start;
    char *end;

    start = text;
    while (is_space(*start)) {
        start++;
    }

    if (start != text) {
        memmove(text, start, strlen(start) + 1);
    }

    end = text + strlen(text);
    while (end > text && is_space(*(end - 1))) {
        end--;
    }
    *end = '\0';
}

static int extract_redirection(char *command, char *target, int target_size)
{
    char *read;
    char *redirect;
    int in_quote;

    read = command;
    redirect = 0;
    in_quote = 0;

    while (*read != '\0') {
        if (*read == '"') {
            in_quote = !in_quote;
        } else if (*read == '>' && !in_quote) {
            redirect = read;
            break;
        }
        read++;
    }

    if (redirect == 0) {
        target[0] = '\0';
        return 0;
    }

    *redirect = '\0';
    redirect++;
    trim_token(command);
    trim_token(redirect);

    if (redirect[0] == '"') {
        redirect++;
        read = strchr(redirect, '"');
        if (read != 0) {
            *read = '\0';
        }
    }

    if (redirect[0] == '\0') {
        puts("redirect: missing output file");
        target[0] = '\0';
        return 1;
    }

    strncpy(target, redirect, target_size - 1);
    target[target_size - 1] = '\0';
    return 0;
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
        while (split_next_command(&cursor, command, sizeof(command))) {
            if (extract_redirection(command, redirect_target, sizeof(redirect_target)) != 0) {
                continue;
            }

            argc = parse_line(command, argv, SHELL_MAX_ARGS);
            if (argc == 0) {
                continue;
            }

            if (strcmp(argv[0], "exit") == 0) {
                should_exit = 1;
                break;
            }

            if (strcmp(argv[0], "echo") != 0) {
                unquote_args(argv, argc);
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
