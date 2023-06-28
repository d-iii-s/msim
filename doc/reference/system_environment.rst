System environment
==================

When no configuration file ``msim.conf`` exists in the current directory
(and no other configuration file is specified via the ``-c`` command line
parameter),
the simulator represents and "empty computer" after startup.
In this case MSIM will go into interactive mode.

By default, there are no hardware devices configured, there is no physical
memory, there is not even a CPU for running code. Just the MSIM command
line prompt can be seen:

.. code-block:: msim

    [msim]

You have to use the system commands to configure the environment.
Here is a reasonable minimal set of commands which are required to execute
some MIPS code (each command is followed by ``Enter`` keypress):

.. code-block:: msim

    [msim] add dcpu cpu0
    [msim] add rwm memory 0x00000000
    [msim] memory generic 16M
    [msim] add rom firmware 0x1fc00000
    [msim] firmware generic 4096k
    [msim] firmware load "firmware.img"
    [msim]

This will configure the environment with the following devices:

- a single CPU (called ``cpu0``)

- a piece of read/write memory starting at the physical address 0x00000000
  and with the size of 16 MB (called ``memory``)

- a piece of read-only memory starting at the physical address 0x1fc00000
  and with the size of 4096 KB (called ``firmware``)

  - this read-only memory is populated with the contents of the file
    ``firmware.img`` (this file must exist it the current directory of the
    host computer)

After this basic configuration the simulator is ready to execute the code
(sequence of MIPS instructions) loaded from the file ``firmware.img`` and
stored in the firmware read-only memory.
The single CPU configured in the system (``cpu0``) will start the execution
at the virtual address 0xbfc00000, which exactly corresponds to the physical
address 0x1fc00000 (i.e. the start of firmware memory).

The executed code will be able to use the read/write memory memory located at
the physical address 0x00000000. The execution environment is modeled strictly
according to the specification of MIPS R4000 CPU (please refer to MIPS
R4000 Microprocessor User's Manual).

Please refer also to further sections of this text for details about the
commands used in our brief example.
