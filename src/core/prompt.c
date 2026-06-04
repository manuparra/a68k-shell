#include <stdio.h>
#include <string.h>

static void normalize_path(char *path)
{
    if (strncmp(path, "RAM Disk:", 9) == 0) {
        memmove(path + 4, path + 9, strlen(path + 9) + 1);
        memcpy(path, "RAM:", 4);
    }
}

void prompt_print(void)
{
    char path[160];

    path[0] = '\0';
    normalize_path(path);

    if (path[0] == '\0') {
        printf("#>");
    } else {
        printf("#%s>", path);
    }

    fflush(stdout);
}
