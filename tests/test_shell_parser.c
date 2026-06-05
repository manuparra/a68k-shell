#include <stdio.h>
#include <string.h>

#include "shell.h"
#include "shell_parser.h"

static int failures;

static void expect_int(const char *name, int actual, int expected)
{
    if (actual != expected) {
        printf("%s: expected %d, got %d\n", name, expected, actual);
        failures++;
    }
}

static void expect_str(const char *name, const char *actual, const char *expected)
{
    if (actual == 0) {
        printf("%s: expected '%s', got null\n", name, expected);
        failures++;
        return;
    }

    if (strcmp(actual, expected) != 0) {
        printf("%s: expected '%s', got '%s'\n", name, expected, actual);
        failures++;
    }
}

static void test_mkdir_arg(void)
{
    char line[] = "mkdir dir";
    char *argv[SHELL_MAX_ARGS] = {0};
    int argc;

    argc = shell_parse_line(line, argv, SHELL_MAX_ARGS);

    expect_int("mkdir argc", argc, 2);
    expect_str("mkdir argv0", argv[0], "mkdir");
    expect_str("mkdir argv1", argv[1], "dir");
}

static void test_echo_quoted_arg(void)
{
    char line[] = "echo \"Hola\"";
    char *argv[SHELL_MAX_ARGS] = {0};
    char expected[6];
    int argc;

    expected[0] = SHELL_QUOTED_ARG_MARKER;
    strcpy(expected + 1, "Hola");

    argc = shell_parse_line(line, argv, SHELL_MAX_ARGS);

    expect_int("echo argc", argc, 2);
    expect_str("echo argv0", argv[0], "echo");
    expect_str("echo argv1", argv[1], expected);
}

static void test_redirect_is_optional(void)
{
    char command[] = "mkdir dir";
    char target[SHELL_MAX_LINE + 1];
    char *argv[SHELL_MAX_ARGS] = {0};
    int argc;
    int rc;

    rc = shell_extract_redirection(command, target, sizeof(target));
    argc = shell_parse_line(command, argv, SHELL_MAX_ARGS);

    expect_int("optional redirect rc", rc, 0);
    expect_str("optional redirect target", target, "");
    expect_int("optional redirect argc", argc, 2);
    expect_str("optional redirect argv1", argv[1], "dir");
}

static void test_redirect_target(void)
{
    char command[] = "echo \"Hola\" > test";
    char target[SHELL_MAX_LINE + 1];
    char *argv[SHELL_MAX_ARGS] = {0};
    int argc;
    int rc;

    rc = shell_extract_redirection(command, target, sizeof(target));
    argc = shell_parse_line(command, argv, SHELL_MAX_ARGS);

    expect_int("redirect rc", rc, 0);
    expect_str("redirect target", target, "test");
    expect_int("redirect argc", argc, 2);
    expect_str("redirect argv0", argv[0], "echo");
}

static void test_command_sequence(void)
{
    char line[] = "echo \"hola\" ; ps";
    char command[SHELL_MAX_LINE + 1];
    char *cursor;
    int found;

    cursor = line;
    found = shell_split_next_command(&cursor, command, sizeof(command));
    expect_int("sequence first found", found, 1);
    expect_str("sequence first", command, "echo \"hola\" ");

    found = shell_split_next_command(&cursor, command, sizeof(command));
    expect_int("sequence second found", found, 1);
    expect_str("sequence second", command, "ps");

    found = shell_split_next_command(&cursor, command, sizeof(command));
    expect_int("sequence end", found, 0);
}

static void test_sequence_with_redirect(void)
{
    char line[] = "echo \"hola\" ; ps > tasks";
    char command[SHELL_MAX_LINE + 1];
    char target[SHELL_MAX_LINE + 1];
    char *argv[SHELL_MAX_ARGS] = {0};
    char *cursor;
    int found;
    int argc;

    cursor = line;
    found = shell_split_next_command(&cursor, command, sizeof(command));
    expect_int("redirect sequence first found", found, 1);
    expect_int("redirect sequence first rc", shell_extract_redirection(command, target, sizeof(target)), 0);
    argc = shell_parse_line(command, argv, SHELL_MAX_ARGS);
    expect_int("redirect sequence first argc", argc, 2);
    expect_str("redirect sequence first target", target, "");

    found = shell_split_next_command(&cursor, command, sizeof(command));
    expect_int("redirect sequence second found", found, 1);
    expect_int("redirect sequence second rc", shell_extract_redirection(command, target, sizeof(target)), 0);
    argc = shell_parse_line(command, argv, SHELL_MAX_ARGS);
    expect_int("redirect sequence second argc", argc, 1);
    expect_str("redirect sequence second argv0", argv[0], "ps");
    expect_str("redirect sequence second target", target, "tasks");
}

static void test_extra_spaces(void)
{
    char command[] = "  ls   -l  ";
    char target[SHELL_MAX_LINE + 1];
    char *argv[SHELL_MAX_ARGS] = {0};
    int argc;

    expect_int("spaces redirect rc", shell_extract_redirection(command, target, sizeof(target)), 0);
    argc = shell_parse_line(command, argv, SHELL_MAX_ARGS);

    expect_int("spaces argc", argc, 2);
    expect_str("spaces argv0", argv[0], "ls");
    expect_str("spaces argv1", argv[1], "-l");
}

int main(void)
{
    test_mkdir_arg();
    test_echo_quoted_arg();
    test_redirect_is_optional();
    test_redirect_target();
    test_command_sequence();
    test_sequence_with_redirect();
    test_extra_spaces();

    if (failures != 0) {
        return 1;
    }

    return 0;
}
