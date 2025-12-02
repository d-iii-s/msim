DAP support
===========

DAP (debugger adapter protocol) is a protocol that enables standardized communication
between development tools (IDEs) and debuggers.

An experimental DAP support now arrived to MSIM, allowing DAP-enabled IDEs to connect to
the running MSIM for interactive debugging.

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

The first supported debugging feature are simple breakpoints.
You can set breakpoints in your IDE, and when MSIM hits them during execution,
it will enter interactive mode to allow you to e.g. inspect CPU registers.extension

This can be useful to pause execution at specific points in your program and
examine the state of your kernel.

A major advantage to adding a breakpoint directly in MSIM's interactive mode
using the ``break`` command is that you don't need to know the specific memory address
where to set the breakpoint, the extension does that for you.

It is conceptually similar to adding a breakpoint via the special `ebreak` instruction,
as described in :ref:`entering-the-debugger`.

When stopped at a breakpoint, you can resume the execution just as you would normally,
using the ``continue`` command.

Limitations
^^^^^^^^^^^

You can't remove the breakpoints when MSIM is running yet.
You need to restart MSIM and the debugging session after removing them in your IDE.

The IDE is currently not aware when and where MSIM stops,
this is a planned feature in next releases.

Some sources files might not be correctly mapped to the memory addresses, and
will not have the breakpoints hit.

Bug reports and feedback
------------------------

This extension is *very* experimental and a work in progress.

Please, report any bugs or feedback you have to the `issues in the MSIM repository <https://github.com/d-iii-s/msim/issues>`_.

Your bug reports and feedback are very much appreciated! :)
