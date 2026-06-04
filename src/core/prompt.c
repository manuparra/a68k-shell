#include <stdio.h>
#include <string.h>

#include "session.h"

void prompt_print(void)
{
    const char *leaf;

    leaf = session_current_leaf();

    if (leaf[0] == '\0') {
        printf("#>");
    } else {
        printf("#%s>", leaf);
    }

    fflush(stdout);
}
