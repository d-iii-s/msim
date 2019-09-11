MSIM -- MIPS R4000 system simulator
===================================

MSIM is a light-weight computer simulator based on MIPS R4000.
It is used for education and research purposes, mainly to teach
the construction and implementation of operating systems.
MSIM is distributed with source code under the GNU GPL license to make
possible modifications for users and works on most POSIX-compliance
environments (mainly GNU/Linux, Mac OS X, but can be also compiled
in Cygwin or MinGW in Windows).

The user interface is simple terminal-style.

MSIM provides following features (along others):

* MIPS R4000 CPU
    * full memory management (TLB)
    * multiprocessor support
    * the instruction set is restricted to 32 bits
    * CPU cache is not simulated
* simple debugging features (including disassembling, register content dump)
* several simple hardware devices
* various hardware-manipulating commands
* widely configurable memory mapping of devices
* script-like start-up configuration file

MSIM does not aim to be a speed-optimized real hardware simulator, but
rather a fully deterministic simulator useful for kernel debugging.
There are several other projects which aim speed-optimized simulation
(e.g. [GXemul](http://gavare.se/gxemul/)), but they are more complex to use.

Project homepage is at http://d3s.mff.cuni.cz/~holub/sw/msim/.

Development branch is available at https://github.com/D-iii-S/msim.


Compilation and installation
----------------------------

    ./configure && make install

