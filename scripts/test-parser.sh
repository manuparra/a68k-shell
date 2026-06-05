#!/usr/bin/env sh
set -eu

mkdir -p build

cc -Wall -Wextra -Werror -Iinclude -Isrc/core \
  -o build/test-shell-parser \
  tests/test_shell_parser.c \
  src/core/shell_parser.c

build/test-shell-parser
