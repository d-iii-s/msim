Special instructions
====================

To further improve the ease of debugging the code running in MSIM, there are
several non-standard MIPS and RISC-V instructions available.
This allows to change the behaviour of the simulator exactly at the place of
code required, with only a minimal influence to the debugged code.

.. contents:: Overview
   :local:



MIPS instructions
-----------------

Trace on ``DTRC``
^^^^^^^^^^^^^^^^^

Enable the trace mode.
Together with ``DTRO`` instruction, ``DTRC`` is useful to show a part
of executed code.

**Opcode**: ``0x39``


Trace off ``DTRO``
^^^^^^^^^^^^^^^^^^

Disable the trace mode.
Together with ``DTRC`` instruction, ``DTRO`` is useful to show a part of
executed code.

**Opcode**: ``0x3d``


Interactive mode on ``DINT``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Enable the interactive mode. The command line is activated immediately.

**Opcode**: ``0x29``


Dump general registers ``DRV``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The instruction dumps the content of all the general registers to the screen.

**Opcode**: ``0x37``


Halt machine ``DHLT``
^^^^^^^^^^^^^^^^^^^^^

Halt the machine as well as the simulation immediately.
Useful also as the power off feature.

**Opcode**: ``0x28``


View value ``DVAL``
^^^^^^^^^^^^^^^^^^^

Print the content of the ``a0`` (``r4``) register.
The instruction can be used to instantly print a value of a variable.

**Opcode**: `0x35``


CP0 Register view ``DCRV``
^^^^^^^^^^^^^^^^^^^^^^^^^^

The instruction dumps the content of all the registers of
CP0 coprocessor to the screen.

**Opcode**: ``0x0e``


GCC macros
^^^^^^^^^^

The following example shows an implementation in GCC:

.. code-block:: c

    #define ___trace_on()      asm volatile ( ".word 0x39\n");
    #define ___trace_off()     asm volatile ( ".word 0x3d\n");
    #define ___reg_dump()      asm volatile ( ".word 0x37\n");
    #define ___cp0_reg_dump()  asm volatile ( ".word 0x0e\n");
    #define ___halt()          asm volatile ( ".word 0x28\n");
    #define ___val(i)          asm volatile ( ".word 0x35\n" :: "r" (i));



RISC-V instructions
-------------------


Environment Halt ``EHALT``
^^^^^^^^^^^^^^^^^^^^^^^^^^

Halt the machine as well as the simulation immediately.

**Opcode**: ``0x8C000073``


Dump general registers ``EDUMP``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The instruction dumps the content of all the general registers to the screen.

**Opcode**: ``0x8C100073``


Environment Set/Reset Trace ``ETRACES``/``ETRACER``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The instructions turn on/off instruction tracing.

**Opcode**: ``0x8C200073``/``0x8C300073``


Environment CSR Dump ``ECSRRD``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The instruction prints a specified CSR.
The CSR value is taken from the rd register from the I instruction format
(bits 7..11)

**Opcode**: ``0x8C400073`` (for register ``x0``)


GCC macros
^^^^^^^^^^

The following example shows an implementation in GCC:

.. code-block:: c

    #define ___ehalt()      asm volatile ( ".word 0x8C000073\n");
    #define ___edump()      asm volatile ( ".word 0x8C100073\n");
