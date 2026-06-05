#include <stdio.h>

#include <dos/dos.h>
#include <exec/memory.h>
#include <inline/exec_protos.h>
#include <inline/dos_protos.h>

#include "commands/dir.h"
#include "output.h"

extern struct DosLibrary *DOSBase;
extern struct ExecBase *SysBase;

static int list_dir(const char *path)
{
    BPTR lock;
    struct FileInfoBlock *fib;
    int column;

    lock = Lock((STRPTR)path, ACCESS_READ);
    if (lock == 0) {
        printf("dir: cannot access %s\n", path);
        return 1;
    }

    fib = (struct FileInfoBlock *)AllocMem(sizeof(struct FileInfoBlock), MEMF_CLEAR);
    if (fib == 0) {
        output_puts("dir: out of memory");
        UnLock(lock);
        return 1;
    }

    if (Examine(lock, fib) == 0) {
        printf("dir: cannot examine %s\n", path);
        FreeMem(fib, sizeof(struct FileInfoBlock));
        UnLock(lock);
        return 1;
    }

    column = 0;
    while (ExNext(lock, fib) != 0) {
        output_fputs(fib->fib_FileName);
        if (fib->fib_DirEntryType > 0) {
            output_putchar('/');
        }

        column++;
        if (column == 4) {
            output_putchar('\n');
            column = 0;
        } else {
            output_putchar('\t');
        }
    }

    if (column != 0) {
        output_putchar('\n');
    }

    FreeMem(fib, sizeof(struct FileInfoBlock));
    UnLock(lock);
    return 0;
}

int command_dir(int argc, char **argv)
{
    const char *path;

    if (argc > 2) {
        puts("usage: dir [directory]");
        return 1;
    }

    path = "";
    if (argc == 2) {
        path = argv[1];
    }

    return list_dir(path);
}
