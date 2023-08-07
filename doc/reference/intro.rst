Introduction
============

MSIM is a light-weight computer simulator, supporting MIPS R4000 and
RISC-V RV32IMA CPUs.

MSIM does not aim to be a speed-optimized real hardware simulator, but rather
a fully deterministic simulator useful for kernel debugging. There are several
other projects which aim speed-optimized simulation (e.g. `GXemul <http://gavare.se/gxemul/>`_),
but they are more complex to use.

Other hardware devices are designed in a very straightforward way to
emphasize basic principles of operation, but do not confuse the programmers
with historical heritage and complex issues.

There are several modes the MSIM runs in. Typically, the user will
simply configure the system and execute the code. For debugging purposes,
the simulation can be switched into *trace mode*, where all
executed instructions are displayed together with system events.
In *interactive mode* the user communicates with the simulator via
a command line interface and can modify the environment of the running
system.

MSIM is programmed in the C programming language. It is not specifically
designed to run on any architecture of the host computer and is thus very portable.
The design of the simulator is also modular and extensible. A simulator core
provides a common clock for all modules. The modules (or *devices*
in the MSIM terminology) implement a specific functionality such as the CPU,
keyboard input, character output, etc.
