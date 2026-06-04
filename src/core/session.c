#include <stdio.h>
#include <string.h>

#include <dos/dos.h>
#include <inline/dos_protos.h>

#include "session.h"

extern struct DosLibrary *DOSBase;

static BPTR active_lock = 0;
static BPTR initial_lock = 0;
static char current_path[160] = "";
static char current_leaf[80] = "";

static void trim_trailing_newline(char *path)
{
    int len;

    len = strlen(path);
    while (len > 0 && (path[len - 1] == '\n' || path[len - 1] == '\r')) {
        path[len - 1] = '\0';
        len--;
    }
}

static void trim_trailing_slash(char *path)
{
    int len;

    len = strlen(path);
    while (len > 0 && path[len - 1] == '/') {
        path[len - 1] = '\0';
        len--;
    }
}

static int has_volume(const char *path)
{
    return strchr(path, ':') != 0;
}

static void normalize_display_path(char *path)
{
    char *colon;

    trim_trailing_newline(path);
    trim_trailing_slash(path);

    colon = strchr(path, ':');
    if (colon != 0 && colon[1] == '/') {
        memmove(colon + 1, colon + 2, strlen(colon + 2) + 1);
    }
}

static void update_leaf(void)
{
    char *last_slash;
    char *colon;

    if (current_path[0] == '\0') {
        current_leaf[0] = '\0';
        return;
    }

    last_slash = strrchr(current_path, '/');
    if (last_slash != 0 && last_slash[1] != '\0') {
        strncpy(current_leaf, last_slash + 1, sizeof(current_leaf) - 1);
        current_leaf[sizeof(current_leaf) - 1] = '\0';
        return;
    }

    colon = strchr(current_path, ':');
    if (colon != 0 && colon[1] != '\0') {
        strncpy(current_leaf, colon + 1, sizeof(current_leaf) - 1);
        current_leaf[sizeof(current_leaf) - 1] = '\0';
        return;
    }

    strncpy(current_leaf, current_path, sizeof(current_leaf) - 1);
    current_leaf[sizeof(current_leaf) - 1] = '\0';
}

static void remove_last_component(void)
{
    char *last_slash;
    char *colon;

    last_slash = strrchr(current_path, '/');
    if (last_slash != 0) {
        *last_slash = '\0';
        return;
    }

    colon = strchr(current_path, ':');
    if (colon != 0 && colon[1] != '\0') {
        colon[1] = '\0';
        return;
    }

    current_path[0] = '\0';
}

static void update_display_path(const char *path)
{
    char clean[160];

    strncpy(clean, path, sizeof(clean) - 1);
    clean[sizeof(clean) - 1] = '\0';
    normalize_display_path(clean);

    if (strcmp(clean, "..") == 0) {
        remove_last_component();
        update_leaf();
        return;
    }

    if (has_volume(clean) || current_path[0] == '\0') {
        strncpy(current_path, clean, sizeof(current_path) - 1);
    } else {
        if (current_path[strlen(current_path) - 1] != ':') {
            strncat(current_path, "/", sizeof(current_path) - strlen(current_path) - 1);
        }
        strncat(current_path, clean, sizeof(current_path) - strlen(current_path) - 1);
    }

    current_path[sizeof(current_path) - 1] = '\0';
    update_leaf();
}

int session_change_dir(const char *path)
{
    BPTR new_lock;
    BPTR old_lock;
    BPTR temp_lock;

    if (path == 0 || path[0] == '\0') {
        puts("cd: missing operand");
        return 1;
    }

    if (strcmp(path, "..") == 0) {
        temp_lock = 0;
        if (active_lock != 0) {
            new_lock = ParentDir(active_lock);
        } else {
            temp_lock = Lock("", ACCESS_READ);
            new_lock = temp_lock == 0 ? 0 : ParentDir(temp_lock);
        }
        if (temp_lock != 0) {
            UnLock(temp_lock);
        }
    } else {
        new_lock = Lock((STRPTR)path, ACCESS_READ);
    }

    if (new_lock == 0) {
        printf("cd: cannot access %s\n", path);
        return 1;
    }

    old_lock = CurrentDir(new_lock);
    if (initial_lock == 0) {
        initial_lock = old_lock;
    } else if (active_lock != 0) {
        UnLock(old_lock);
    }
    active_lock = new_lock;
    update_display_path(path);

    return 0;
}

const char *session_current_path(void)
{
    if (current_path[0] == '\0') {
        return ".";
    }

    return current_path;
}

const char *session_current_leaf(void)
{
    return current_leaf;
}

void session_cleanup(void)
{
    BPTR old_lock;

    if (initial_lock != 0) {
        old_lock = CurrentDir(initial_lock);
        if (old_lock != 0) {
            UnLock(old_lock);
        }
        initial_lock = 0;
        active_lock = 0;
    } else if (active_lock != 0) {
        UnLock(active_lock);
        active_lock = 0;
    }
}
