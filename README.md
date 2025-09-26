MSIM -- MIPS R4000 and RISC-V system simulator
==============================================

MSIM is a light-weight computer simulator based on MIPS R4000 and RISC-V.
It is used for education and research purposes, mainly to teach
the construction and implementation of operating systems.
MSIM is distributed with source code under the GNU GPL license to make
possible modifications for users and works on most POSIX-compliance
environments (mainly GNU/Linux, Mac OS X, but can be also compiled
in Cygwin or MinGW in Windows).

The user interface is simple terminal-style.

* Project homepage: https://d3s.mff.cuni.cz/software/msim/
* Documentation: https://msim.readthedocs.io/
* Source code: https://github.com/d-iii-s/msim

MSIM provides following features (along others):

* MIPS R4000 CPU
  * full memory management (TLB)
  * multiprocessor support
  * the instruction set is restricted to 32 bits
  * CPU cache is not simulated
* RISC-V RV32IMA CPU
  * A extension
  * M extension
  * Supervisor and User modes
  * multiprocessor support
  * Cache is not simulated
  * Sv32 virtual address translation with TLB simulation
* RISC-V RV64IMA CPU
  * A extension
  * M extension
  * Supervisor and User modes
  * multiprocessor support
  * Cache is not simulated
  * Sv39 virtual address translation with TLB simulation
* simple debugging features (including disassembling, register content dump)
* several simple hardware devices
* various hardware-manipulating commands
* widely configurable memory mapping of devices
* script-like start-up configuration file

MSIM does not aim to be a speed-optimized real hardware simulator, but
rather a fully deterministic simulator useful for kernel debugging.
There are several other projects which aim speed-optimized simulation
(e.g. [GXemul](http://gavare.se/gxemul/)), but they are more complex to use.


Compilation and installation
----------------------------

We provide a simple DEB package as part of every
[GitHub release](https://github.com/d-iii-s/msim/releases).
For RPM-based systems we build `msim-git` and `msim` packages at our
[COPR repository](https://copr.fedorainfracloud.org/coprs/d3s/main/)
(`msim` is build with each release while `msim-git` follows the `master`
branch).

Building from sources is possible with standard commands.

    ./configure && make && make test && sudo make install
