Mini-kernel tutorial
====================

This page contains several small exercises that should help you
with your first steps within MIPS or RISC-V kernel running in MSIM.

We will provide you with both instructions how to run MSIM as well as
a source code of small kernel that you can play with.

.. contents:: Here is an overview of the exercises.
   :local:


Toolchain setup
---------------

We expect that you have a cross-compiler toolchain already installed so
that you can try the examples yourself.

Please, refer to our :doc:`instructions <toolchain>` if you need help with
installing a toolchain.

Once you have your toolchain ready, we can dive into kernel code for real :-).


First compilation
-----------------

If you have never compiled an operating system kernel (or if you
are new to C, GCC, or make), you may wish to start with compiling
a smaller kernel first.

Please, clone the `MSIM repository <https://github.com/d-iii-s/msim>`__.

This tutorial contains examples for both MIPS and RISC-V mini-kernels
(both in 32bit variants). Because the architectures are rather similar
(after all, RISC-V designers admit they were inspired by MIPS) the text
will only contain following markers if the behavior (or source code)
differs significantly between these two architectures.

.. archbox:: MIPS

    MIPS examples are in the ``contrib/kernel-tutorial-mips32/`` subdirectory.

.. archbox:: RISC-V

    RISC-V examples are in the ``contrib/kernel-tutorial-riscv32/`` subdirectory.

We will start inside the ``first`` subdirectory, please, choose either MIPS
or RISC-V architecture for this exercise
(of course, intrepid developers can choose to inspect and experiment with
both at the same time).

Before we discuss the contents of the directory, we will build the kernel.
All the examples use `make <https://www.gnu.org/software/make/>`__
as the build tool so simply type ``make`` to build it.

The ``make`` command launches the make tool, which reads dependency rules
from a file named ``Makefile`` and uses them to figure out how to
compile C sources into a binary executable.

In this case, make should run a sequence of commands to build the
``loader.bin`` executable from the ``loader.S`` source, and the
``kernel.bin`` executable from the ``head.S`` and ``main.c``
sources.

``make`` will produce the following output (there might be some differences
in the paths but otherwise the output should look the same on your machine).

.. tabs:: arch

   .. code-tab:: bash
      :caption: MIPS

      make -C kernel
      make[1]: Entering directory './kernel'
      /usr/bin/mipsel-unknown-linux-gnu-gcc -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 -mno-abicalls -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=c11 -I. -D__ASM__ -c -o boot/loader.o boot/loader.S
      /usr/bin/mipsel-unknown-linux-gnu-ld -G 0 -static -g -T kernel.lds -Map loader.map -o loader.raw boot/loader.o
      /usr/bin/mipsel-unknown-linux-gnu-objcopy -O binary loader.raw loader.bin
      /usr/bin/mipsel-unknown-linux-gnu-objdump -d loader.raw > loader.disasm
      /usr/bin/mipsel-unknown-linux-gnu-gcc -O2 -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 -mno-abicalls -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=c11  -c -o src/main.o src/main.c
      /usr/bin/mipsel-unknown-linux-gnu-gcc -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 -mno-abicalls -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=c11 -I. -D__ASM__ -c -o src/head.o src/head.S
      /usr/bin/mipsel-unknown-linux-gnu-ld -G 0 -static -g -T kernel.lds -Map kernel.map -o kernel.raw src/main.o src/head.o
      /usr/bin/mipsel-unknown-linux-gnu-objcopy -O binary kernel.raw kernel.bin
      /usr/bin/mipsel-unknown-linux-gnu-objdump -d kernel.raw > kernel.disasm
      make[1]: Leaving directory './kernel'

   .. code-tab:: bash
      :caption: RISC-V

      make -C kernel
      make[1]: Entering directory './kernel'
      /usr/bin/riscv32-unknown-elf-gcc -msmall-data-limit=0 -mstrict-align -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -mno-riscv-attribute -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=c11 -I. -D__ASM__ -march=rv32g -c -o boot/loader.o boot/loader.S
      /usr/bin/riscv32-unknown-elf-ld -G 0 -static -g -T loader.lds -Map loader.map -o loader.raw boot/loader.o
      /usr/bin/riscv32-unknown-elf-ld: warning: loader.raw has a LOAD segment with RWX permissions
      /usr/bin/riscv32-unknown-elf-objcopy -O binary loader.raw loader.bin
      /usr/bin/riscv32-unknown-elf-objdump -d loader.raw > loader.disasm
      /usr/bin/riscv32-unknown-elf-gcc -O2 -msmall-data-limit=0 -mstrict-align -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -mno-riscv-attribute -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=c11 -march=rv32g  -c -o src/main.o src/main.c
      /usr/bin/riscv32-unknown-elf-gcc -msmall-data-limit=0 -mstrict-align -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -mno-riscv-attribute -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=c11 -I. -D__ASM__ -march=rv32g -c -o src/head.o src/head.S
      /usr/bin/riscv32-unknown-elf-ld -G 0 -static -g -T kernel.lds -Map kernel.map -o kernel.raw src/main.o src/head.o
      /usr/bin/riscv32-unknown-elf-ld: warning: kernel.raw has a LOAD segment with RWX permissions
      /usr/bin/riscv32-unknown-elf-objcopy -O binary kernel.raw kernel.bin
      /usr/bin/riscv32-unknown-elf-objdump -d kernel.raw > kernel.disasm
      make[1]: Leaving directory './kernel'

