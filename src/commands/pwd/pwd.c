#include <stdio.h>

#include "commands/pwd.h"
#include "session.h"

int command_pwd(int argc, char **argv)
{
    printf("%s\n", session_current_path());
    return 0;
}

