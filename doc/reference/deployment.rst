Deployment
==========

A POSIX compliant environment is required for MSIM to be compiled and
executed. It has been succesfully tested on GNU/Linux in various distributions
(Debian, Fedora, Gentoo) and platforms (x86, AMD64, UltraSPARC, PowerPC).
MSIM also runs in Solaris, OpenSolaris, FreeBSD and Mac OS X

Under Windows MSIM can be compiled and executed in Cygwin or using MinGW/MSYS
as a native Win32 console application (although the functionality might be
somehow limited).

A standard toolchain of consisting of a C compiler (preferably GCC) and
usual utilities (Bash, GNU Make) are the prerequisites for building MSIM.
The `GNU readline <http://tiswww.tis.case.edu/~chet/readline/rltop.html>`_
library is also required.


Package installation
--------------------

We provide automatically built packages for Fedora and other RPM-based
distributions over `COPR <https://copr.fedorainfracloud.org/>`_
in `our D3S repository <https://copr.fedorainfracloud.org/coprs/d3s/main/>`_.

Fedora
^^^^^^

Following commands are intended for Fedora and you might need to update them
to match your distribution.

.. code-block:: shell

    sudo dnf install dnf-plugins-core  # Only if COPR plugin is not installed
    sudo dnf copr enable d3s/main      # Add our repository
    sudo dnf install msim              # Install MSIM package
    msim --version                     # Verify package is installed

Arch Linux
^^^^^^^^^^

MSIM is available in the Arch User Repository (AUR) and can be installed
via the `msim <https://aur.archlinux.org/packages/msim>`_ or
`msim-git <https://aur.archlinux.org/packages/msim-git>`_ packages.


Manual installation
-------------------

If you cannot use prepackaged version (or you wish to modify MSIM) then the
following will guide you through the manual installation process.

TL;DR version is: clone our Git repository and execute the standard trio
if `./configure`, `make` and `sudo make install`.


Download
^^^^^^^^

The following commands will checkout the latest version from GitHub.

.. code-block:: shell

    git clone https://github.com/d-iii-s/msim.git
    cd msim


Configuration
^^^^^^^^^^^^^

To configure the package for compilation use the ``configure``
scripts. The script should detect all important compilation options
and check for prerequisites.

Specific options to the ``configure`` script can be used,
e.g. ``--prefix=`` to set the installation prefix.

.. code-block:: shell

    ./configure --prefix=/usr


Compilation
^^^^^^^^^^^

After a successful execution of ``configure`` just run
``make`` to compile the sources. No special arguments
are usually necessary.

.. code-block:: shell

    make

Note that MSIM uses ``makedepend`` for dependency generation.


Installation
^^^^^^^^^^^^

If the compilation is successful, you can use the following command
to install the binary and supplementary files into the installation
prefix. You will probably need root privileges to install MSIM
into system-wide prefix.

.. code-block:: shell

    make install
    # or sudo make install
    # or make install DESTDIR=$PWD/PKG/
