#ifndef A68K_COMMAND_H
#define A68K_COMMAND_H

typedef int (*CommandHandler)(int argc, char **argv);

struct Command {
    const char *name;
    CommandHandler handler;
};

const struct Command *commands_get_all(void);
int commands_count(void);
int commands_dispatch(int argc, char **argv);

#endif

