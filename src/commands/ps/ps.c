#include <stdio.h>
#include <string.h>

#include <dos/dosextens.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/nodes.h>
#include <exec/tasks.h>
#include <inline/exec_protos.h>

#include "commands/ps.h"
#include "output.h"

#define PS_MAX_TASKS 64

extern struct ExecBase *SysBase;

struct TaskInfo {
    ULONG address;
    LONG priority;
    UBYTE type;
    UBYTE state;
    UBYTE flags;
    ULONG sig_alloc;
    ULONG sig_wait;
    ULONG sig_recvd;
    ULONG sp;
    ULONG stack_lower;
    ULONG stack_upper;
    ULONG mem_bytes;
    UWORD mem_entries;
    LONG cli_number;
    ULONG seg_list;
    ULONG current_dir;
    ULONG input_stream;
    ULONG output_stream;
    ULONG console_task;
    ULONG filesystem_task;
    char name[32];
};

static const char *task_state_name(UBYTE state)
{
    switch (state) {
    case TS_RUN:
        return "RUN";
    case TS_READY:
        return "READY";
    case TS_WAIT:
        return "WAIT";
    case TS_EXCEPT:
        return "EXCEPT";
    case TS_ADDED:
        return "ADDED";
    case TS_REMOVED:
        return "REMOVED";
    default:
        return "UNKNOWN";
    }
}

static const char *node_type_name(UBYTE type)
{
    switch (type) {
    case NT_TASK:
        return "task";
    case NT_PROCESS:
        return "process";
    default:
        return "node";
    }
}

static void capture_task(struct TaskInfo *info, struct Task *task, UBYTE state)
{
    struct Process *process;
    struct Node *node;
    struct MemList *mem_list;
    int i;

    info->address = (ULONG)task;
    info->priority = (LONG)task->tc_Node.ln_Pri;
    info->type = task->tc_Node.ln_Type;
    info->state = state;
    info->flags = task->tc_Flags;
    info->sig_alloc = task->tc_SigAlloc;
    info->sig_wait = task->tc_SigWait;
    info->sig_recvd = task->tc_SigRecvd;
    info->sp = (ULONG)task->tc_SPReg;
    info->stack_lower = (ULONG)task->tc_SPLower;
    info->stack_upper = (ULONG)task->tc_SPUpper;
    info->mem_bytes = 0;
    info->mem_entries = 0;
    info->cli_number = 0;
    info->seg_list = 0;
    info->current_dir = 0;
    info->input_stream = 0;
    info->output_stream = 0;
    info->console_task = 0;
    info->filesystem_task = 0;

    if (task->tc_Node.ln_Name != 0) {
        strncpy(info->name, task->tc_Node.ln_Name, sizeof(info->name) - 1);
        info->name[sizeof(info->name) - 1] = '\0';
    } else {
        strcpy(info->name, "(unnamed)");
    }

    node = task->tc_MemEntry.lh_Head;
    while (node->ln_Succ != 0) {
        mem_list = (struct MemList *)node;
        for (i = 0; i < mem_list->ml_NumEntries; ++i) {
            info->mem_bytes += mem_list->ml_ME[i].me_Length;
            info->mem_entries++;
        }
        node = node->ln_Succ;
    }

    if (task->tc_Node.ln_Type == NT_PROCESS) {
        process = (struct Process *)task;
        info->cli_number = process->pr_TaskNum;
        info->seg_list = (ULONG)process->pr_SegList;
        info->current_dir = (ULONG)process->pr_CurrentDir;
        info->input_stream = (ULONG)process->pr_CIS;
        info->output_stream = (ULONG)process->pr_COS;
        info->console_task = (ULONG)process->pr_ConsoleTask;
        info->filesystem_task = (ULONG)process->pr_FileSystemTask;
    }
}

static void print_task(const struct TaskInfo *info)
{
    const char *name;

    name = info->name;
    if (name == 0 || name[0] == '\0') {
        name = "(unnamed)";
    }

    output_printf("%08lx  %3ld  %-7s %-7s %s\n",
        info->address,
        info->priority,
        task_state_name(info->state),
        node_type_name(info->type),
        name);
}

static void print_task_amiga(const struct TaskInfo *info)
{
    ULONG stack_size;
    ULONG stack_used;
    ULONG stack_free;

    stack_size = 0;
    stack_used = 0;
    stack_free = 0;

    if (info->stack_upper > info->stack_lower) {
        stack_size = info->stack_upper - info->stack_lower;
        if (info->sp >= info->stack_lower && info->sp <= info->stack_upper) {
            stack_used = info->stack_upper - info->sp;
            stack_free = info->sp - info->stack_lower;
        }
    }

    output_printf("          stack=%lu used=%lu free=%lu sp=%08lx flags=%02lx\n",
        stack_size,
        stack_used,
        stack_free,
        info->sp,
        (ULONG)info->flags);
    output_printf("          sigalloc=%08lx sigwait=%08lx sigrecvd=%08lx mem=%lu/%lu\n",
        info->sig_alloc,
        info->sig_wait,
        info->sig_recvd,
        (ULONG)info->mem_entries,
        info->mem_bytes);

    if (info->type == NT_PROCESS) {
        output_printf("          cli=%ld seg=%08lx curdir=%08lx cis=%08lx cos=%08lx\n",
            info->cli_number,
            info->seg_list,
            info->current_dir,
            info->input_stream,
            info->output_stream);
        output_printf("          console=%08lx filesystem=%08lx\n",
            info->console_task,
            info->filesystem_task);
    }
}

static void capture_task_list(struct List *list, UBYTE state,
    struct TaskInfo *tasks, int *count)
{
    struct Node *node;

    node = list->lh_Head;
    while (node->ln_Succ != 0 && *count < PS_MAX_TASKS) {
        capture_task(&tasks[*count], (struct Task *)node, state);
        (*count)++;
        node = node->ln_Succ;
    }
}

int command_ps(int argc, char **argv)
{
    struct Task *current;
    struct TaskInfo tasks[PS_MAX_TASKS];
    int count;
    int i;
    int amiga_details;

    amiga_details = 0;
    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-a") == 0) {
            amiga_details = 1;
        } else {
            output_printf("ps: unsupported option %s\n", argv[i]);
            return 1;
        }
    }

    count = 0;

    Forbid();

    current = SysBase->ThisTask;
    if (current != 0 && count < PS_MAX_TASKS) {
        capture_task(&tasks[count], current, current->tc_State);
        count++;
    }

    capture_task_list(&SysBase->TaskReady, TS_READY, tasks, &count);
    capture_task_list(&SysBase->TaskWait, TS_WAIT, tasks, &count);

    Permit();

    output_printf("ADDRESS   PRI STATE   TYPE    NAME\n");
    for (i = 0; i < count; ++i) {
        print_task(&tasks[i]);
        if (amiga_details) {
            print_task_amiga(&tasks[i]);
        }
    }

    if (count == PS_MAX_TASKS) {
        output_puts("ps: task list truncated");
    }

    return 0;
}
