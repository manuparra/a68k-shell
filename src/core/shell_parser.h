#ifndef SHELL_PARSER_H
#define SHELL_PARSER_H

#define SHELL_QUOTED_ARG_MARKER '\001'

int shell_parse_line(char *line, char **argv, int max_args);
int shell_split_next_command(char **cursor, char *command, int command_size);
int shell_extract_redirection(char *command, char *target, int target_size);
void shell_unquote_args(char **argv, int argc);

#endif
