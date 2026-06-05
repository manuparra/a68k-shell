#include <stdio.h>
#include <string.h>

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <inline/dos_protos.h>

#include "commands/df.h"
#include "output.h"

#define DF_MAX_SEEN 32

extern struct DosLibrary *DOSBase;

static char seen_volumes[DF_MAX_SEEN][40];
static int seen_count;

static void bstr_to_cstr(BSTR bstr, char *out, int out_size)
{
    UBYTE *source;
    int len;
    int i;

    if (out_size <= 0) {
        return;
    }

    out[0] = '\0';
    if (bstr == 0) {
        return;
    }

    source = (UBYTE *)BADDR(bstr);
    len = source[0];
    if (len > out_size - 1) {
        len = out_size - 1;
    }

    for (i = 0; i < len; ++i) {
        out[i] = source[i + 1];
    }
    out[len] = '\0';
}

static int already_seen(const char *name)
{
    int i;

    for (i = 0; i < seen_count; ++i) {
        if (strcmp(seen_volumes[i], name) == 0) {
            return 1;
        }
    }

    return 0;
}

static void remember_seen(const char *name)
{
    if (seen_count >= DF_MAX_SEEN || already_seen(name)) {
        return;
    }

    strncpy(seen_volumes[seen_count], name, sizeof(seen_volumes[seen_count]) - 1);
    seen_volumes[seen_count][sizeof(seen_volumes[seen_count]) - 1] = '\0';
    seen_count++;
}

static void make_lock_name(const char *volume, char *out, int out_size)
{
    strncpy(out, volume, out_size - 1);
    out[out_size - 1] = '\0';

    if (strchr(out, ':') == 0) {
        strncat(out, ":", out_size - strlen(out) - 1);
    }
}

static ULONG blocks_to_kb(ULONG blocks, ULONG bytes_per_block)
{
    return (blocks / 1024) * bytes_per_block
        + ((blocks % 1024) * bytes_per_block) / 1024;
}

static int print_volume(const char *volume)
{
    char lock_name[48];
    BPTR lock;
    struct InfoData info;
    ULONG total_kb;
    ULONG used_kb;
    ULONG free_kb;
    ULONG total_mb;
    ULONG used_mb;
    ULONG free_mb;
    LONG free_blocks;

    if (volume == 0 || volume[0] == '\0' || already_seen(volume)) {
        return 0;
    }

    make_lock_name(volume, lock_name, sizeof(lock_name));
    lock = Lock((STRPTR)lock_name, ACCESS_READ);
    if (lock == 0) {
        return 0;
    }

    if (Info(lock, &info) == 0) {
        UnLock(lock);
        return 0;
    }

    UnLock(lock);
    remember_seen(volume);

    free_blocks = info.id_NumBlocks - info.id_NumBlocksUsed;
    if (free_blocks < 0) {
        free_blocks = 0;
    }

    total_kb = blocks_to_kb(info.id_NumBlocks, info.id_BytesPerBlock);
    used_kb = blocks_to_kb(info.id_NumBlocksUsed, info.id_BytesPerBlock);
    free_kb = blocks_to_kb(free_blocks, info.id_BytesPerBlock);
    total_mb = total_kb / 1024;
    used_mb = used_kb / 1024;
    free_mb = free_kb / 1024;

    output_printf("%-12s %9lu %9lu %9lu %6lu %6lu %6lu\n",
        lock_name,
        total_kb,
        used_kb,
        free_kb,
        total_mb,
        used_mb,
        free_mb);

    return 1;
}

static int print_dos_volumes(void)
{
    struct RootNode *root;
    struct DosInfo *dos_info;
    struct DosList *entry;
    char name[40];
    int printed;

    printed = 0;
    root = DOSBase->dl_Root;
    if (root == 0 || root->rn_Info == 0) {
        return 0;
    }

    dos_info = (struct DosInfo *)BADDR(root->rn_Info);
    entry = (struct DosList *)BADDR(dos_info->di_DevInfo);

    while (entry != 0) {
        if (entry->dol_Type == DLT_VOLUME) {
            bstr_to_cstr(entry->dol_Name, name, sizeof(name));
            printed += print_volume(name);
        }
        entry = (struct DosList *)BADDR(entry->dol_Next);
    }

    return printed;
}

static int print_fallback_volumes(void)
{
    int printed;

    printed = 0;
    printed += print_volume("SYS");
    printed += print_volume("RAM");
    printed += print_volume("DF0");
    printed += print_volume("DF1");

    return printed;
}

int command_df(int argc, char **argv)
{
    int printed;

    if (argc > 1) {
        output_puts("usage: df");
        return 1;
    }

    seen_count = 0;
    output_puts("Filesystem     TotalKB    UsedKB    FreeKB TotalM  UsedM  FreeM");

    printed = print_dos_volumes();
    printed += print_fallback_volumes();

    if (printed == 0) {
        output_puts("df: no mounted volumes found");
        return 1;
    }

    return 0;
}