.. extras:: using ``make``

    The advantage of using make as opposed to a shell script is in
    that make will only rebuild files (along dependency chains) that
    have changed since the last compilation, which saves build time,
    especially on larger projects (you can try that by running
    ``make`` again now).

    In this example, the rules in the top-level ``Makefile`` just tell
    make to run ``make`` again, but this time using the ``Makefile``
    in the ``kernel`` subdirectory; more details of the
    compilation will come later on.

Note that there is ``msim.conf`` in our directory. It contains
directives for the MSIM simulator, configuring it so as to provide
a simple computer equipped with one processor, two
blocks of memory, and a console-like device for textual output (we
will dissect the configuration in the next exercise).

To run the compiled kernel code, run ``msim`` without any
arguments. MSIM will load the binary images (``loader.bin`` and
``kernel.bin``) into the two memory blocks and reset the simulated
CPU so that it starts executing code at factory-defined addresses.
You should see the following output:

.. tabs:: arch

   .. code-tab:: msim
      :caption: MIPS

      Hello, World.
      <msim> Alert: XHLT: Machine halt

      Cycles: 41

   .. code-tab:: msim
      :caption: RISC-V

      Hello, World.
      <msim> Alert: EHALT: Machine halt

      Cycles: 42

The “Hello, World.” message was printed from C code compiled into
machine code running on the processor of your choosing. Getting the
target processor to execute your (compiled) C code is usually one
of the major technical obstacles when starting OS development from
scratch, which is why we have taken care of this step for now.

The last line (as well as the line prefixed with ``<msim>``) is
the output of the simulator, telling us how many virtual cycles
has the CPU executed. This is the exact amount of executed instructions.
We can safely ignore those lines for now.

.. important::

   If the compilation failed for you, or if the execution printed
   something completely different, please, feel free to contact us:
   please, `open an issue here <https://github.com/d-iii-s/msim/issues>`__
   and describe what have you tried, what failed and please do not
   forget to describe your environment.

   If you are a NSWI200 student, please, prefer the standard means of
   communicating with your teachers instead of the GitHub issues. Thank you.


Configuring the virtual machine
-------------------------------

We will now take a closer look at the ``msim.conf`` file, which
contains the configuration of the simulated computer that runs
your kernel.

Using a simulated computer instead of a real one
makes it much easier to develop a small kernel (for one thing,
installation does not require sacrificing your own computer, also,
the simulation is completely deterministic and therefore bugs that
appear once keep appearing until you fix them). However, rest
assured the simulated environment is close enough to the real
thing.

Reading ``msim.conf`` from top to bottom and ignoring the comment
lines starting with the ``#`` character, the first configuration
line tells MSIM to add one processor and name it ``cpu0``

