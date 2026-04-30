DAP support
===========

DAP (debugger adapter protocol) is a protocol that enables standardized communication
between development tools (IDEs) and debuggers.

DAP support is integrated to MSIM, allowing DAP-enabled IDEs to connect to
the running MSIM for interactive debugging via a debugging adapter.

Currently the only supported and tested IDE is Visual Studio Code with the provided
`msim-debugger extension <https://github.com/0xVector/msim-debugger>`_.

Usage
-----

To enable the DAP mode, start MSIM with the ``-d`` or ``--dap`` command line option,
specifying the port number on which MSIM should listen for incoming DAP connections.

The port number to use depends on your debugging extension configuration in the IDE
(see your `.vscode/launch.json` file), but the default port used by both MSIM and
the `msim-debugger` extension is `10505`.
Do note that there must be no space nor `=` between the option and the port number
if you choose to specify your own port number.

Features
--------

Features that are currently supported are specified here. More features will be added
in future releases.

Breakpoints
~~~~~~~~~~~

Code breakpoints are fully supported.
You can set breakpoints in your IDE, and when MSIM hits them during execution,
it will pause and wait for further commands from the IDE.

This can be useful to pause execution at specific points in your program and
examine the state of your kernel.

A major advantage to adding a breakpoint directly in MSIM's interactive mode
using the ``break`` command is that you don't need to know the specific memory address
where to set the breakpoint, the extension does that for you.

It is conceptually similar to adding a breakpoint via the special `ebreak` instruction,
as described in :ref:`entering-the-debugger`.

When stopped at a breakpoint, you can resume the execution just as you would normally,
using the resume command in your IDE.

Execution control
~~~~~~~~~~~~~~~~~

MSIM can be paused and resumed from the IDE, allowing you to control the execution of your kernel.

Register Inspection
~~~~~~~~~~~~~~~~~~~

When MSIM is paused, you can inspect the values of the registers in your kernel from the IDE.
Both general-purpose and control/status registers are supported. The registers are displayed
in the IDE's variables view, and their values can be modified.

Memory Inspection
~~~~~~~~~~~~~~~~~

When MSIM is paused, you can inspect the memory of your kernel from the IDE.
The memory view in the IDE allows you to view and modify the contents of the memory at specific addresses.
This is done by clicking the memory view next to a register with pointer-like semantics.

CPU View
~~~~~~~~

The CPUs in MSIM are displayed in the IDE, presented as threads. You can see the highlighted CPU when MSIM
is paused, denoting the CPU causing the stopping.

Stepping
~~~~~~~~

Stepping is supported, allowing you to step through your kernel's execution one instruction at a time.
The step-in command steps by one instruction, while the step-over command attempts to step to the next statement.
The step-out command is not supported yet, defaulting to step-in behavior.

Limitations
^^^^^^^^^^^

Some sources files might not be correctly mapped to the memory addresses, and
will not have the breakpoints hit. This happens especially when entering a userspace application,
as that is usually built as a separate ELF file, which the extension has no access to.

The step-out command is not supported yet.

Bug reports and feedback
------------------------

Please, report any bugs or feedback you have to the `issues in the MSIM repository <https://github.com/0xVector/msim/issues>`_.

Your bug reports and feedback are very much appreciated! :)
