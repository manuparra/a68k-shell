#include <stdio.h>

#include <dos/dos.h>
#include <inline/dos_protos.h>

#include "commands/rm.h"

extern struct DosLibrary *DOSBase;

int command_rm(int argc, char **argv)
{
    if (argc < 2) {
        puts("usage: rm <file-or-empty-directory>");
        return 1;
    }

    if (argc > 2) {
        puts("rm: too many arguments");
        return 1;
    }

    if (DeleteFile((STRPTR)argv[1]) == 0) {
        printf("rm: cannot remove %s\n", argv[1]);
        return 1;
    }

    return 0;
}

