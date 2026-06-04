# Adding commands

Commands are intentionally isolated so they can be replaced or extended without
changing the shell loop.

To add a command:

1. Create a directory under `src/commands/<name>/`.
2. Add a header under `include/commands/<name>.h`.
3. Implement a handler with this shape:

   ```c
   int command_name(int argc, char **argv);
   ```

4. Register it in `src/core/commands.c`.
5. Add the new source file to `scripts/build.sh`.
6. Build with `make build`.

Commands that call AmigaDOS should include the DOS structs from the NDK headers
and the vbcc inline prototypes from the Kickstart 1.3 target, as done by `cd`
and `ls`.

The parser is intentionally small. It splits on spaces and tabs, with no quotes,
globbing, pipes, redirection, variables, history, or autocomplete.
