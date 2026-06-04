#include <stdio.h>
#include <string.h>

static void normalize_path(char *path)
{
    if (strncmp(path, "RAM Disk:", 9) == 0) {
        memmove(path + 4, path + 9, strlen(path + 9) + 1);
        memcpy(path, "RAM:", 4);
    }
}

void prompt_build(char *buffer, int buffer_size)
{
    char path[160];

    path[0] = '\0';
    normalize_path(path);

    if (path[0] == '\0') {
        strncpy(buffer, "#>", buffer_size - 1);
    } else {
        strncpy(buffer, "#", buffer_size - 1);
        strncat(buffer, path, buffer_size - strlen(buffer) - 1);
        strncat(buffer, ">", buffer_size - strlen(buffer) - 1);
    }

    buffer[buffer_size - 1] = '\0';
}

void prompt_print(void)
{
    char prompt[180];

    prompt_build(prompt, sizeof(prompt));
    fputs(prompt, stdout);
    fflush(stdout);
}
