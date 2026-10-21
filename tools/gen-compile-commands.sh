#!/bin/sh
# Write compile_commands.json from the flags Make just expanded.
# The C/C++ extension uses this database to resolve each call to the
# function that was actually compiled.
set -eu

out=${1:?}
dir=${DIRECTORY:-$(pwd)}
tmp=${out}.tmp

json_escape() {
    printf '%s' "$1" | sed 's/\\/\\\\/g; s/"/\\"/g'
}

# Make stores a shell-quoted string macro as -DNAME='"text"'. The command
# in this database is itself parsed by a shell, which would strip those
# single quotes and leave the double quotes.
shell_flags() {
    printf '%s' "$1" | sed "s/='\"/=\"/g; s/\"'/\"/g"
}

first=1
printf '[\n' > "$tmp"

APP_CFLAGS=$(shell_flags "$(cat "$BUILD/.app-cflags")")
KERNEL_CFLAGS=$(shell_flags "$(cat "$BUILD/.kernel-cflags")")
APP_FILES=$(cat "$BUILD/.app-files")
KERNEL_FILES=$(cat "$BUILD/.kernel-files")

emit_group() {
    flags=$1
    shift
    for src in "$@"; do
        [ -n "$src" ] || continue
        if [ "$first" -eq 0 ]; then
            printf ',\n' >> "$tmp"
        fi
        first=0
        printf '  {"directory":"%s","file":"%s","command":"%s"}' \
            "$(json_escape "$dir")" \
            "$(json_escape "$dir/$src")" \
            "$(json_escape "$CC $flags -c $src")" >> "$tmp"
    done
}

# shellcheck disable=SC2086
emit_group "$APP_CFLAGS" $APP_FILES
# shellcheck disable=SC2086
emit_group "$KERNEL_CFLAGS" $KERNEL_FILES

printf '\n]\n' >> "$tmp"
mv "$tmp" "$out"