.. tabs:: arch

   .. code-tab:: msim
      :caption: MIPS

      add dr4kcpu cpu0

   .. code-tab:: msim
      :caption: RISC-V

      add drvcpu cpu0

.. archbox:: MIPS

   The MIPS R4000 processor device is named ``dr4kcpu``.

.. archbox:: RISC-V

   The RISC-V RV32IMA processor device is named ``drvcpu``.

The next two groups of directives add two blocks of physical
memory, one for the bootloader and one for the main memory, both
initialized from files on disk.

The main memory block (called ``mainmem``) is a read-write memory
with a size of ``1 MiB``. The memory block is initialized with
the contents of the ``kernel/kernel.bin`` file before the simulated
computer starts running:

.. tabs:: arch

   .. code-tab:: msim
      :caption: MIPS

      add rwm mainmem 0
      mainmem generic 1M
      mainmem load "kernel/kernel.bin"

   .. code-tab:: msim
      :caption: RISC-V

      add rwm mainmem 0x80000000
      mainmem generic 1M
      mainmem load "kernel/kernel.bin"

.. archbox:: MIPS

   The ``mainmem`` memory segment starts at physical address ``0``.
   The processor then maps it to a virtual address ``0x80000000``
   (so printing a pointer address in your code will print addresses with
   the highest bit set).

.. archbox:: RISC-V

   The ``mainmem`` memory segment starts at physical address ``0x80000000``.
   The processor uses identity mapping when booting, hence we do not need to
   explicitly distinguish virtual and physical addresses (at least, for now¨).

The bootloader memory block (called ``loadermem``) is a read-only
memory initialized with the contents of the ``kernel/loader.bin`` file:

.. tabs:: arch

   .. code-tab:: msim
      :caption: MIPS

      add rom loadermem 0x1FC00000
      loadermem generic 4K
      loadermem load "kernel/loader.bin"

   .. code-tab:: msim
      :caption: RISC-V

      add rom loadermem 0xF0000000
      loadermem generic 8K
      loadermem load "kernel/loader.bin"

.. archbox:: MIPS

   The ``loadermem`` memory segment starts at physical address ``0x1FC00000`` and has a size of ``4 KiB``.

.. archbox:: RISC-V

   The ``loadermem`` memory segment starts at physical address ``0xF0000000`` and has a size of ``8 KiB``.

Finally, we add a simple output device (called ``printer``),
which will allow the code running in the simulator to display
text on the host computer console.
This is similar to serial console found on real
hardware, except the printer device is much simpler:

.. tabs:: arch

   .. code-tab:: msim
      :caption: MIPS

      add dprinter printer 0x10000000

   .. code-tab:: msim
      :caption: RISC-V

      add dprinter printer 0x90000000

.. archbox:: MIPS

   This device resides at physical address ``0x10000000``.

.. archbox:: RISC-V

   This device resides at physical address ``0x90000000``.

This is actually enough for a simple machine and more than enough
for our purposes :-).


Disassembling the kernel
------------------------

With the simulator configured to provide us with a simple
computer, it is now time to look at the files in the
``kernel`` directory. Again, there is a ``Makefile`` which
controls the compilation, and a linker script which controls the
layout of the binary image produced by the linker.

.. extras:: linker scripts

   We will not dissect the linker script further, because we will
   not need to modify it in this tutorial.

   As a matter of fact, linker scripts are rarely modified and in normal
   circumstances come with your linker. For our purposes, where we have
   a non-standard kernel and a simplified emulator, we have our own ones.

The ``boot`` subdirectory contains ``loader.S``, an assembly
source file which contains the computer bootloader code. On a real
computer, the bootloader is (ultimately) responsible for loading
the operating system into memory. In our case, the MSIM simulator
does this for us (see the directives telling MSIM to load
``kernel/kernel.bin`` into ``mainmem`` in ``msim.conf``), so we
just need a few instructions to make the processor jump into the
kernel code after reset.

The loader code needs to be present at a specific address (it is
hard-wired into the CPU, see ``msim.conf``) which the CPU starts
executing instructions from after a power up/reset. Other than
that, the loader code does not really do anything – it just jumps
to another fixed address, where our main code will reside.

