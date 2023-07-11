System commands
===============

This section describes the commands of MSIM.
These commands can be used either in the interactive mode or in
a configuration file.

.. contents:: Overview
   :local:
   :depth: 1




``add``: Add a new device into the system
-----------------------------------------

Add a new device into the system under a specified name.

.. code-block:: msim

    add device_type device_name [device_parameters...]

``device_type``
    Specifies a device type.
    The list of available device types is in the Devices section.
``device_name``
    An identifier of the newly created device.
    The name must not be equal to a system command and must be unique among
    already added devices.
``device_parameters``
    Initial device parameters.
    The actual parameters depends on the device type.


Example
"""""""

The first example adds a new printer ``p0`` to the system.
The printer register is located at physical address ``0x0001000``.

.. code-block:: msim

    [msim] add dprinter p0 0x1000


The second example adds a read-only memory m0 mapped at
physical address ``0x00010000``. The size of the memory is 16 KB.

.. code-block:: msim

    [msim] add rom m0 0x10000 16k




``quit``: Quit the simulation
-----------------------------

Quit the simulator immediately.




``dumpmem``: Dump words from unmapped memory
--------------------------------------------

Print a part of a memory from an unmapped address.

.. code-block:: msim

    dumpmem address count

``address``
   Starting address of the memory block.
``count``
   Number of words to print.


Example
"""""""

The following example prints 10 words from the physical address
``0x00001240``.

.. code-block:: msim

   [msim] dumpmem 0x1240 10
     00001240    265a45cd  2fefe111  11deadee  30957311
     00001250    7b218f9f  ffff2345  baba5555  deadbeaf
     00001260    29dc9aff  1298aa23
   [msim]




``dumpins``: Dump instructions from unmapped memory
---------------------------------------------------

Print disassembled instructions from the specified unmapped address.

.. code-block:: msim

    dumpins cpu address count

``cpu``
   CPU architecture instructions of which will be dumped. (``r4k`` or
   ``rv``)
``address``
   Starting address of the memory block.
``count``
   Number of instructions to print.


Example
"""""""

The following example prints 10 MIPS R4000 instructions from the address
0x000012a8.

.. code-block:: msim

   [msim] dumpins r4k 0x12a8 10
       000012A8    ori   s0, s0, 0x5427    # 0x5427h=21543
       000012AC    sw    s2, 0x18(sp)      # 0x18=24
       000012B0    lui   s2, 0x805a        # 0x805a=32858
       000012B4    ori   s2, s2, 0xce55
       000012B8    sw    s1, 0x14(sp)      # 0x14=20
       000012BC    lui   s1, 0x8100        # 0x8100=33024
       000012C0    sw    ra, 0x1c(sp)      # 0x1c=28
       000012C4    lw    a0, 0x1640(gp)    # 0x1640=5696
       000012C8    jal   +0x4009d4         # 0x4009d4=4196820
       000012CC    nop
   [msim]




``dumpdev``: Dump all installed devices
---------------------------------------

Print a list of devices with parameters configured in the environment.

Example
"""""""

The following example prints all devices in the system.

