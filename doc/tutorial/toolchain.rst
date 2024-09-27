Cross-compiler toolchain setup
==============================

This page describes how to install the toolchain (i.e., compiler, linker and
other utilities) to be able to compile for MIPS or RISC-V architecture.
That is needed if we want to run our own code in MSIM.
It is not required if we want to play with binary images only
(e.g. we can get a plug-and-play
`image of HelenOS for MSIM <https://www.helenos.org/wiki/Download>`__ that way).

Because we do not run on a MIPS or a RISC-V machine, we will need
a so-called cross-compiler. That is compiler that produces code for
a different architecture (platform) that the one we are compiling on.
Some compilers provide this feature in their base installation, for some one
needs to build the compiler with special configuration options.

We will use the standard GNU toolchain (GCC and binutils) compiler for
MIPS and RISC-V in our tutorials.

Cross-compiler is not a standard package available on all machines. We can
either compile it from sources or find a special package for our distribution.

If you happen to use some RPM-based distribution, we provide cross-compiler
toolchain in our
`COPR repository <https://copr.fedorainfracloud.org/coprs/d3s/main/>`__.
The packages are built based on
`these SPEC files <https://gitlab.com/mffd3s/nswi200/-/tree/main/copr?ref_type=heads>`__.

With a properly setup system, the following should install all the required
tools.

.. code-block:: shell

    # You might already have these
    dnf install make gcc git
    # Probably not needed if you use COPR already
    dnf install dnf-plugins-core
    # Enable our repository
    dnf copr enable d3s/main
    # Install toolchain for MIPS
    dnf install mffd3s-binutils-mipsel-linux-gnu mffd3s-gcc-mipsel-linux-gnu
    # Install toolchain for RISC-V
    dnf install mffd3s-binutils-riscv32-unknown-elf mffd3s-gcc-riscv32-unknown-elf
    # Install MSIM
    dnf install msim

We also provide a script to
`build the toolchain from sources <https://gitlab.com/mffd3s/nswi200/-/tree/main/from-sources?ref_type=heads>`__
and also a set of
`Docker images <https://gitlab.com/mffd3s/nswi200/container_registry>`__
(based on Fedora and using the COPR repository mentioned above).