.. archbox:: MIPS

   The loader jumps to address ``0x80000400``.

   The reason why we keep the rest of the kernel code separate from
   the loader is quite simple – the entry point of the loader is
   quite far from the entry points of the exception handlers, which
   are also hardwired, and which the kernel must implement. We simply
   want to keep the rest of the kernel code in one piece, and that
   means next to the exception handlers.

.. archbox:: RISC-V

   The loader jumps to address ``0x80001000``.

The ``loader.S`` file is compiled and linked into ``loader.bin``.
This file contains only machine instructions (no symbol
information, no debugging information, no relocation information):
it is code in its rawest form, a form that the CPU actually sees.

Look into ``loader.bin`` and ``loader.disasm``. The second one is
a disassembly of the binary format back to assembler.

::

   cat loader.disasm
   hexdump -C loader.bin

Since ``loader.bin`` and ``loader.disasm`` are produced from
``loader.S``, they should contain the same instructions as in the
original ``loader.S``. Do take a look.

.. quiz::

   A question for you: why are the instructions in ``loader.disasm``
   different from ``loader.S``?


   .. collapse:: Hint

      Think about the limited instruction repertoire of the CPU.

   .. collapse:: Solution MIPS

      The difference in code concerns the loading of the
      32-bit constant (jump target address). The CPU does
      not have an instruction that can load an entire 32-bit
      constant in one go (because the instruction itself
      must fit into 32 bits), hence two instructions are
      used. The assembly code uses a shorthand notation so
      that the programmer does not have to perform this
      trivial conversion.

   .. collapse:: Solution RISC-V

      The difference in code concerns the loading of the 32-bit constant (jump
      target address). The CPU does not have an instruction that can load an
      entire 32-bit constant in one go (because the instruction itself must
      fit into 32 bits), hence two instructions would need to be used
      generally. (For example ``li t0, 0x0x80000001`` would be transformed
      into ``lui t0, 0x80000`` and ``addi t0, t0, 1`` - try it yourself!) Our
      code manages with only one, because the lowest 12 bits (3 hex digits) of
      our target address are all 0. The ``lui t0, 0x80001`` instruction loads
      the constant ``0x80001`` to the highest 20 bits of ``t0``, meaning it
      sets it to ``0x80001000``, which is exactly our desired address. The
      assembly code uses a shorthand notation so that the programmer does not
      have to perform this trivial conversion.


From boot to C code
-------------------

We will now look into the ``src`` directory, where the foundations
of our kernel reside.

The ``head.S`` file contains a lot of assembly code, but do not be
afraid ;-).

.. archbox:: MIPS

   Find the line containing ``start:`` (around line 120). Above this,
   we can see a special directive ``.org 0x400`` that says that the
   following code will be placed at address 0x400 bytes away from the
   start of the code segment. The linker specifies that the code
   segment starts at ``0x80000000``, together this yields ``0x80000400`` -
   exactly the address our boot loader jumps to! Hence, after the
   boot loader is done, the execution will continue here.

   We start by setting up few registers (such as the stack pointer)
   and execute ``jal kernel_main``. This will pass control from the
   assembly code to the ``kernel_main`` function, which is a standard
   C function that you can see if you open ``src/main.c``.

.. archbox:: RISC-V

   Find the line containing ``start:`` (around line 90). Above this, we can
   see a special directive ``.org 0x1000`` that says that the following
   code will be placed at address 0x1000 bytes away from the start of the
   code segment. The linker specifies that the code segment starts at
   0x80000000, together this yields ``0x80001000`` - exactly the address our
   boot loader jumps to! Hence, after the boot loader is done, the
   execution will continue here.

   We start by setting up few registers (such as the stack pointer and the
   ``mepc`` CSR) and execute ``mret``. This will pass control from the
   assembly code to the ``kernel_main`` function, which is a standard C
   function that you can see if you open ``src/main.c``.


These few lines of assembler (``loader.S`` and ``head.S``)
constitute the only assembly code needed to boot the processor and
get into C.

