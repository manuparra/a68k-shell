#!/usr/bin/env sh
set -eu

if [ -z "${GITHUB_TOKEN:-}" ]; then
  printf 'Set GITHUB_TOKEN before pushing.\n' >&2
  exit 1
fi

BRANCH="${1:-$(git branch --show-current)}"
ASKPASS="$(mktemp "${TMPDIR:-/tmp}/a68k-askpass.XXXXXX")"

cleanup()
{
  rm -f "$ASKPASS"
}
trap cleanup EXIT HUP INT TERM

cat > "$ASKPASS" <<'EOF'
#!/usr/bin/env sh
case "$1" in
  *Username*) printf '%s\n' "${GITHUB_USERNAME:-x-access-token}" ;;
  *Password*) printf '%s\n' "$GITHUB_TOKEN" ;;
  *) printf '\n' ;;
esac
EOF

chmod 700 "$ASKPASS"

GIT_ASKPASS="$ASKPASS" GIT_TERMINAL_PROMPT=0 git push -u origin "$BRANCH"

