#include <stdio.h>

#include <exec/execbase.h>
#include <exec/nodes.h>
#include <exec/tasks.h>
#include <inline/exec_protos.h>

#include "commands/ps.h"

#define PS_MAX_TASKS 64

extern struct ExecBase *SysBase;

struct TaskInfo {
    ULONG address;
    LONG priority;
    UBYTE type;
    UBYTE state;
    char *name;
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
    info->address = (ULONG)task;
    info->priority = (LONG)task->tc_Node.ln_Pri;
    info->type = task->tc_Node.ln_Type;
    info->state = state;
    info->name = task->tc_Node.ln_Name;
}

static void print_task(const struct TaskInfo *info)
{
    const char *name;

    name = info->name;
    if (name == 0 || name[0] == '\0') {
        name = "(unnamed)";
    }

    printf("%08lx  %3ld  %-7s %-7s %s\n",
        info->address,
        info->priority,
        task_state_name(info->state),
        node_type_name(info->type),
        name);
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

    printf("ADDRESS   PRI STATE   TYPE    NAME\n");
    for (i = 0; i < count; ++i) {
        print_task(&tasks[i]);
    }

    if (count == PS_MAX_TASKS) {
        puts("ps: task list truncated");
    }

    return 0;
}
