#!/usr/bin/env sh
set -eu

IMAGE="vintagecomputingcarinthia/vbcc4vcc:latest"
OUT="build/a68ksh"

mkdir -p build

docker run --rm --platform linux/amd64 \
  -v "$PWD:/work" \
  -w /work \
  "$IMAGE" \
  vc +kick13 -Iinclude -o "$OUT" src/core/main.c

printf 'Built %s\n' "$OUT"