.. code-block:: msim

   [msim] dumpdev
   [  name  ] [  type  ] [ parameters...
   printer    dprinter   address:0x01000000
   startmem   rom        start:0x1fc00000 size:1k type:mem
   xxx        rwm        start:0x00400000 size:256k type:mem
   mainmem    rwm        start:0x00000000 size:256k type:mem
   mips1      dcpu       type:R4000.32
   [msim]




``dumpphys``: Dump all installed memory blocks
----------------------------------------------

Print all configured memory blocks.


Example
"""""""

The following example prints all installed memory blocks.

.. code-block:: msim

   [msim] dumpphys
   [  name  ] [  type  ] [ parameters...
   startmem   rom        start:0x1fc00000 size:1k type:mem
   xxx        rwm        start:0x00400000 size:256k type:mem
   mainmem    rwm        start:0x00000000 size:256k type:mem
   [msim]




``break``: Add memory breakpoint
--------------------------------

Add memory access breakpoint. If a read or write access on the the
physical address of the breakpoint occurs, the simulator is immediately
switched to interactive mode.

.. code-block:: msim

    break address type

``address``
   Address of the breakpoint.
``count``
   Consider read accesses (``r``), write accesses (``w``) or both (``rw``).




``dumpbreak``: Dump memory breakpoints
--------------------------------------

Print all configured memory access breakpoints.




``rembreak``: Remove memory breakpoint
--------------------------------------

Remove previously configured memory breakpoint.

.. code-block:: msim

    rembreak address

``address``
   Address of the previously configured breakpoint.




``stat``: Dump available statistic information
----------------------------------------------

Print statistics of installed devices.


Example
"""""""

The following example prints the statistics of all the installed
devices.

.. code-block:: msim

   [msim] stat
   [  name  ] [  type  ] [ statistics...
   printer    dprinter   count:42248
   startmem   rom        no statistics
   xxx        rwm        no statistics
   mainmem    rwm        no statistics
   mips1      dcpu       cycles total:1373061 in kernel:1373061 in user:0
                         in stdby:0 tlb refill:0 invalid: 0 modified:0
                         interrupts 0:0 1:0 2:0 3:0 4:0 5:0 6:0 7:0
   [msim]




``echo``: Print user message
----------------------------

Print user message. The ``echo`` command is usually used for debugging
purposes, mainly in the configuration file.

.. code-block:: msim

    echo message

``message``
   String which will be printed.

Example
"""""""

The following example prints the message "The point A has been reached".

.. code-block:: msim

   [msim] echo "The point A has been reached"
   The point A has been reached
   [msim]




``continue``: Continue simulation
---------------------------------

Continue in the simulation. The interactive mode is leaved.


Example
"""""""

In the following example, the execution is terminated by pressing Ctrl+C
and later restarted by the ``continue`` command. The trace mode in still
on:

.. code-block:: msim

   ...

    0  80400ED4    subu  v0, v0, a1        # v0: 0x24640->0x245b7
    0  80400ED8    sll   v1, v0, 0x02      # v1: 0xb71d6a00->0x916dc
   <Ctrl+C>
   [msim] continue
    0  80400EDC    addu  v0, v0, v1        # v0: 0x245b7->0xb5c93
    0  80400EE0    sll   v0, v0, 0x04      # v0: 0xb5c93->0xb5c930
    0  80400EE4    addu  v0, v0, a1        # v0: 0xb5c930->0xb5c9b9

   ...




``step``: Simulate one or a specified number of instructions
------------------------------------------------------------

Execute one or a specified number of cycles. The ``step`` command is
default (it is executed if a blank command line entered).

.. code-block:: msim

    step [count]

``count``
   Optional number of cycles to execute.


Example
"""""""

In the following example, the user executes 3 steps on a 2-processor
machine:

.. code-block:: msim

   [msim] s 3
    1  80400ACC    mfc0  v0, status        # v0: 0x0->0x8001
    0  80400EC4    srl   a1, a1, 0x18      # 0x18=24, a1: 0x6538b04f->0x65
    1  80400AD0    addiu v1, 0, 0xfffe     # v1: 0x0->0xfffffffe
    0  80400EC8    sll   v0, a1, 0x04      # v0: 0x219dad69->0x650
    1  80400AD4    and   v0, v0, v1        # v0: 0x8001->0x8000
    0  80400ECC    addu  v0, v0, a1        # v0: 0x650->0x6b5
   [msim]




``set``: Set environment variable
---------------------------------

Set an internal variable on or to a specified value or print a list of
all variables.

.. code-block:: msim

    set [variable [= value]]

``variable``
   Name of the internal variable name to be set (if not specified, list
   of all variables is printed).
``value``
   Value to be assigned to the variable (if not specified, logical
   ``true`` is assumed).

Refer to the *Internal variables section* for a
list and a description of available variables. If ``value`` is not
specified, the variable is set to the logical ``true``. Note that not
all variables can hold a logical values.

For boolean variables, there are several synonyms for ``true``:
``true``, ``on``, ``yes``. Similarly the synonyms for ``false`` are:
``false``, ``off``, ``no`` and their prefixes.


Example
"""""""

In the first example, we list all the variables:

.. code-block:: msim

   [msim] set
   Group                  Variable   Value
   ---------------------- ---------- ----------
   Disassembling features
                          iaddr      on
                          iopc       off
                          icmt       on
                          iregch     on
                          r4k_ireg   2
                          rv_ireg    1
   Debugging features
                          trace      off
   [msim]

In the second example, we switch to trace mode:

.. code-block:: msim

   [msim] s
   [msim] set trace
   [msim] s
       0  80401378    addiu a0, a0, 0x40      # 0x40=64, a0: 0xb7b8->0xf8
   [msim]

And in the third example, we set the type of register names:

.. code-block:: msim

   [msim] set r4k_ireg = 2
   [msim] id 0x00401330 1
       00401330    srl   v0, v0, 0x01
   [msim] set r4k_ireg = 0
   [msim] id 0x00401330 1
       00401330    srl   r2, r2, 0x01
   [msim]




``unset``: Unset environment variable
-------------------------------------

Unset a logical internal variable.

.. code-block:: msim

    unset variable

``variable``
   Internal variable name to be unset.

Specified ``variable`` must hold a logical value.
Refer to the *Internal variables section* for the list and description
of available variables.


Example
"""""""

In the following example, the trace mode is switched off:

.. code-block:: msim

   [msim] s
       0  80401334    addu  r5, r5, r2        # r5: 0x6512c4be->0x9754a5b8
   [msim] unset trace
   [msim] s
   [msim]




``help``: Display a help text
-----------------------------

Show the help for the specified system command or print the list of
available system commands.

.. code-block:: msim

    help [command]


``command``
   Command name (if omited, the list of available system commands is
   printed).

