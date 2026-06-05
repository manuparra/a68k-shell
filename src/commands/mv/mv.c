#include <stdio.h>

#include <dos/dos.h>
#include <inline/dos_protos.h>

#include "commands/mv.h"

extern struct DosLibrary *DOSBase;

int command_mv(int argc, char **argv)
{
    if (argc != 3) {
        puts("usage: mv <source> <destination>");
        return 1;
    }

    if (Rename((STRPTR)argv[1], (STRPTR)argv[2]) == 0) {
        printf("mv: cannot move %s to %s\n", argv[1], argv[2]);
        return 1;
    }

    return 0;
}
