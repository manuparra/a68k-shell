#include <stdio.h>

#include "commands/history.h"
#include "history.h"

int command_history(int argc, char **argv)
{
    int i;
    int count;
    const char *entry;

    count = history_count();
    for (i = 1; i <= count; ++i) {
        entry = history_get_display_number(i);
        if (entry != 0) {
            printf("%2d %s\n", i, entry);
        }
    }

    return 0;
}
