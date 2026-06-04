#include <stdio.h>

#include "commands/cd.h"
#include "session.h"

int command_cd(int argc, char **argv)
{
    if (argc < 2) {
        puts("usage: cd <directory>");
        return 1;
    }

    if (argc > 2) {
        puts("cd: too many arguments");
        return 1;
    }

    return session_change_dir(argv[1]);
}

