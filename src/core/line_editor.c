#include <stdio.h>
#include <string.h>

#include <dos/dos.h>
#include <inline/dos_protos.h>

#include "history.h"
#include "line_editor.h"
#include "prompt.h"
#include "shell.h"

extern struct DosLibrary *DOSBase;

#define KEY_EOF -1
#define KEY_ENTER -2
#define KEY_BACKSPACE -3
#define KEY_UP -4
#define KEY_DOWN -5

static int raw_mode_enabled = 0;

static void set_raw_mode(void)
{
    if (!raw_mode_enabled) {
        fputs("\233" "20h", stdout);
        fflush(stdout);
        raw_mode_enabled = 1;
    }
}

void line_editor_end(void)
{
    if (raw_mode_enabled) {
        fputs("\233" "20l", stdout);
        fflush(stdout);
        raw_mode_enabled = 0;
    }
}

static void redraw_line(const char *prompt, const char *line, int old_len)
{
    int i;
    int visible_len;

    visible_len = strlen(prompt) + strlen(line);

    putchar('\r');
    fputs(prompt, stdout);
    fputs(line, stdout);

    for (i = visible_len; i < old_len; ++i) {
        putchar(' ');
    }

    putchar('\r');
    fputs(prompt, stdout);
    fputs(line, stdout);
    fflush(stdout);
}

static int read_key(void)
{
    BPTR input;
    char ch;
    unsigned char c;
    unsigned char c2;
    unsigned char c3;

    input = Input();
    if (Read(input, &ch, 1) != 1) {
        return KEY_EOF;
    }

    c = (unsigned char)ch;

    if (c == '\n' || c == '\r') {
        return KEY_ENTER;
    }

    if (c == '\b' || c == 127) {
        return KEY_BACKSPACE;
    }

    if (c == 27) {
        if (Read(input, &ch, 1) != 1) {
            return KEY_EOF;
        }
        c2 = (unsigned char)ch;

        if (c2 == '[') {
            if (Read(input, &ch, 1) != 1) {
                return KEY_EOF;
            }
            c3 = (unsigned char)ch;
            if (c3 == 'A') {
                return KEY_UP;
            }
            if (c3 == 'B') {
                return KEY_DOWN;
            }
        }

        return 0;
    }

    if (c == 155) {
        if (Read(input, &ch, 1) != 1) {
            return KEY_EOF;
        }
        c2 = (unsigned char)ch;
        if (c2 == 'A') {
            return KEY_UP;
        }
        if (c2 == 'B') {
            return KEY_DOWN;
        }
        return 0;
    }

    return c;
}

int line_editor_read(char *line, int line_size)
{
    char prompt[180];
    char draft[SHELL_MAX_LINE + 1];
    const char *history_line;
    int len;
    int old_visible_len;
    int history_index;
    int key;

    set_raw_mode();
    prompt_build(prompt, sizeof(prompt));
    fputs(prompt, stdout);
    fflush(stdout);

    line[0] = '\0';
    draft[0] = '\0';
    len = 0;
    old_visible_len = strlen(prompt);
    history_index = history_count();

    for (;;) {
        key = read_key();

        if (key == KEY_EOF) {
            return 0;
        }

        if (key == KEY_ENTER) {
            line[len] = '\0';
            putchar('\n');
            return 1;
        }

        if (key == KEY_BACKSPACE) {
            if (len > 0) {
                len--;
                line[len] = '\0';
                redraw_line(prompt, line, old_visible_len);
                old_visible_len = strlen(prompt) + len;
            }
            continue;
        }

        if (key == KEY_UP) {
            if (history_count() == 0) {
                continue;
            }

            if (history_index == history_count()) {
                strcpy(draft, line);
            }

            if (history_index > 0) {
                history_index--;
            }

            history_line = history_get(history_index);
            if (history_line != 0) {
                strncpy(line, history_line, line_size - 1);
                line[line_size - 1] = '\0';
                len = strlen(line);
                redraw_line(prompt, line, old_visible_len);
                old_visible_len = strlen(prompt) + len;
            }
            continue;
        }

        if (key == KEY_DOWN) {
            if (history_index < history_count()) {
                history_index++;
            }

            if (history_index == history_count()) {
                strncpy(line, draft, line_size - 1);
                line[line_size - 1] = '\0';
            } else {
                history_line = history_get(history_index);
                if (history_line != 0) {
                    strncpy(line, history_line, line_size - 1);
                    line[line_size - 1] = '\0';
                }
            }

            len = strlen(line);
            redraw_line(prompt, line, old_visible_len);
            old_visible_len = strlen(prompt) + len;
            continue;
        }

        if (key >= 32 && key < 127 && len < line_size - 1) {
            line[len++] = key;
            line[len] = '\0';
            putchar(key);
            fflush(stdout);
            old_visible_len = strlen(prompt) + len;
        }
    }
}
