#include <stdio.h>
#include <string.h>

#include <dos/dos.h>
#include <exec/memory.h>
#include <inline/exec_protos.h>
#include <inline/dos_protos.h>

#include "commands/ls.h"
#include "output.h"

extern struct DosLibrary *DOSBase;
extern struct ExecBase *SysBase;

static void print_permissions(LONG protection, int is_dir)
{
    char mode[11];

    mode[0] = is_dir ? 'd' : '-';
    mode[1] = (protection & FIBF_READ) ? '-' : 'r';
    mode[2] = (protection & FIBF_WRITE) ? '-' : 'w';
    mode[3] = (protection & FIBF_EXECUTE) ? '-' : 'x';
    mode[4] = 'r';
    mode[5] = '-';
    mode[6] = 'x';
    mode[7] = 'r';
    mode[8] = '-';
    mode[9] = 'x';
    mode[10] = '\0';

    output_fputs(mode);
}

static void print_date(struct DateStamp *stamp)
{
    static const char *months[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    static const int month_days[] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    LONG days;
    int year;
    int month;
    int day;
    int days_in_year;
    int days_in_month;
    int hour;
    int minute;

    days = stamp->ds_Days;
    year = 1978;

    for (;;) {
        days_in_year = 365;
        if ((year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0)) {
            days_in_year = 366;
        }

        if (days < days_in_year) {
            break;
        }

        days -= days_in_year;
        year++;
    }

    month = 0;
    for (;;) {
        days_in_month = month_days[month];
        if (month == 1
            && (year % 4) == 0
            && ((year % 100) != 0 || (year % 400) == 0)) {
            days_in_month = 29;
        }

        if (days < days_in_month) {
            break;
        }

        days -= days_in_month;
        month++;
    }

    day = (int)days + 1;
    hour = (int)(stamp->ds_Minute / 60);
    minute = (int)(stamp->ds_Minute % 60);

    output_printf("%02d-%s-%04d %02d:%02d", day, months[month], year, hour, minute);
}

static void print_entry(struct FileInfoBlock *fib, int long_format)
{
    int is_dir;

    is_dir = fib->fib_DirEntryType > 0;

    if (long_format) {
        print_permissions(fib->fib_Protection, is_dir);
        output_printf(" %8ld ", fib->fib_Size);
        print_date(&fib->fib_Date);
        output_printf("  %s", fib->fib_FileName);
    } else {
        output_fputs(fib->fib_FileName);
    }

    if (is_dir) {
        output_putchar('/');
    }

    output_putchar('\n');
}

static int list_path(const char *path, int long_format)
{
    BPTR lock;
    struct FileInfoBlock *fib;

    lock = Lock((STRPTR)path, ACCESS_READ);
    if (lock == 0) {
        printf("ls: cannot access %s\n", path);
        return 1;
    }

    fib = (struct FileInfoBlock *)AllocMem(sizeof(struct FileInfoBlock), MEMF_CLEAR);
    if (fib == 0) {
        output_puts("ls: out of memory");
        UnLock(lock);
        return 1;
    }

    if (Examine(lock, fib) == 0) {
        printf("ls: cannot examine %s\n", path);
        FreeMem(fib, sizeof(struct FileInfoBlock));
        UnLock(lock);
        return 1;
    }

    while (ExNext(lock, fib) != 0) {
        print_entry(fib, long_format);
    }

    FreeMem(fib, sizeof(struct FileInfoBlock));
    UnLock(lock);
    return 0;
}

int command_ls(int argc, char **argv)
{
    int long_format;
    const char *path;
    int i;

    long_format = 0;
    path = "";

    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-l") == 0) {
            long_format = 1;
        } else if (argv[i][0] == '-') {
            printf("ls: unsupported option %s\n", argv[i]);
            return 1;
        } else {
            path = argv[i];
        }
    }

    return list_path(path, long_format);
}
