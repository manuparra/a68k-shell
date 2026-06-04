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
    src/core/shell.c \
    src/core/prompt.c \
    src/core/session.c \
    src/core/commands.c \
    src/commands/cd/cd.c \
    src/commands/echo/echo.c \
    src/commands/date/date.c

printf 'Built %s\n' "$OUT"
