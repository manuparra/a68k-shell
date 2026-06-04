#include <stdio.h>
#include <string.h>

#include <dos/dos.h>
#include <inline/dos_protos.h>

#include "session.h"

extern struct DosLibrary *DOSBase;

static BPTR current_lock = 0;

int session_change_dir(const char *path)
{
    BPTR new_lock;
    BPTR old_lock;

    if (path == 0 || path[0] == '\0') {
        puts("cd: missing operand");
        return 1;
    }

    new_lock = Lock((STRPTR)path, ACCESS_READ);
    if (new_lock == 0) {
        printf("cd: cannot access %s\n", path);
        return 1;
    }

    old_lock = CurrentDir(new_lock);
    if (current_lock != 0) {
        UnLock(current_lock);
    }
    current_lock = old_lock;

    return 0;
}

void session_cleanup(void)
{
    BPTR old_lock;

    if (current_lock != 0) {
        old_lock = CurrentDir(current_lock);
        if (old_lock != 0) {
            UnLock(old_lock);
        }
        current_lock = 0;
    }
}
