#include <stdio.h>

#include <dos/dos.h>
#include <inline/dos_protos.h>

#include "commands/head.h"
#include "output.h"

#define HEAD_LINES 5

extern struct DosLibrary *DOSBase;

int command_head(int argc, char **argv)
{
    BPTR file;
    char ch;
    LONG bytes_read;
    int lines;

    if (argc < 2) {
        puts("usage: head <file>");
        return 1;
    }

    if (argc > 2) {
        puts("head: too many arguments");
        return 1;
    }

    file = Open((STRPTR)argv[1], MODE_OLDFILE);
    if (file == 0) {
        printf("head: cannot open %s\n", argv[1]);
        return 1;
    }

    lines = 0;
    bytes_read = 0;
    while (lines < HEAD_LINES && (bytes_read = Read(file, &ch, 1)) == 1) {
        output_putchar(ch);
        if (ch == '\n') {
            lines++;
        }
    }

    Close(file);

    if (bytes_read < 0) {
        printf("head: read error on %s\n", argv[1]);
        return 1;
    }

    return 0;
}
