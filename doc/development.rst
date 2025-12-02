Development manual
==================

For information about the project and user documentation see
the :doc:`reference documentation <reference/index>`.


Structure
---------

The entry point and main event loop are specified in ``main.c``.

Input parsing is located in ``input.c`` and ``parser.c`` while command
execution is located in ``cmd.c``.

MSIM environment variables are handled in ``env.c``.

Physical memory logic is in ``physmem.c``.

Some useful utilities can be found in ``utils.c``, ``fault.c``, ``assert.h``,
``endian.c`` and ``list.c``.

String constants are located in ``text.c``.

Architecture dependent code is located in ``arch/`` directory.

Debugging features, including work-in-progress GDB and DAP support can be found in the
``debug`` directory.

The device interface is specified in ``device/device.h``.

All the devices are specified in the ``device`` directory.

The general CPU interface in located in ``device/cpu/general_cpu.h``.

The CPU architectures are implemented in their own directories: inside
``device/cpu/mips_r4000`` and ``device/cpu/riscv_rv32ima``.
