#!/usr/bin/env sh
set -eu

IMAGE="vintagecomputingcarinthia/vbcc4vcc:latest"
OUT="build/a68ksh"

mkdir -p build

docker run --rm --platform linux/amd64 \
  -v "$PWD:/work" \
  -w /work \
  "$IMAGE" \
  vc +kick13 -Iinclude -Isrc/core -I/opt/NDK_3.9/Include/include_h -o "$OUT" \
    src/core/banner.c \
    src/core/main.c \
    src/core/history.c \
    src/core/output.c \
    src/core/shell_parser.c \
    src/core/shell.c \
    src/core/prompt.c \
    src/core/session.c \
    src/core/commands.c \
    src/commands/cat/cat.c \
    src/commands/cd/cd.c \
    src/commands/cp/cp.c \
    src/commands/df/df.c \
    src/commands/dir/dir.c \
    src/commands/echo/echo.c \
    src/commands/head/head.c \
    src/commands/history/history.c \
    src/commands/ls/ls.c \
    src/commands/mkdir/mkdir.c \
    src/commands/mv/mv.c \
    src/commands/ps/ps.c \
    src/commands/pwd/pwd.c \
    src/commands/rm/rm.c \
    src/commands/tail/tail.c \
    src/commands/date/date.c

printf 'Built %s\n' "$OUT"
