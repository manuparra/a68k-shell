#include <stdio.h>
#include <string.h>

#include <dos/dos.h>
#include <inline/dos_protos.h>

#include "commands/vi.h"

#define VI_MAX_LINES 256
#define VI_MAX_LINE_LENGTH 160
#define VI_SCREEN_LINES 20
#define VI_COMMAND_LENGTH 80

extern struct DosLibrary *DOSBase;

struct ViEditor {
    char filename[108];
    int line_count;
    int current_line;
    int top_line;
    int dirty;
    char status[120];
};

static char vi_lines[VI_MAX_LINES][VI_MAX_LINE_LENGTH + 1];

static void trim_newline(char *line)
{
    int len;

    len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[len - 1] = '\0';
        len--;
    }
}

static int read_editor_line(char *line, int line_size)
{
    int len;
    int ch;
    int truncated;

    if (fgets(line, line_size, stdin) == 0) {
        return 0;
    }

    truncated = 0;
    len = strlen(line);
    if (len > 0 && line[len - 1] != '\n' && line[len - 1] != '\r') {
        truncated = 1;
        while ((ch = getchar()) != EOF && ch != '\n' && ch != '\r') {
        }
    }

    trim_newline(line);
    return truncated ? 2 : 1;
}

static void set_status(struct ViEditor *editor, const char *message)
{
    strncpy(editor->status, message, sizeof(editor->status) - 1);
    editor->status[sizeof(editor->status) - 1] = '\0';
}

static void clear_screen(void)
{
    int i;

    printf("\033[2J\033[H");
    for (i = 0; i < 2; ++i) {
        putchar('\n');
    }
}

static void ensure_cursor_visible(struct ViEditor *editor)
{
    if (editor->current_line < 0) {
        editor->current_line = 0;
    }

    if (editor->current_line >= editor->line_count) {
        editor->current_line = editor->line_count - 1;
    }

    if (editor->current_line < 0) {
        editor->current_line = 0;
    }

    if (editor->current_line < editor->top_line) {
        editor->top_line = editor->current_line;
    }

    if (editor->current_line >= editor->top_line + VI_SCREEN_LINES) {
        editor->top_line = editor->current_line - VI_SCREEN_LINES + 1;
    }

    if (editor->top_line < 0) {
        editor->top_line = 0;
    }
}

static void editor_draw(struct ViEditor *editor)
{
    int i;
    int index;

    ensure_cursor_visible(editor);
    clear_screen();

    printf("A68K vi - %s%s\n", editor->filename, editor->dirty ? " [+]" : "");
    puts("------------------------------------------------------------");

    for (i = 0; i < VI_SCREEN_LINES; ++i) {
        index = editor->top_line + i;
        if (index < editor->line_count) {
            printf("%c%3d %s\n",
                index == editor->current_line ? '>' : ' ',
                index + 1,
                vi_lines[index]);
        } else {
            puts("~");
        }
    }

    puts("------------------------------------------------------------");
    puts("j/k move  g/G first/last  i replace  o append  dd delete");
    puts(":w write  :q quit  :q! force quit  :wq write+quit");
    if (editor->status[0] != '\0') {
        puts(editor->status);
    }
}

static int append_line(struct ViEditor *editor, const char *line)
{
    if (editor->line_count >= VI_MAX_LINES) {
        set_status(editor, "vi: file too large");
        return 1;
    }

    strncpy(vi_lines[editor->line_count], line, VI_MAX_LINE_LENGTH);
    vi_lines[editor->line_count][VI_MAX_LINE_LENGTH] = '\0';
    editor->line_count++;
    return 0;
}

static int insert_line_at(struct ViEditor *editor, int index, const char *line)
{
    int i;

    if (editor->line_count >= VI_MAX_LINES) {
        set_status(editor, "vi: file too large");
        return 1;
    }

    if (index < 0) {
        index = 0;
    }

    if (index > editor->line_count) {
        index = editor->line_count;
    }

    for (i = editor->line_count; i > index; --i) {
        strcpy(vi_lines[i], vi_lines[i - 1]);
    }

    strncpy(vi_lines[index], line, VI_MAX_LINE_LENGTH);
    vi_lines[index][VI_MAX_LINE_LENGTH] = '\0';
    editor->line_count++;
    editor->dirty = 1;
    return 0;
}

static void delete_current_line(struct ViEditor *editor)
{
    int i;

    if (editor->line_count == 0) {
        set_status(editor, "vi: buffer is empty");
        return;
    }

    for (i = editor->current_line; i < editor->line_count - 1; ++i) {
        strcpy(vi_lines[i], vi_lines[i + 1]);
    }

    editor->line_count--;
    if (editor->current_line >= editor->line_count && editor->current_line > 0) {
        editor->current_line--;
    }

    editor->dirty = 1;
    set_status(editor, "1 line deleted");
}

