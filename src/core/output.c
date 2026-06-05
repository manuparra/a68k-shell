#include <stdarg.h>
#include <stdio.h>

#include "output.h"

static FILE *current_output = 0;

void output_set(FILE *file)
{
    current_output = file == 0 ? stdout : file;
}

FILE *output_get(void)
{
    if (current_output == 0) {
        current_output = stdout;
    }

    return current_output;
}

int output_printf(const char *format, ...)
{
    va_list args;
    int result;

    va_start(args, format);
    result = vfprintf(output_get(), format, args);
    va_end(args);
    return result;
}

int output_puts(const char *text)
{
    int result;

    result = fputs(text, output_get());
    fputc('\n', output_get());
    return result;
}

int output_putchar(int ch)
{
    return fputc(ch, output_get());
}

int output_fputs(const char *text)
{
    return fputs(text, output_get());
}

int output_write(const char *buffer, int size)
{
    return fwrite(buffer, 1, size, output_get());
}

