#!/bin/bash

# Checks that text files are in proper encoding.

set -ueo pipefail

find_sources_0() {
    git ls-tree -r --name-only -z HEAD | grep -zZ -e '[.][ch]$' -e '[.]in$' -e '[.]sh$'
}

find_crlf_files() {
    find_sources_0 | xargs -0 grep -lUP '\r$'
}

print_non_utf8_files() {
    local i
    local mime
    local rc=0
    for i in "$@"; do
        mime="$( file --mime-encoding -b "$i" )"
        case "$mime" in
            us-ascii|utf-8) ;;
            *) echo "Error: $i is in bad encoding ($mime)." >&2 ; rc=1 ;;
        esac
    done
    return "$rc"
}
export -f print_non_utf8_files

find_non_utf8_files() {
    find_sources_0 | xargs -0 bash -c 'print_non_utf8_files "$@"'
}

echo "Checking line endings..." >&2
if [ "$( find_crlf_files | wc -l )" -ne 0 ]; then
    echo "Error: following file(s) have CR-LF line endings." >&2
    find_crlf_files >&2
    exit 1
fi

echo "Checking file encodings..." >&2
find_non_utf8_files
