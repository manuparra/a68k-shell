#include <stdio.h>

#include <dos/dos.h>
#include <inline/dos_protos.h>

#include "commands/cat.h"
#include "output.h"

extern struct DosLibrary *DOSBase;

int command_cat(int argc, char **argv)
{
    BPTR file;
    char buffer[128];
    LONG bytes_read;

    if (argc < 2) {
        puts("usage: cat <file>");
        return 1;
    }

    if (argc > 2) {
        puts("cat: too many arguments");
        return 1;
    }

    file = Open((STRPTR)argv[1], MODE_OLDFILE);
    if (file == 0) {
        printf("cat: cannot open %s\n", argv[1]);
        return 1;
    }

    while ((bytes_read = Read(file, buffer, sizeof(buffer))) > 0) {
        output_write(buffer, bytes_read);
    }

    Close(file);

    if (bytes_read < 0) {
        printf("cat: read error on %s\n", argv[1]);
        return 1;
    }

    return 0;
}
