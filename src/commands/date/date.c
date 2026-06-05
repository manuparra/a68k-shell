#include <stdio.h>
#include <time.h>

#include "commands/date.h"
#include "output.h"

int command_date(int argc, char **argv)
{
    time_t now;
    struct tm *local;

    now = time(0);
    local = localtime(&now);

    if (local == 0) {
        output_puts("date unavailable");
        return 1;
    }

    output_printf("%04d-%02d-%02d %02d:%02d:%02d\n",
        local->tm_year + 1900,
        local->tm_mon + 1,
        local->tm_mday,
        local->tm_hour,
        local->tm_min,
        local->tm_sec);

    return 0;
}
