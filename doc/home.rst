MSIM Features
=============

MSIM is a light-weight computer simulator based on MIPS R4000 and RISC-V.
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
* RISC-V RV32IMA CPU  
    * A extension
    * M extension
    * Supervisor and User modes
    * multiprocessor support
    * Cache is not simulated
    * 
* simple debugging features (including disassSv32 virtual address translation with TLB simulationembling, register content dump)
* several simple hardware devices
* various hardware-manipulating commands
* widely configurable memory mapping of devices
* script-like start-up configuration file
