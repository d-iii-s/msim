#!/bin/bash

# Get release notes from change log file.

set -ueo pipefail

get_changelog_for() {
    # Find text between header lines and remove those header lines
    sed -n -e "/^## v${1} - 20[0-9][0-9]-[01][0-9]-[0123][0-9]$/,/^## v/{/^## v/d;p}"
}

strip_empty_lines() {
    # https://unix.stackexchange.com/a/552195
    sed -e '/./,$!d' -e :a -e '/^\n*$/{$d;N;ba' -e '}'
}

get_changelog_for "$1" < "CHANGELOG.md" | strip_empty_lines
