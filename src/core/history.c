#include <string.h>

#include "history.h"
#include "shell.h"

static char entries[SHELL_HISTORY_SIZE][SHELL_MAX_LINE + 1];
static int entry_count = 0;

void history_add(const char *line)
{
    int i;

    if (line == 0 || line[0] == '\0') {
        return;
    }

    if (entry_count > 0 && strcmp(entries[entry_count - 1], line) == 0) {
        return;
    }

    if (entry_count == SHELL_HISTORY_SIZE) {
        for (i = 1; i < SHELL_HISTORY_SIZE; ++i) {
            strcpy(entries[i - 1], entries[i]);
        }
        entry_count--;
    }

    strncpy(entries[entry_count], line, SHELL_MAX_LINE);
    entries[entry_count][SHELL_MAX_LINE] = '\0';
    entry_count++;
}

const char *history_get(int index)
{
    if (index < 0 || index >= entry_count) {
        return 0;
    }

    return entries[index];
}

int history_count(void)
{
    return entry_count;
}

