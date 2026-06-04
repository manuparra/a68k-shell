# a68k-shell

A substitute for the Amiga Workbench 1.3 Shell/CLI inspired by Linux shells.

The project starts with a small AmigaOS 1.3-compatible command-line program and
will grow incrementally into a modular shell replacement.

## Build

The build uses vbcc inside Docker:

```sh
make build
```

The output binary is written to `build/a68ksh`.
