#include <stdio.h>

#include <dos/dos.h>
#include <inline/dos_protos.h>

#include "commands/cp.h"

extern struct DosLibrary *DOSBase;

int command_cp(int argc, char **argv)
{
    BPTR src;
    BPTR dst;
    char buffer[512];
    LONG bytes_read;
    LONG bytes_written;

    if (argc != 3) {
        puts("usage: cp <source> <destination>");
        return 1;
    }

    src = Open((STRPTR)argv[1], MODE_OLDFILE);
    if (src == 0) {
        printf("cp: cannot open %s\n", argv[1]);
        return 1;
    }

    dst = Open((STRPTR)argv[2], MODE_NEWFILE);
    if (dst == 0) {
        printf("cp: cannot create %s\n", argv[2]);
        Close(src);
        return 1;
    }

    while ((bytes_read = Read(src, buffer, sizeof(buffer))) > 0) {
        bytes_written = Write(dst, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            printf("cp: write error on %s\n", argv[2]);
            Close(dst);
            Close(src);
            return 1;
        }
    }

    Close(dst);
    Close(src);

    if (bytes_read < 0) {
        printf("cp: read error on %s\n", argv[1]);
        return 1;
    }

    return 0;
}