static int load_file(struct ViEditor *editor)
{
    BPTR file;
    char ch;
    char line[VI_MAX_LINE_LENGTH + 1];
    int pos;
    LONG bytes_read;
    int truncated;

    file = Open((STRPTR)editor->filename, MODE_OLDFILE);
    if (file == 0) {
        editor->line_count = 0;
        set_status(editor, "new file");
        return 0;
    }

    pos = 0;
    truncated = 0;

    while ((bytes_read = Read(file, &ch, 1)) == 1) {
        if (ch == '\r') {
            continue;
        }

        if (ch == '\n') {
            line[pos] = '\0';
            if (append_line(editor, line) != 0) {
                Close(file);
                return 1;
            }
            pos = 0;
            continue;
        }

        if (pos < VI_MAX_LINE_LENGTH) {
            line[pos++] = ch;
        } else {
            truncated = 1;
        }
    }

    if (pos > 0) {
        line[pos] = '\0';
        if (append_line(editor, line) != 0) {
            Close(file);
            return 1;
        }
    }

    Close(file);

    if (bytes_read < 0) {
        set_status(editor, "vi: read error");
        return 1;
    }

    if (truncated) {
        set_status(editor, "vi: long line truncated");
    } else {
        set_status(editor, "file loaded");
    }

    return 0;
}

static int save_file(struct ViEditor *editor)
{
    BPTR file;
    int i;

    file = Open((STRPTR)editor->filename, MODE_NEWFILE);
    if (file == 0) {
        set_status(editor, "vi: cannot write file");
        return 1;
    }

    for (i = 0; i < editor->line_count; ++i) {
        if (Write(file, vi_lines[i], strlen(vi_lines[i])) < 0) {
            Close(file);
            set_status(editor, "vi: write error");
            return 1;
        }
        if (Write(file, "\n", 1) < 0) {
            Close(file);
            set_status(editor, "vi: write error");
            return 1;
        }
    }

    Close(file);
    editor->dirty = 0;
    set_status(editor, "file written");
    return 0;
}

static void insert_mode(struct ViEditor *editor, int start_index, int replace)
{
    char line[VI_MAX_LINE_LENGTH + 4];
    int index;
    int inserted;
    int read_result;

    index = start_index;
    inserted = 0;

    puts("INSERT mode: enter text. A single '.' ends insert mode.");

    if (replace && editor->line_count > 0) {
        delete_current_line(editor);
        if (index > editor->line_count) {
            index = editor->line_count;
        }
    }

    for (;;) {
        printf("I> ");
        read_result = read_editor_line(line, sizeof(line));
        if (read_result == 0) {
            set_status(editor, "insert aborted");
            return;
        }

        if (strcmp(line, ".") == 0) {
            break;
        }

        if (insert_line_at(editor, index, line) != 0) {
            return;
        }

        editor->current_line = index;
        index++;
        inserted++;

        if (read_result == 2) {
            set_status(editor, "vi: long line truncated");
        }
    }

    if (inserted == 1) {
        set_status(editor, "1 line inserted");
    } else {
        set_status(editor, "lines inserted");
    }
}

static int handle_command(struct ViEditor *editor, char *command)
{
    trim_newline(command);

    if (strcmp(command, "j") == 0) {
        if (editor->current_line < editor->line_count - 1) {
            editor->current_line++;
        }
        set_status(editor, "");
        return 0;
    }

    if (strcmp(command, "k") == 0) {
        if (editor->current_line > 0) {
            editor->current_line--;
        }
        set_status(editor, "");
        return 0;
    }

    if (strcmp(command, "g") == 0) {
        editor->current_line = 0;
        set_status(editor, "");
        return 0;
    }

    if (strcmp(command, "G") == 0) {
        editor->current_line = editor->line_count > 0 ? editor->line_count - 1 : 0;
        set_status(editor, "");
        return 0;
    }

    if (strcmp(command, "i") == 0) {
        insert_mode(editor, editor->current_line, 1);
        return 0;
    }

    if (strcmp(command, "o") == 0) {
        insert_mode(editor, editor->current_line + 1, 0);
        return 0;
    }

    if (strcmp(command, "dd") == 0) {
        delete_current_line(editor);
        return 0;
    }

    if (strcmp(command, ":w") == 0) {
        save_file(editor);
        return 0;
    }

    if (strcmp(command, ":q") == 0) {
        if (editor->dirty) {
            set_status(editor, "No write since last change (:q! to quit)");
            return 0;
        }
        return 1;
    }

    if (strcmp(command, ":q!") == 0) {
        return 1;
    }

    if (strcmp(command, ":wq") == 0) {
        if (save_file(editor) == 0) {
            return 1;
        }
        return 0;
    }

    set_status(editor, "vi: unknown command");
    return 0;
}

int command_vi(int argc, char **argv)
{
    struct ViEditor editor;
    char command[VI_COMMAND_LENGTH + 1];

    if (argc < 2) {
        puts("usage: vi <file>");
        return 1;
    }

    if (argc > 2) {
        puts("vi: too many arguments");
        return 1;
    }

    memset(&editor, 0, sizeof(editor));
    strncpy(editor.filename, argv[1], sizeof(editor.filename) - 1);
    editor.filename[sizeof(editor.filename) - 1] = '\0';

    if (load_file(&editor) != 0) {
        return 1;
    }

    for (;;) {
        editor_draw(&editor);
        printf("vi> ");

        if (fgets(command, sizeof(command), stdin) == 0) {
            putchar('\n');
            return 1;
        }

        if (handle_command(&editor, command)) {
            return 0;
        }
    }
}
