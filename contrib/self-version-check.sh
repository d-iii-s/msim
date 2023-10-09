#!/bin/bash

# Checks that version is updated in all relevant files.
# Ground truth is version number in configure.ac.

set -ueo pipefail

get_msim_version() {
    # configure.ac shall be the master file to determine the version
    grep '^AC_INIT' configure.ac | cut '-d,' -f 2 | tr -cd '0-9.'
}

check_version() {
    local where="$1"
    local actual="$( echo "$2" | sed -e 's:^[ \t]*::' -e 's:[ \t]*$::' )"
    if [ "$actual" != "$msim_version" ]; then
        echo "$where is not up-to-date (contains $actual)."
        return 1
    else
        echo "$where ok."
    fi
}

msim_version="$( get_msim_version )"

if [ "${1:-}" = "--make-release" ]; then
    new_version="${2:-}"
    if [ -z "$new_version" ]; then
        echo "Missing second parameter with new version number." >&2
        exit 1
    fi
    for i in \
            configure.ac \
            contrib/msim.spec msim-git.rpkg.spec \
            doc/Doxyfile doc/tutorial.rst \
            .github/workflows/package.yml .github/workflows/release.yml; do
        sed -i -e "s#$msim_version#$new_version#g" "$i";
    done
    autoconf
    exit 0
fi

echo "Current MSIM version is $msim_version."
check_version "configure" "$( grep '^PACKAGE_VERSION=' configure | cut -d = -f 2 | tr -d "'" )"
check_version "CHANGELOG.md" "$( grep -F "## v${msim_version} - " <CHANGELOG.md | sed 's:.*v\([0-9.]*\).*:\1:' )"
check_version "contrib/msim.spec" "$( grep '^Version:' contrib/msim.spec | cut -d : -f 2 )"
check_version "contrib/msim-VER.ebuild" "$( ls contrib/msim-*.ebuild | cut -d- -f 2 )"
check_version "doc/Doxyfile" "$( grep '^PROJECT_NUMBER' doc/Doxyfile | cut -d = -f 2 )"
check_version "doc/tutorial.rst" "$( sed -n 's#.*<msim> Alert: MSIM \([0-9].*\)#\1#p' doc/tutorial.rst )"
check_version "msim-git.rpkg.spec" "$( grep '^Version:' msim-git.rpkg.spec | sed 's:.*lead=\([^ ]*\) .*:\1:' )"
check_version ".github/workflows/package.yml" "$( grep PACKAGE_VERSION: .github/workflows/package.yml | cut -d : -f 2 )"
check_version ".github/workflows/release.yml" "$( grep PACKAGE_VERSION: .github/workflows/release.yml | cut -d : -f 2 )"