.. extras:: assembler and booting

   One cannot boot a CPU without at least a bit of assembler that jumps
   into a C code. But the assembly code is usually straightforward and
   only sets-up basic registers and stack.

   Feel free to return to this code later, understanding it completely is
   not required to continue with the tutorial. As long as you understand
   that we need special instructions to jump to a C code, you will be fine.


``kernel_main`` is where the fun starts
---------------------------------------

The last file we have not commented much on is ``src/main.c``.

It contains the ``kernel_main()`` function, which is called
shortly after boot. This is the function, where the kernel
would initialize itself or launch the first userspace process
(e.g. ``init`` on Linux).

Right now it contains only a very short greeting.

Printing from the simulator is trivial: since we told MSIM that there
should be a console printer device available at an particular address.
MSIM monitors this address and any write to it causes the written
character to appear at the console.

.. archbox:: MIPS

   A question for you: if you look up the console printer device
   address in the source code, you will see it is ``0x90000000``, but
   ``msim.conf`` says ``0x10000000``. Why?

   .. collapse:: Hint

      Think about virtual and physical addresses.

   .. collapse:: Solution

      The code uses virtual addresses, but the simulator
      configuration uses physical addresses (exactly what a
      real hardware would see). In the kernel segment,
      virtual addresses are mapped to physical addresses
      simply by masking the highest bit - virtual address
      ``0x80000000`` therefore corresponds to physical address
      0, and so on. The mapping is intentionally simple
      because the kernel must run even before more complex
      mapping structures, such as page tables, can be set
      up.

An important note: you probably noticed that we print
the characters one by one instead of using ``printf``
or ``puts``. That is because we are in our own kernel
and we do not have any of these functions. As a
matter of fact, **we will have only functions
that we implement ourselfs**.

Thus, there is no ``printf``, no ``malloc`` and definitely no
``fopen`` (unless you implement them yourself).


The first modification of the kernel
------------------------------------

Modify the kernel so that it prints the greeting with an
exclamation mark instead of a plain period. After all, we can be
proud of it ;-).

Before running ``msim`` again do not forget to recompile with
``make``.

.. collapse:: Solution

   Just replace ``'.'`` with ``'!'`` in ``main.c`` :-).

   Note that ``make`` should recompile only ``main.c`` into ``main.o``
   and re-link the ``kernel.*`` files. Files related to
   the bootloader should remain without change.


Tracing the execution
---------------------

Let’s see which instructions were actually executed by MSIM. This
may come in handy in later debugging tasks.

We will run ``msim -t``. This turns on a trace mode where MSIM prints
every instruction as it is executed. (Unfortunately, there is just
one console, so the MSIM output is interleaved with your OS
output.)

.. quiz::

   Compare the trace with your ``*.disasm`` files. What is the
   difference?

   .. collapse:: Solution

      The answer is obvious: ``*.disasm`` contains the code
      in its static form while the trace represents the true
      execution - jumps are taken, loop bodies are executed
      repeatedly etc.


Stepping through the execution
------------------------------

To run the kernel instruction by instruction interactively, launch
MSIM with ``msim -i``. This time, MSIM will wait for further
commands, as indicated by the ``[msim]`` prompt.

Simply typing ``continue`` will resume standard execution, which
will run our OS and eventually terminate MSIM.

Run MSIM again but instead of typing ``continue``, we will just hit Enter.

An empty command in MSIM is equivalent to typing ``step`` and
executes a single instruction. We should see how the greeting
starts to appear next to the prompt as we continue pressing
Enter.

We can also do ``step 10`` to execute ten instructions at once.

.. _entering-the-debugger:

Entering the debugger
---------------------

Stepping through our kernel from the very first instruction is
not so useful for debugging when the code we are interested in is
executed long after boot. In that case, we can also enter the
interactive mode programmatically, by asking for it from inside
our (kernel) code.

That is something that is super-easy when running in a simulator such
as MSIM but somewhat more difficult on real hardware.
That is why simulators are so useful :-).

To enter the interactive mode, we will use a special assembly language
instruction, which the real CPU does not recognize but MSIM does.

