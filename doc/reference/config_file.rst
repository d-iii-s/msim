Configuration file
==================

The configuration file of MSIM (usually called ``msim.conf``) contains any
system commands which would be otherwise typed in interactive mode in MSIM
prompt.
This allows for maximum flexibility and ease of use as you don't have to
learn to use the set of commands for the interactive mode and another set of
commands for the use in the configuration file.

A typical msim.conf might look like this:

.. code-block:: msim

    #
    # MSIM configuration script
    #

    add dr4kcpu cpu0
    add dr4kcpu cpu1

    add rwm mainmem 0x00000000
    mainmem generic 16M
    mainmem load "/dev/zero"

    add rom bootmem 0x1fc00000
    bootmem generic 4096k
    bootmem load "image.boot"

    add dprinter printer 0x10000000
    add dkeyboard keyboard 0x10000000 2
    add dorder order 0x10000004 5

Similarily to shell scripts empty lines are ignored.
Also any text beginning with the ``#`` character to the end of the line is
considered a comment and is ignored.
