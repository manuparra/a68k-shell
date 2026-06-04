#include <string.h>

#include "history.h"
#include "shell.h"

static char entries[SHELL_HISTORY_SIZE][SHELL_MAX_LINE + 1];
static int entry_count = 0;

static void trim_line(char *line)
{
    int len;

    len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[len - 1] = '\0';
        len--;
    }
}

void history_add(const char *line)
{
    char clean[SHELL_MAX_LINE + 1];
    int i;

    if (line == 0 || line[0] == '\0') {
        return;
    }

    strncpy(clean, line, SHELL_MAX_LINE);
    clean[SHELL_MAX_LINE] = '\0';
    trim_line(clean);

    if (clean[0] == '\0') {
        return;
    }

    if (entry_count > 0 && strcmp(entries[entry_count - 1], clean) == 0) {
        return;
    }

    if (entry_count == SHELL_HISTORY_SIZE) {
        for (i = 1; i < SHELL_HISTORY_SIZE; ++i) {
            strcpy(entries[i - 1], entries[i]);
        }
        entry_count--;
    }

    strcpy(entries[entry_count], clean);
    entry_count++;
}

const char *history_get_display_number(int number)
{
    if (number < 1 || number > entry_count) {
        return 0;
    }

    return entries[number - 1];
}

int history_count(void)
{
    return entry_count;
}