We will insert the following fragment at a location (in the C code) where
we want to interrupt the execution.

.. tabs:: arch

   .. code-tab:: c
      :caption: MIPS

      __asm__ volatile(".word 0x29\n");

   .. code-tab:: c
      :caption: RISC-V

      __asm__ volatile("ebreak\n");


Let us try it: insert the break after printing ``Hello``. If we execute
``msim``, it will print ``Hello`` and enter interactive mode. We
can again step through the execution or ``continue``.


Inspecting the registers
------------------------

Let us start MSIM in interactive mode again and type ``set trace`` as the
first command.

Then we will hit Enter several times. We executed several instructions
and MSIM is printing what instructions are executed.

We can also inspect all registers at once. We will use the ``cpu0 rd``
command for a **r**\ egister **d**\ ump of the `cpu0`` processor
(that is the only processor that we added to our computer in
MSIM).

This is an extremely useful command as it allows us to inspect
what is the current state of the processor and what code it
executes.

.. quiz::

   Which register would tell you what code is executed?

   .. collapse:: Solution

      The ``pc`` register is the program counter telling the
      (virtual) address where the CPU decodes the next
      instruction.


Matching instructions back to source code
-----------------------------------------

Start MSIM again in the interactive mode and step until it starts
printing the greeting. Look at the register dump.

You will see something like this (note that we have dropped the
64bit extension to make the dump a bit shorter):

.. tabs:: arch

   .. code-tab:: msim
      :caption: MIPS

       0 00000000   at 00000000   v0 90000000   v1 00000000   a0 00000000
      a1 00000048   a2 00000000   a3 00000000   t0 00000000   t1 00000000
      t2 00000000   t3 00000000   t4 00000000   t5 00000000   t6 00000000
      t7 00000000   s0 00000000   s1 00000000   s2 00000000   s3 00000000
      s4 00000000   s5 00000000   s6 00000000   s7 00000000   t8 00000000
      t9 00000000   k0 0000FF01   k1 00000000   gp 80000000   sp 80000400
      fp 00000000   ra 80000420   pc 8000043C   lo 00000000   hi 00000000

   .. code-tab:: msim
      :caption: RISC-V

         zero:      0    ra: 80001060    sp: 80001000    gp:        0
         tp:        0    t0:      800    t1:        0    t2:        0
      s0/fp:        0    s1:        0    a0:        0    a1:        0
         a2:        0    a3:        0    a4:       48    a5: 90000000
         a6:        0    a7:        0    s2:        0    s3:        0
         s4:        0    s5:        0    s6:        0    s7:        0
         s8:        0    s9:        0   s10:        0   s11:        0
         t3:        0    t4:        0    t5:        0    t6:        0
         pc: 8000106c                               Privilege mode: S

.. archbox:: MIPS

   In our dump, ``pc`` contains the ``8000043C``.
   If we open ``kernel.disasm`` and find this address there, we will see
   it is few lines below ``80000430 <kernel_main>`` which indicates that
   it is an instruction inside ``kernel_main()``.

.. archbox:: RISC-V

   In our dump, ``pc`` contains ``8000106c``.
   If we open ``kernel.disasm``  and find this address there, we will see
   it is few lines below ``80001060 <kernel_main>`` which indicates that
   it is an instruction inside ``kernel_main()``.

This is extremely important information because it allows us to
decide in which function our OS will be when it is interrupted
etc.

We can interrupt code in MSIM by hitting ``Ctrl-C``. That is
useful if our code enters an unexpected loop and we want to
investigate in which function it got stuck.


Instruction and memory dumps
----------------------------

MSIM allows us to inspect not only registers but also memory.

Let us see the ``string`` directory. It contains almost the same code
as the previous example, but uses iteration over a string
(``const char *``) to print the greeting.

.. quiz::

   Compile the code, run MSIM interactively and step until it starts
   printing characters.

   What is the value of the program counter?

Let’s inspect the code of the loop. We can look at
``kernel.disasm`` or inspect it directly from MSIM.

.. archbox:: MIPS

   To inspect things in MSIM, we need to work with physical
   addresses. Recall that ``pc`` contains a virtual address. As long
   as our code runs in the kernel segment, the mapping between the
   virtual and physical addresses is hardwired into the processor
   as a simple shift by 2GB. For example, virtual address ``0x8000042C``
   maps to physical address ``0x42C``.

   It is quite important to remember that if we see an address above
   ``0x80000000`` in MSIM, it points into the kernel segment, but if
   we see a numerically lower address, it is either an untranslated
   physical address (such as those in ``msim.conf``), an address in
   the user segment, which at this time most likely indicates a bug
   in our code.

   Now, we will take the virtual address ``0x80000042C``, translate
   it to a physical address (simply by removing the leading ``8``),
   and disassemble in MSIM.

.. archbox:: RISC-V

   We can use the address ``0x8000106c`` directly, as we are using the
   BARE virtual address translation mode, which keeps the addresses
   unchanged.

To disassemble instructions in MSIM:

.. tabs:: arch

   .. code-tab:: msim
      :caption: MIPS

      [msim] dumpins r4k 0x42c 10

   .. code-tab:: msim
      :caption: RISC-V

      [msim] dumpins rv 0x80001060 10

This will dump 10 instructions starting at the specified address.

.. archbox:: MIPS

   We should notice that we are (in overly simplified terms) reading
   the string via registers ``v0`` and ``v1`` and writing it to the
   console via ``a0``.

   Let’s look at the register content:

   ::

      v0 80000460   v1 00000048   a0 90000000


   ``v0`` looks like a virtual address of our kernel, ``v1`` looks
   like an ASCII value (actually, it is the capital ``H``) and ``a0``
   is the address of our console (recall code in ``src/main.c``).

   So we can guess that ``v0`` would contain the address of the
   string.

.. archbox:: RISC-V

   We should notice that we are (in overly simplified terms) reading
   the string via registers ``a4`` and ``a5`` and writing it to the
   console via ``a3``.

   Let’s look at the register content:

   ::

      a3: 90000000    a4:       48    a5: 8000108a

   ``a5`` looks like a virtual address of our kernel, ``a4`` looks like an
   ASCII value (actually, it is the uppercase ``H``) and ``a3`` is the
   address of our console (recall code in ``src/main.c``).

   So we can guess that ``a5`` would contain the address of the string.

Let’s look at that address. Now we do not want to see it as an
instruction dump but rather as plain **m**\ emory **d**\ ump,
hence:

.. tabs:: arch

   .. code-tab:: msim
      :caption: MIPS

      [msim] dumpmem 0x460 4
        0x00000460    6c6c6548 57202c6f 646c726f 00000a21

   .. code-tab:: msim
      :caption: RISC-V

      [msim] dumpmem 0x8000108a 4
        0x080001088   6c6c6548 57202c6f 646c726f 00000a21

``6c6c`` is actually ``ll`` from our ``Hello`` greeting and if we
translate the rest of the numbers, it is really our greeting.

.. quiz::

   Why is the string ordered backwards?

   If we run ``hexdump -C kernel.bin`` you will see these characters
   there as well.

   .. collapse:: Solution

     While we read strings character by character,
     MSIM dumps memory by 4 byte words. Both MIPS and RISC-V
     are little endian, so the bytes on lower addresses take place
     in less significant bits of the word, making them appear more
     towards the right when written down.


Exception handling
------------------

Let’s now see how MSIM (and our kernel) behaves when things go
wrong.

We will use the ``unaligned`` directory. We will compile it and let us
open ``main.c``.

It contains a simple code: we build an array of individual bytes and
later typecast it to a 32-bit integer. This is something our
program might do for example to inspect memory, however, it is
also an operation that may be illegal on some CPUs.
Including ours as we will shortly see.

(The code uses ``volatile`` variables to prevent the compiler from
optimizing the code too much.)

If we run the code, MSIM will switch to the interactive mode and
show a dump of registers. This is because the access to a 32-bit
integer that is not aligned (the address we access is not a
multiple of the size of an integer) is illegal. The CPU reacts by
generating an exception. Our kernel is currently written so that
it reacts to an exception by switching MSIM to the interactive
mode (which is a sane default for debugging).

We can return to this example and run (once MSIM
switches to the interactive mode) the following commands to find
what addresses caused the problem and what is the interrupt code
(type).

.. tabs:: arch

   .. code-tab:: msim
      :caption: MIPS

      cpu0 cp0d 0x0d
      cpu0 cp0d 0x08
      cpu0 cp0d 0x0e

   .. code-tab:: msim
      :caption: RISC-V

      cpu0 csrd mepc
      cpu0 csrd mcause
      cpu0 csrd mtval


The ``volatile`` modifier
-------------------------

Let us go back to our first kernel again.

You perhaps noticed that our console printer uses a special
modifier ``volatile``. If you are new to C, you may want to read
for example `this
article <https://barrgroup.com/Embedded-Systems/How-To/C-Volatile-Keyword>`__
about ``volatile`` first.

