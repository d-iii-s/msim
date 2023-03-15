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

Download
--------

The following commands will checkout the latest version from GitHub.

.. code-block:: shell

    git clone https://github.com/d-iii-s/msim.git
    cd git


Configuration
-------------

To configure the package for compilation use the ``configure``
scripts. The script should detect all important compilation options
and check for prerequisites.

Specific options to the ``configure`` script can be used,
e.g. ``--prefix=`` to set the installation prefix.

.. code-block:: shell

    ./configure --prefix=/usr


Compilation
-----------

After a successful execution of ``configure`` just run
``make`` to compile the sources. No special arguments
are usually necessary.

.. code-block:: shell

    make

Note that MSIM uses ``makedepend`` for dependency generation.


Installation
------------

If the compilation is successful, you can use the following command
to install the binary and supplementary files into the installation
prefix. You will probably need root privileges to install MSIM
into system-wide prefix.

.. code-block:: shell

    make install
    # or sudo make install
