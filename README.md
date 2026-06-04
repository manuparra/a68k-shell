# a68k-shell

A substitute for the Amiga Workbench 1.3 Shell/CLI inspired by Linux shells.

The current MVP is a small AmigaOS 1.3-compatible interactive shell with:

- Prompt: `#>` or `#folder>` after changing directory
- Startup banner with author/contact details
- Commands: `echo`, `date`, `cd`, `pwd`, `ls`, `history`
- Built-in exit command: `exit`
- Modular command layout under `src/commands/`

The long-term goal is to grow this into a practical shell-like environment for
Workbench 1.3 while keeping the code compatible with vbcc and Kickstart 1.3.

## Build

The build uses vbcc inside Docker:

```sh
make build
```

The output binary is written to `build/a68ksh`.

The Docker image is:

```sh
vintagecomputingcarinthia/vbcc4vcc:latest
```

The build script runs it with `--platform linux/amd64` and compiles with
`vc +kick13`.

## Run in fs-UAE

Set the Kickstart 1.3 ROM path:

```sh
export FSUAE_KICKSTART_FILE=/path/to/kickstart-1.3.rom
```

Optionally set a Workbench 1.3 ADF:

```sh
export FSUAE_WORKBENCH_ADF=/path/to/Workbench1.3.adf
```

Then run:

```sh
make run-fsuae
```

The script builds `build/a68ksh` if needed, copies it to
`dist/A68KShell/a68ksh`, generates `build/a68k-shell.fs-uae`, and mounts
`dist/A68KShell` as a hard drive directory.

Inside Workbench or Amiga CLI, open the mounted directory and run:

```text
a68ksh
```

Expected smoke test:

```text
#>echo hello amiga
hello amiga
#>date
2026-06-04 19:00:00
#>cd RAM:
#RAM:>pwd
RAM:
#RAM:>ls
#RAM:>ls -l
#RAM:>history
 1 echo hello amiga
 2 date
 3 cd RAM:
 4 pwd
 5 ls
 6 ls -l
#RAM:>!5
ls
#RAM:>unknown
unknown: command not found
#RAM:>exit
```

The exact `date` value comes from the emulator/runtime clock.

## Current limitations

- The prompt tracks directories changed through this shell and displays only
  the final component, for example `#folder>`.
- `pwd` prints the shell's tracked path. It starts as `.` until the first `cd`
  because Kickstart 1.3 does not provide a simple safe `NameFromLock` path API
  in the current vbcc target.
- `ls -l` uses a Unix-like aesthetic, but AmigaOS protection bits are not Unix
  ownership/group permissions.
- No pipes, redirection, globbing, variables, quotes, or autocomplete yet.

## Commands

### echo

```text
#>echo hello amiga
hello amiga
```

### date

```text
#>date
2026-06-04 19:00:00
```

The value comes from the emulator/runtime clock.

### cd

Changes the process current directory using AmigaDOS locks:

```text
#>cd RAM:
#>cd ..
#>cd DF0:folder
```

The prompt shows the final component of the tracked path after a successful
`cd`.

### pwd

Shows the shell's tracked current directory:

```text
#>pwd
.
#>cd RAM:tmp/folder
#folder>pwd
RAM:tmp/folder
```

### ls

Lists files from the current directory or a supplied path:

```text
#>ls
#>ls RAM:
#>ls -l
#>ls -l DF0:folder
```

`ls -l` prints a Unix-inspired view with type, permission-style flags, size, and
name.

### history

Shows the last 16 commands with a number:

```text
#>history
 1 cd RAM:
 2 ls -l
```

Run a previous command with `!number`:

```text
#>!1
cd RAM:
```

The expanded command is printed before it runs.

## Development flow

Each feature should be developed in its own branch:

```sh
git switch -c feature/my-feature
```

Commit locally and push with a temporary `GITHUB_TOKEN` environment variable.
Do not store the token in the remote URL or in files.

Recommended push pattern:

```sh
export GITHUB_TOKEN=...
./scripts/push-branch.sh feature/my-feature
```

The helper creates a temporary `GIT_ASKPASS` script, reads the token from the
environment, pushes the branch, and deletes the helper.

## Adding commands

See `docs/commands.md`.