.. quiz::

   Let's compile the code and open ``kernel.disasm`` again. We will see
   that most code of ``kernel_main()`` is a mix of constant loads
   (``li``) and stores to memory (``sb``). These instructions
   represent the call to ``print_char`` that writes the character to
   a special part of memory that represents the console (recall that
   MSIM is printing any value written here on your console).

   Now let us remove the ``volatile`` modifier and recompile the code.
   Let us run MSIM again.

   Nothing (except the newline) was printed!

   We will look at the disassembly again: the code is much shorter! Why?

   .. collapse:: Hint

      Imagine what the code looks like when ``print_char``
      is actually inlined into ``kernel_main``.

   .. collapse:: Solution

      Without ``volatile``, the source is actually this:

      .. code-block:: c

         char *printer = (char*)(0x90000000);
         *printer = 'H';
         *printer = 'e';
         ...
         *printer = '.';

      Any decent compiler will recognize that we are
      overwriting the same variable without reading the
      values. When optimizing code, the compiler is only
      required to preserve an externally visible behavior,
      and a write that nobody reads is not externally
      visible - hence all writes but the last are removed by
      the compiler. This means only ``*printer = '\n'``
      remains.

      Using ``volatile`` informs the compiler that someone
      else (here it is the console device of the simulator,
      but it can also be another thread) can read or write
      the variable and therefore accesses to it must not be
      optimized away.


