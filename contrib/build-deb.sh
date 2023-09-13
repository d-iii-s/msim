#!/bin/sh

set -ue

get_msim_version() {
    # configure.ac shall be the master file to determine the version
    grep '^AC_INIT' configure.ac | cut '-d,' -f 2 | tr -cd '0-9.'
}

cd "$( dirname "$0" )/.."


msim_version="$( get_msim_version )"

# Prepare Debian packaging directory with DEBIAN/control
rm -rf debian-package
mkdir -p debian-package/msim/DEBIAN
cat >debian-package/msim/DEBIAN/control <<EOF
Package: msim
Version: $msim_version-git
Section: contrib/misc
Priority: optional
Maintainer: Vojtech Horky <horky@d3s.mff.cuni.cz>
Standards-Version: $msim_version
Homepage: https://github.com/d-iii-s/msim
Vcs-Git: https://github.com/d-iii-s/msim.git
Architecture: all
Depends: libc6, libreadline-dev
Description: Light-weight computer simulator based on MIPS R4000
 MSIM is a light-weight computer simulator based on MIPS R4000.
 It is used for education and research purposes, mainly to teach
 the construction and implementation of operating systems.
 MSIM is distributed with source code under the GNU GPL license to make
 possible modifications for users and works on most POSIX-compliance
 environments (mainly GNU/Linux, Mac OS X, but can be also compiled
 in Cygwin or MinGW in Windows)
EOF


# Build MSIM
make distclean || true
./configure --prefix=/usr
make

# And create a package from it
make install DESTDIR="$( pwd )/debian-package/msim/"
dpkg-deb --build debian-package/msim "debian-package/msim_${msim_version}-git_$( dpkg-architecture -q DEB_HOST_ARCH).deb"
