#!/usr/bin/env sh
set -eu

if [ ! -f build/a68ksh ] || [ ! -f build/A68KShell ]; then
  ./scripts/build.sh
fi

mkdir -p dist/A68KShell
cp build/a68ksh dist/A68KShell/a68ksh
cp build/A68KShell dist/A68KShell/A68KShell

docker run --rm --platform linux/amd64 \
  -v "$PWD:/work" \
  -w /work \
  vintagecomputingcarinthia/vbcc4vcc:latest \
  cp /opt/amiga/ZShell/ZSH.info /work/dist/A68KShell/A68KShell.info

printf 'Packaged Workbench app in dist/A68KShell\n'
printf 'Double-click A68KShell from Workbench to open the shell.\n'

