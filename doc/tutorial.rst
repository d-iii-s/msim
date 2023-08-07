Tutorial
========

.. warning::
   This tutorial is still work-in-progress and covers only few topics.


With a small piece of practice, you fill find MSIM easy and comfortable.
Follow this tutorial to overcome the difficult beginner's live.
Status of the document


Installing
----------

Please, refer to our detailed
:doc:`installation instructions <reference/deployment>` page.


Devices
-------

Let's assume that the installation has been completed. After starting
``msim`` (from your terminal), you will see this:

.. code-block:: text

    ~$ msim
    <msim> Alert: Configuration file "msim.conf" not found, skipping
    <msim> Alert: MSIM 2.0.0
    <msim> Alert: Entering interactive mode, type `help' for help.
    [msim]

Not a lot - we are at the command line. A lot of magic things could be
done via this simple but powerful interface. The first command we will
learn is ``help``. Just type h-e-l-p and press Enter.

.. code-block:: msim

    [msim] help<Enter>
    [Command and arguments       ] [Description
    add <type> <name>              Add a new device into the system
    dumpmem <addr> <cnt>           Dump words from physical memory
    dumpins <cpu> <addr> <cnt>     Dump instructions from physical memory
    dumpdev                        Dump installed devices
    dumpphys                       Dump installed physical memory blocks
    break <addr> <cnt> <type>      Add a new physical memory breakpoint
    dumpbreak                      Dump physical memory breakpoints
    rembreak <addr>                Remove a physical memory breakpoint
    stat                           Print system statistics
    echo [<text>]                  Print user message
    continue                       Continue simulation
    step [<cnt>]                   Simulate one or a specified number of instructions
    set [<name> [= <val>]]         Set enviroment variable
    unset <name>                   Unset environment variable
    help [<cmd>]                   Display help
    quit                           Exit MSIM
    <device> <cmd>                 Commands of each added device
    [msim]

And we can see a list of available commands with a short description. To
make life easier, you don't have to write the whole command name. Only
first few letters which identifies the command uniquely are necessary.
Just try to type ``hel`` and see. Even ``he`` and ``h`` is enough. You
can also type the Tab key and the simulator will continue in typing
until there is no ambiguity in command names (the so-called tab
completion). When you type ``he`` and press Tab, the full name ``help``
will appear. If there are more choices available, msim will show you all
of them by typing Tab twice.

Try to type ``s`` and double-press Tab:

.. code-block:: msim

    [msim] s<Tab><Tab>
    set   stat  step
    [msim] s

There are three commands which starts with ``s``. Type the second letter
``t`` and the available commands are:

.. code-block:: msim

    [msim] st<Tab><Tab>
    stat  step
    [msim] st

By the way, the best practice of how to learn MSIM is to follow this
tutorial and try all the things yourself. Did you tried that? No, you
didn't. Do so now, I'll wait for you...

You didn't try it anyway, am I right? :-)

OK, let's move on. Complete the command ``step`` and run it:

.. code-block:: msim

    [msim] step
    [msim]

Nothing happens? Well, it does, but nothing visible. The simulator is
cycle-driven. It means that all the performed operations depends on a
virtual clock. However, when you run the simulator without any
parameters, the simulated machine is clear as the sky on the moon. You
can have a look on it by the ``dumpdev`` command which - as the help said -
dumps all installed devices:

.. code-block:: msim

    [msim] dumpdev
    [  name  ] [  type  ] [ parameters...
    No matching devices found.

It's clearly visible the list is empty. All the machine peripherals are
referred as devices. One such device may be a memory or a processor. By
the way, why not add a memory and a processor? All we need is to choose
a fine name :-).

The command ``add`` will enhance the machine on our request.
We can ask for help when using add:

.. code-block:: msim

    [msim] help add
    Add a new device into the system
    Syntax: add <type> <name>
    <type> Device type
    <name> Device name

All the commands have a similar help text. Yes, even the help command.
So we need a device type and a name.

Let's start with ``add``. We are now interested in processors, so the
``dr4kcpu`` device type is what we need (MIPS R4000 processor).
For other devices, please, consult :doc:`reference documentation <reference/devices>`.

The first ``d`` letter is just for clearity, to somehow make a space for usual
users identifies. Let's add a processor named ``c0`` and check it by listing devices:

.. code-block:: msim

    [msim] add dr4kcpu c0
    [msim] dumpdev
    [  name  ] [  type  ] [ parameters...
    c0         dr4kcpu    R4000


Here it is, we have a processor in our system. Pretty easy, isn't it?
Yeah, we can have more than one processors:

.. code-block:: msim

    [msim] add dr4kcpu George
    [msim] add dr4kcpu Fred
    [msim] dumpdev
    [  name  ] [  type  ] [ parameters...
    c0         dr4kcpu    R4000
    George     dr4kcpu    R4000
    Fred       dr4kcpu    R4000


Once again, to make us life easier, you can use up and down arrows on
you keyboard to scroll the history of your typed commands.


Processors feels better in a memory. Thus we add a read-write memory by
the add command:

.. code-block:: msim

    [msim] add rwm main 0x0
    [msim] main generic 256k


   [msim] add rwm main 0x0 256K
   [msim]

We have added a memory called ``main`` which starts at the address 0
(that's the start of the address space) and has 256 kilobytes. Addresses
are often typed in hexadecimal so we have used the ``0x`` prefix,
otherwise the number would be decimal. The number may be postfixed by a
modificator "k", "K" or "M". The number will be than multiplied by 1000,
1024 or 1048576 respectively. In our example, the size of the memory is
256k which is 262144 bytes.

And the list of devices says:

.. code-block:: msim

    [msim] dumpdev
    [  name  ] [  type  ] [ parameters...
    c0         dr4kcpu    R4000
    George     dr4kcpu    R4000
    Fred       dr4kcpu    R4000
    main       rwm        [Start    ] [Size      ] [Type]
    00000000000         256K mem


So we have a processor and a memory. Fine, what to do next? We should
somehow initialize the memory. We can do that by special device-oriented
commands. Commands presented so far have been system-wide or, in other
words, general. But each device has its own commands appliable on
specified device instance. To type such a command, simply start the
command line with the device name. In our case, the name of the memory
block is "main". And the classic command help may look like:

.. code-block:: msim

    [msim] main help
    [Command and arguments       ] [Description
    help                           Usage help
    info                           Configuration information
    generic <size>                 Generic memory type.
    fmap <File name>               Map the memory into the file.
    fill [<value>]                 Fill the memory with specified character
    load <File name>               Load the file into the memory
    save <File name>               Save the context of the memory into the file specified
    <device> <cmd>                 Commands of each added device


TO BE CONTINUED
