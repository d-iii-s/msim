MSIM
====

MSIM is a light-weight computer simulator based on MIPS R4000 and RISC-V.
It is used for education and research purposes, mainly to teach
the construction and implementation of operating systems.
Following features are available:

- MIPS R4000 CPU

  - full memory management (TLB)
  - multiprocessor support
  - the instruction set is restricted to 32 bits
  - CPU cache is not simulated

- RISC-V RV32IMA CPU

  - A extension
  - M extension
  - Supervisor and User modes
  - multiprocessor support
  - cache is not simulated
  - Sv32 virtual address translation with TLB simulation

- simple debugging features (including disassembling, register content dump)
- several simple hardware devices
- various hardware-manipulating commands
- widely configurable memory mapping of devices
- script-like start-up configuration file


MSIM is distributed with source code under the GNU GPL license to make
possible modifications for users and works on most POSIX-compliance
environments (mainly GNU/Linux, Mac OS X, but can be also compiled
in Cygwin or MinGW in Windows). See :doc:`installation instructions <reference/deployment>`.




Table of Contents
-----------------

.. toctree::
   :maxdepth: 2

   tutorial
   reference/index
   development