Surviving without sources
-------------------------

The directory ``endless`` contains only an image of a simple
kernel, without sources.

The kernel image contains an endless loop. Run MSIM, after a while
break the execution with ``Ctrl-C`` to get into the interactive
mode.

Inspect the state of the machine and decide in which function the
endless loop is (function names are in the ``kernel.disasm``
file).


.. collapse:: Hint

   Dump the registers.

.. collapse:: Solution MIPS

   The ``PC`` register will contain values around
   ``0x80000460``, hence it is function ``endless_two``.

.. collapse:: Solution RISC-V

   The ``PC`` register will contain values around
   ``0x80001090``, hence it is function ``endless_two``.

The complex one
---------------

The ``printers`` directory again contains only a binary kernel
image, this time it is a bit bigger kernel and ``msim.conf``
actually contains several printers (consoles).

The task is simple: determine what console device is actually
used. This changes with every boot so do not try editing
``msim.conf``, that would be cheating ;-) …

Note that with newer version of MSIM, you need to execute with
``-n`` as the hardware is configured with time device that adds
non-determinism to the simulator.

To find the right answer, inspect the code loaded into MSIM and
check the contents of the registers. To make the task easier, the
kernel prints dots in an infinite loop.

.. collapse:: Solution

   The printer number is the last but one digit in the
   *Run id*.

   Tracing the instructions would be enough, somewhere in
   the registers we would see the address of the printer.

   Other option is to look into the disassembly and we
   would see that ``print_char`` was not inlined. Hence
   we can watch until program reaches this point
   and then inspect the target address of the ``sb`` instruction.

   .. archbox:: MIPS

      Watch until the program counter reaches address ``0x80000430``
      and look into the content of the ``v0`` register.

   .. archbox:: RISC-V

      Watch until the program counter reaches address ``0x80001068``
      and look into the content of the ``a5`` register.

