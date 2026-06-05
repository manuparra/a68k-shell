#include <stdio.h>
#include <string.h>

#include "shell_parser.h"

static int is_space(char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

int shell_parse_line(char *line, char **argv, int max_args)
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

        argv[argc++] = write;

        if (*read == '"') {
            *write++ = SHELL_QUOTED_ARG_MARKER;
            read++;
            while (*read != '\0' && *read != '"') {
                *write++ = *read++;
            }
            if (*read == '"') {
                read++;
            }
        } else {
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

int shell_split_next_command(char **cursor, char *command, int command_size)
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

int shell_extract_redirection(char *command, char *target, int target_size)
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
        trim_token(command);
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

static int arg_is_quoted(char *arg)
{
    return arg != 0 && arg[0] == SHELL_QUOTED_ARG_MARKER;
}

void shell_unquote_args(char **argv, int argc)
{
    int i;

    for (i = 0; i < argc; ++i) {
        if (arg_is_quoted(argv[i])) {
            argv[i]++;
        }
    }
}
