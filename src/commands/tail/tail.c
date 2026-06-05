#include <stdio.h>
#include <string.h>

#include <dos/dos.h>
#include <inline/dos_protos.h>

#include "commands/tail.h"
#include "output.h"

#define TAIL_LINES 5
#define TAIL_LINE_LENGTH 160

extern struct DosLibrary *DOSBase;

static void append_char(char *line, int *pos, char ch)
{
    if (*pos < TAIL_LINE_LENGTH) {
        line[*pos] = ch;
        (*pos)++;
        line[*pos] = '\0';
    }
}

static void store_line(char lines[TAIL_LINES][TAIL_LINE_LENGTH + 1],
    int *next, int *count, const char *line)
{
    strncpy(lines[*next], line, TAIL_LINE_LENGTH);
    lines[*next][TAIL_LINE_LENGTH] = '\0';

    *next = (*next + 1) % TAIL_LINES;
    if (*count < TAIL_LINES) {
        (*count)++;
    }
}

int command_tail(int argc, char **argv)
{
    BPTR file;
    char lines[TAIL_LINES][TAIL_LINE_LENGTH + 1];
    char current[TAIL_LINE_LENGTH + 1];
    char ch;
    LONG bytes_read;
    int current_pos;
    int next;
    int count;
    int i;
    int index;

    if (argc < 2) {
        puts("usage: tail <file>");
        return 1;
    }

    if (argc > 2) {
        puts("tail: too many arguments");
        return 1;
    }

    file = Open((STRPTR)argv[1], MODE_OLDFILE);
    if (file == 0) {
        printf("tail: cannot open %s\n", argv[1]);
        return 1;
    }

    for (i = 0; i < TAIL_LINES; ++i) {
        lines[i][0] = '\0';
    }
    current[0] = '\0';
    current_pos = 0;
    next = 0;
    count = 0;

    while ((bytes_read = Read(file, &ch, 1)) == 1) {
        if (ch == '\r') {
            continue;
        }

        if (ch == '\n') {
            append_char(current, &current_pos, ch);
            store_line(lines, &next, &count, current);
            current[0] = '\0';
            current_pos = 0;
        } else {
            append_char(current, &current_pos, ch);
        }
    }

    Close(file);

    if (bytes_read < 0) {
        printf("tail: read error on %s\n", argv[1]);
        return 1;
    }

    if (current_pos > 0) {
        append_char(current, &current_pos, '\n');
        store_line(lines, &next, &count, current);
    }

    for (i = 0; i < count; ++i) {
        index = (next - count + i + TAIL_LINES) % TAIL_LINES;
        output_fputs(lines[index]);
    }

    return 0;
}
