#include <stdio.h>

#include <dos/dos.h>
#include <inline/dos_protos.h>

#include "commands/mkdir.h"

extern struct DosLibrary *DOSBase;

int command_mkdir(int argc, char **argv)
{
    BPTR lock;

    if (argc < 2) {
        puts("usage: mkdir <directory>");
        return 1;
    }

    if (argc > 2) {
        puts("mkdir: too many arguments");
        return 1;
    }

    lock = CreateDir((STRPTR)argv[1]);
    if (lock == 0) {
        printf("mkdir: cannot create %s\n", argv[1]);
        return 1;
    }

    UnLock(lock);
    return 0;
}

