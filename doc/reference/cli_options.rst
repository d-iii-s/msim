Command line parameters
=======================

This section briefly describes the command line parameters of MSIM.
Most of the parameters can be combined.
For example a typical usage is to use the combination ``-i -t`` to enter both
interactive and trace mode on startup.


Version ``-V``, ``--version``
-----------------------------

Display MSIM version information and quit.


Configuration file ``-c``, ``--config``
---------------------------------------

Specify the configuration file name which overrides the default searching rules.

Syntax: ``-c|--config[=]filename``.

Example

.. code-block:: shell

    msim -c my.conf


Interactive mode ``-i``, ``--interactive``
------------------------------------------

The simulator enters the interactive mode immediately after the configuration file has been processed.


Trace mode ``-t``, ``--trace``
------------------------------

Enter the trace mode, but does not enable the interactive mode.

.. code-block:: shell

    $ msim -t
     1  BFC00000    lui   a0, 0x8000        # 0x8000=32768, a0: 0x0->0x80000000
     0  BFC00000    lui   a0, 0x8000        # 0x8000=32768, a0: 0x0->0x80000000
     1  BFC00004    ori   a0, a0, 0x1000    # 0x1000h=4096, a0: 0x80000000->0x80001000
     0  BFC00004    ori   a0, a0, 0x1000    # 0x1000h=4096, a0: 0x80000000->0x80001000
     1  BFC00008    sw    0, (a0)
     0  BFC00008    sw    0, (a0)


GDB mode ``-g``, ``--remote-gdb``
---------------------------------

Enter the GDB mode which allows a MIPS GDB to be connected to the running
MSIM for remote debugging.

The GDB mode is rather experimental.

Syntax: ``-g|--remote-gdb[=]port_number``


DAP mode ``-d``, ``--dap[port_number]``
---------------------------------------

Enter the DAP (debugger adapter protocol) mode which allows a DAP-enabled IDE to connect to the running
MSIM for interactive debugging.

The DAP mode is experimental and a work in progress.

The port argument is the port number on which MSIM listens for incoming DAP connections.
The current version of the debugging extension and adapter by default tries to connect to the default
port `10505`.

See the :doc:`detailed DAP documentation <./dap>` for more information.

Syntax: ``-d|--dap[port_number]`` (do not put space nor `=` between the option and the port number)


Help ``-h``, ``--help``
-----------------------

Print command line help and quit.

Allow non-determinism ``-n``, ``--non-deterministic``
-----------------------------------------------------

Enable non-deterministic behaviour.
MSIM must be run with this option when using a non-deterministic device.

Tip - When used repeatedly creating an alias might be useful (shell example):

.. code-block:: shell

    alias msim='msim -n'
