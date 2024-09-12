MIPS Tutorial
===============


.. container:: row

   -  `Overview </teaching/nswi200/202324/>`__
   -  `Lectures </teaching/nswi200/202324/lectures/>`__
   -  `Labs </teaching/nswi200/202324/labs/>`__
   -  `Resources </teaching/nswi200/202324/resources/>`__
   -  `Schedule </teaching/nswi200/202324/schedule/>`__
   -  `Toolchain </teaching/nswi200/202324/toolchain/>`__
   -  `Points </teaching/nswi200/202324/points/>`__
   -  `Code style </teaching/nswi200/202324/style/>`__
   -  `Dive into Kernel </teaching/nswi200/202324/dive-into-kernel/>`__
   -  `Teams </teaching/nswi200/202324/teams/>`__
   -  `Q & A </teaching/nswi200/202324/qa/>`__

   .. container:: col-lg-10

      .. container:: alert alert-warning

         Information below is **not for the current semester**. `The
         current semester can be found here. </teaching/nswi200/>`__

      This page contains several small exercises that should help you
      with your first steps within MIPS kernel running in MSIM.

      Here is an overview of the exercises.

      .. container:: table-of-contents

         -  `First compilation <#first-compilation>`__
         -  `Configuring the virtual
            machine <#configuring-the-virtual-machine>`__
         -  `Disassembling the kernel <#disassembling-the-kernel>`__
         -  `From boot to C code <#from-boot-to-c-code>`__
         -  ```kernel_main`` is where the fun
            starts <#kernel-main-is-where-the-fun-starts>`__
         -  `The first modification of the
            kernel <#the-first-modification-of-the-kernel>`__
         -  `Tracing the execution <#tracing-the-execution>`__
         -  `Stepping through the
            execution <#stepping-through-the-execution>`__
         -  `Entering the debugger <#entering-the-debugger>`__
         -  `Inspecting the registers <#inspecting-the-registers>`__
         -  `Matching instructions back to source
            code <#matching-instructions-back-to-source-code>`__
         -  `Instruction and memory
            dumps <#instruction-and-memory-dumps>`__
         -  `Exception handling <#exception-handling>`__
         -  `The ``volatile`` modifier <#the-volatile-modifier>`__
         -  `Surviving without sources <#surviving-without-sources>`__
         -  `The complex one <#the-complex-one>`__

      And now, let’s dive into kernel code :-).

      .. rubric:: First compilation
         :name: first-compilation

      If you have never compiled an operating system kernel (or if you
      are new to C, GCC, or make), you may wish to start with compiling
      a smaller kernel first.

      Please, clone the `examples
      repository <https://gitlab.mff.cuni.cz/teaching/nswi200/2023/examples>`__
      and go into the ``first`` subdirectory. This contents of this
      directory are similar to what you can find in Milestone 01, except
      the tests and some other files are omitted.

      Before we discuss the contents of the directory, run ``make``.
      This command launches the make tool, which reads dependency rules
      from a file named ``Makefile`` and uses them to figure out how to
      compile C sources into a binary executable.

      In this case, make should run a sequence of commands to build the
      ``loader.bin`` executable from the ``loader.S`` source, and the
      ``kernel.bin`` executable from the ``head.S`` and ``main.c``
      sources.

      In Rotunda, ``make`` will produce the following output:

      ::

         make -C kernel
         make[1]: Entering directory '/afs/ms.mff.cuni.cz/u/h/horkv6am/nswi200/examples/first/kernel'
         /usr/bin/mipsel-unknown-linux-gnu-gcc -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 -mno-abicalls -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=c11 -I. -D__ASM__ -c -o boot/loader.o boot/loader.S
         /usr/bin/mipsel-unknown-linux-gnu-ld -G 0 -static -g -T kernel.lds -Map loader.map -o loader.raw boot/loader.o
         /usr/bin/mipsel-unknown-linux-gnu-objcopy -O binary loader.raw loader.bin
         /usr/bin/mipsel-unknown-linux-gnu-objdump -d loader.raw > loader.disasm
         /usr/bin/mipsel-unknown-linux-gnu-gcc -O2 -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 -mno-abicalls -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=c11  -c -o src/main.o src/main.c
         /usr/bin/mipsel-unknown-linux-gnu-gcc -march=r4000 -mabi=32 -mgp32 -msoft-float -mlong32 -G 0 -mno-abicalls -fno-pic -fno-builtin -ffreestanding -nostdlib -nostdinc -pipe -Wall -Wextra -Werror -Wno-unused-parameter -Wmissing-prototypes -g3 -std=c11 -I. -D__ASM__ -c -o src/head.o src/head.S
         /usr/bin/mipsel-unknown-linux-gnu-ld -G 0 -static -g -T kernel.lds -Map kernel.map -o kernel.raw src/main.o src/head.o
         /usr/bin/mipsel-unknown-linux-gnu-objcopy -O binary kernel.raw kernel.bin
         /usr/bin/mipsel-unknown-linux-gnu-objdump -d kernel.raw > kernel.disasm
         make[1]: Leaving directory '/afs/ms.mff.cuni.cz/u/h/horkv6am/nswi200/examples/first/kernel'

      The advantage of using make as opposed to a shell script is in
      that make will only rebuild files (along dependency chains) that
      have changed since the last compilation, which saves build time,
      especially on larger projects (you can try that by running
      ``make`` again now).

      In this example, the rules in the top-level ``Makefile`` just tell
      make to run ``make`` again, but this time using the ``Makefile``
      in the ``kernel`` subdirectory, but more details of the
      compilation will come later on.

      One other file you should note is ``msim.conf``. It contains
      directives for the MSIM simulator, configuring it so as to provide
      a simple computer equipped with one MIPS R4000 processor, two
      blocks of memory, and a console-like device for textual output (we
      will dissect the configuration in the next exercise).

      To run the compiled kernel code, run ``msim`` without any
      arguments. MSIM will load the binary images (``loader.bin`` and
      ``kernel.bin``) into the two memory blocks and reset the simulated
      CPU so that it starts executing code at factory-defined addresses.
      You should see the following output:

      ::

         Hello, World.
         <msim> Alert: XHLT: Machine halt

         Cycles: 41

      The “Hello, World.” message was printed from C code compiled into
      MIPS machine code running on the MIPS processor. Getting the
      target processor to execute your (compiled) C code is usually one
      of the major technical obstacles when starting OS development from
      scratch, which is why we have taken care of this step for now.

      The last line (as well as the line prefixed with ``<msim>``) is
      the output of the simulator, telling us that the CPU executed 41
      virtual cycles, which on MIPS means that it executed 41
      instructions. We can safely ignore those lines for now.

      **If the compilation failed for you, or if the execution printed
      something else**, please, **contact us as soon as possible**: open
      an Issue
      `here <https://gitlab.mff.cuni.cz/teaching/nswi200/2023/forum/-/issues>`__
      and describe what have you tried, what failed and please do not
      forget to describe your environment.

      .. rubric:: Configuring the virtual machine
         :name: configuring-the-virtual-machine

      We will now take a closer look at the ``msim.conf`` file, which
      contains the configuration of the simulated computer that runs
      your kernel.

      We use a simulated computer instead of a real one because that
      makes it much easier to work on your assignments (for one thing,
      installation does not require sacrificing your own computer, also,
      the simulation is completely deterministic and therefore bugs that
      appear once keep appearing until you fix them). However, rest
      assured the simulated environment is close enough to the real
      thing.

      Reading ``msim.conf`` from top to bottom and ignoring the comment
      lines starting with the ``#`` character, the first configuration
      line tells MSIM to add one MIPS R4000 processor (``dr4kcpu``) and
      name it ``cpu0``:

      ::

         add dr4kcpu cpu0

      The next two groups of directives add two blocks of physical
      memory, one for the bootloader and one for the main memory, both
      initialized from files on disk.

      The main memory block (called ``mainmem``) is a read-write memory
      starting at physical address ``0`` with a size of ``1 MiB``. The
      memory block is initialized with the contents of the
      ``kernel/kernel.bin`` file before the simulated computer starts
      running:

      ::

         add rwm mainmem 0
         mainmem generic 1M
         mainmem load "kernel/kernel.bin"

      The bootloader memory block (called ``loadermem``) is a read-only
      memory starting at physical address ``0x1FC00000`` with a size of
      ``4 KiB``, initialized with the contents of the
      ``kernel/loader.bin`` file:

      ::

         add rom loadermem 0x1FC00000
         loadermem generic 4K
         loadermem load "kernel/loader.bin"

      Finally, we add a simple output device (called ``printer``)
      residing at physical address 0x10000000. This device will allow
      the code running in the simulator to display text on the host
      computer console. This is similar to serial console found on real
      hardware, except the printer device is much simpler:

      ::

         add dprinter printer 0x10000000

      This is actually enough for a simple machine and more than enough
      for our purposes :-).

      .. rubric:: Disassembling the kernel
         :name: disassembling-the-kernel

      With the simulator configured to provide us with a simple
      MIPS-based computer, it is now time to look at the files in the
      ``kernel`` directory. Again, there is a ``Makefile`` which
      controls the compilation, and a linker script which controls the
      layout of the binary image produced by the linker.

      We will not dissect the linker script further, because explaining
      it in detail would require additional background. Because you will
      not need to modify it in your assignments, we will just say that
      this is where the public symbols ``_kernel_start`` and
      ``_kernel_end`` (that can be referenced from C code) are defined
      in relation to the layout of the binary image.

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
      to another fixed address, in this case ``0x80000400``, where our
      main code will reside.

      The reason why we keep the rest of the kernel code separate from
      the loader is quite simple – the entry point of the loader is
      quite far from the entry points of the exception handlers, which
      are also hardwired, and which the kernel must implement. We simply
      want to keep the rest of the kernel code in one piece, and that
      means next to the exception handlers.

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

      A question for you: why are the instructions in ``loader.disasm``
      different from ``loader.S``? `Hint. <#ModalWindow_13>`__
      `Solution. <#ModalWindow_14>`__

      .. rubric:: From boot to C code
         :name: from-boot-to-c-code

      We will now look into the ``src`` directory, where the foundations
      of our kernel reside.

      The ``head.S`` file contains a lot of assembly code, but do not be
      afraid ;-).

      Find the line containing ``start:`` (around line 120). Above this,
      we can see a special directive ``.org 0x400`` that says that the
      following code will be placed at address 0x400 bytes away from the
      start of the code segment. The linker specifies that the code
      segment starts at 0x80000000, together this yields 0x80000400 -
      exactly the address our boot loader jumps to! Hence, after the
      boot loader is done, the execution will continue here.

      We start by setting up few registers (such as the stack pointer)
      and execute ``jal kernel_main``. This will pass control from the
      assembly code to the ``kernel_main`` function, which is a standard
      C function that you can see if you open ``src/main.c``.

      These few lines of assembler (``loader.S`` and ``head.S``)
      constitute the only assembly code needed to boot the processor and
      get into C.

      .. rubric:: ``kernel_main`` is where the fun starts
         :name: kernel-main-is-where-the-fun-starts

      The last file we have not commented much on is ``src/main.c``.

      It contains the ``kernel_main()`` function, which is called
      shortly after boot. This is the function you will extend to have
      your kernel initialize itself and launch the user applications.

      Right now it contains only a very short greeting.

      Printing from the simulator is trivial: since we told MSIM that
      there should be a console printer device available at address
      0x10000000, MSIM monitors this address and any write to it causes
      the written character to appear at the console.

      A question for you: if you look up the console printer device
      address in the source code, you will see it is 0x90000000, but
      ``msim.conf`` says 0x10000000. Why? `Hint. <#ModalWindow_15>`__
      `Solution. <#ModalWindow_16>`__

      .. rubric:: The first modification of the kernel
         :name: the-first-modification-of-the-kernel

      Modify the kernel so that it prints the greeting with an
      exclamation mark instead of a plain period. After all, we can be
      proud of it ;-).

      Before running ``msim`` again do not forget to recompile with
      ``make``.

      What commands were actually executed by make?
      `Solution. <#ModalWindow_17>`__

      .. rubric:: Tracing the execution
         :name: tracing-the-execution

      Let’s see which instructions were actually executed by MSIM. This
      may come in handy in later debugging tasks.

      Run ``msim -t``. This turns on a trace mode where MSIM prints
      every instruction as it is executed. (Unfortunately, there is just
      one console, so the MSIM ouput is interleaved with your OS
      output.)

      Compare the trace with your ``*.disasm`` files. What is the
      difference? `Solution. <#ModalWindow_18>`__

      .. rubric:: Stepping through the execution
         :name: stepping-through-the-execution

      To run the kernel instruction by instruction interactively, launch
      MSIM with ``msim -i``. This time, MSIM will wait for further
      commands, as indicated by the ``[msim]`` prompt.

      Simply typing ``continue`` will resume standard execution, which
      will run our OS and eventually terminate MSIM.

      Run MSIM again but instead of typing ``continue``, just hit Enter.
      An empty command in MSIM is equivalent to typing ``step`` and
      executes a single instruction. You should see how the greeting
      starts to appear next to the prompt as you continue pressing
      Enter.

      You can also do ``step 10`` to execute ten instructions.

      Try it.

      .. rubric:: Entering the debugger
         :name: entering-the-debugger

      Stepping through your kernel from the very first instruction is
      not so useful for debugging when the code you are interested in is
      executed long after boot. In that case, you can also enter the
      interactive mode programmatically, by asking for it from inside
      your (kernel) code. To do that, use a special assembly language
      instruction, which the real CPU does not recognize but MSIM does.

      Insert the following fragment at a location (in the C code) where
      you want to interrupt the execution.

      ::

         __asm__ volatile(".word 0x29\n");

      Try it: insert the break after printing ``Hello``. If you execute
      ``msim``, it will print ``Hello`` and enter interactive mode. You
      can again step throught the execution or ``continue``.

      .. rubric:: Inspecting the registers
         :name: inspecting-the-registers

      Start MSIM in interactive mode again and type ``set trace`` as the
      first command.

      Then hit Enter several times. You executed several instructions
      and MSIM is printing what instructions are executed.

      We can also inspect all registers at once. Use the ``cpu0 rd``
      command for a **r**\ egister **d**\ ump of ``cpu0`` processor
      (that is the only processor that we added to our computer in
      MSIM).

      This is an extremely useful command as it allows you to inspect
      what is the current state of the processor and what code it
      executes.

      Which register would tell you what code is executed?
      `Solution. <#ModalWindow_19>`__

      .. rubric:: Matching instructions back to source code
         :name: matching-instructions-back-to-source-code

      Start MSIM again in the interactive mode and step until it starts
      printing the greeting. Look at the register dump.

      You will see something like this (note that we have dropped the
      64bit extension to make the dump a bit shorter):

      ::

          0 00000000   at 00000000   v0 90000000   v1 00000000   a0 00000000
         a1 00000048   a2 00000000   a3 00000000   t0 00000000   t1 00000000
         t2 00000000   t3 00000000   t4 00000000   t5 00000000   t6 00000000
         t7 00000000   s0 00000000   s1 00000000   s2 00000000   s3 00000000
         s4 00000000   s5 00000000   s6 00000000   s7 00000000   t8 00000000
         t9 00000000   k0 0000FF01   k1 00000000   gp 80000000   sp 80000400
         fp 00000000   ra 80000420   pc 8000043C   lo 00000000   hi 00000000

      Note that in our dump, ``pc`` contains the ``8000043C``.

      Open ``kernel.disasm`` and find this address there. It is few
      lines below ``80000430 <kernel_main>`` which indicates that it is
      an instruction inside ``kernel_main()``.

      This is extremely important information because it allows you to
      decide in which function your OS will be when it is interrupted
      etc.

      You can interrupt code in MSIM by hitting ``Ctrl-C``. That is
      useful if your code enters an unexpected loop and you want to
      investigate in which function it got stuck.

      .. rubric:: Instruction and memory dumps
         :name: instruction-and-memory-dumps

      MSIM allows you to inspect not only registers but also memory.

      Go to the ``string`` directory. It contains almost the same code
      as the previous example, but uses iteration over a string
      (``const char *``) to print the greeting.

      Compile the code, run MSIM interactively and step until it starts
      printing characters.

      What is the value of the program counter?

      Let’s inspect the code of the loop. We can look at
      ``kernel.disasm`` or inspect it directly from MSIM.

      To inspect things in MSIM, we need to work with physical
      addresses. Recall that ``pc`` contains a virtual address. As long
      as our code runs in the kernel segment (which it will for the
      following few weeks), the mapping between the virtual and physical
      addresses is hardwired into the processor as a simple shift by
      2GB. For example, virtual address ``0x8000042C`` maps to physical
      address ``0x42C``.

      It is quite important to remember that if you see an address above
      ``0x80000000`` in MSIM, it points into the kernel segment, but if
      you see a numerically lower address, it is either an untranslated
      physical address (such as those in ``msim.conf``), an address in
      the user segment, which at this time most likely indicates a bug
      in your code.

      Now, we will take the virtual address ``0x80000042C``, translate
      it to a physical address (simply by removing the leading ``8``),
      and disassemble in MSIM:

      ::

         [msim] dumpins r4k 0x42c 10

      This will dump 10 instructions starting at address ``0x42c`` (use
      ``rv`` instead of ``r4k`` for dumping instructions for RISC-V).

      You should notice that we are (in overly simplified terms) reading
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

      Let’s look at that address. Now we do not want to see it as an
      instruction dump but rather as plain **m**\ emory **d**\ ump,
      hence:

      ::

         [msim] dumpmem 0x460 4
         0x00000460    6c6c6548 57202c6f 646c726f 00000a21

      ``6c6c`` is actually ``ll`` from our ``Hello`` greeting and if you
      translate the rest of the numbers, it is really our greeting.

      Why is the string ordered backwards?

      If you run ``hexdump -C kernel.bin`` you will see these characters
      there as well.

      .. rubric:: Exception handling
         :name: exception-handling

      Let’s now see how MSIM (and our kernel) behaves when things go
      wrong.

      Go to the ``unaligned`` directory, compile it and open ``main.c``.

      It contains simple code: we build an array of individual bytes and
      later typecast it to a 32-bit integer. This is something your
      program might do for example to inspect memory, however, it is
      also an operation that may be illegal on your CPU, as we will
      shortly see.

      (The code uses ``volatile`` variables to prevent the compiler from
      optimizing too much.)

      If you run the code, MSIM will switch to the interactive mode and
      show a dump of registers. This is because the access to a 32-bit
      integer that is not aligned (the address we access is not a
      multiple of the size of an integer) is illegal. The CPU reacts by
      generating an exception. Your kernel is currently written so that
      it reacts to an exception by switching MSIM to the interactive
      mode (which is a sane default for debugging for now).

      Later on, you can return to this example and run (once MSIM
      switches to the interactive mode) the following commands to find
      what addresses caused the problem and what is the interrupt code
      (type).

      ::

         cpu0 cp0d 0x0d
         cpu0 cp0d 0x08
         cpu0 cp0d 0x0e

      .. rubric:: The ``volatile`` modifier
         :name: the-volatile-modifier

      Let us go back to our first kernel again.

      You perhaps noticed that our console printer uses a special
      modifier ``volatile``. If you are new to C, you may want to read
      for example `this
      article <https://barrgroup.com/Embedded-Systems/How-To/C-Volatile-Keyword>`__
      about ``volatile`` first.

      Compile the code and open ``kernel.disasm`` again. You will see
      that most code of ``kernel_main()`` is a mix of constant loads
      (``LI``) and stores to memory (``SB``). These instructions
      represent the call to ``print_char`` that writes the character to
      a special part of memory that represents the console (recall that
      MSIM is printing any value written here on your console).

      Now remove the ``volatile`` modifier and recompile the code. Run
      MSIM again.

      Nothing (except the newline) was printed!

      Look at the disassembly again - the code is much shorter! Why?
      `Hint. <#ModalWindow_20>`__ `Solution. <#ModalWindow_21>`__

      .. rubric:: Surviving without sources
         :name: surviving-without-sources

      The directory ``endless`` contains only an image of a simple
      kernel, without sources.

      The kernel image contains an endless loop. Run MSIM, after a while
      break the execution with ``Ctrl-C`` to get into the interactive
      mode.

      Inspect the state of the machine and decide in which function the
      endless loop is (function names are in the ``kernel.disasm``
      file).

      `Hint. <#ModalWindow_22>`__ `Solution. <#ModalWindow_23>`__

      .. rubric:: The complex one
         :name: the-complex-one

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
      `Solution. <#ModalWindow_24>`__

      .. container:: modal fade
         :name: ModalWindow_1

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  Think about the limited instruction repertoire of the
                  CPU.

      .. container:: modal fade
         :name: ModalWindow_10

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  Dump the registers.

      .. container:: modal fade
         :name: ModalWindow_11

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  The ``PC`` register will contain values around
                  ``0x80000460``, hence it is function ``endless_two``.

      .. container:: modal fade
         :name: ModalWindow_12

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  The printer number is the last but one digit in the
                  *Run id*.

                  Tracing the instructions would be enough, somewhere in
                  the registers we would see the address of the printer.

                  Other option is to look into the disassembly and we
                  would see that ``print_char`` was not inlined. Hence
                  we can watch until program counter becomes
                  ``0x80000430`` and then inspect the ``v0`` register
                  (it is the only register used with ``SB`` for
                  addressing).

      .. container:: modal fade
         :name: ModalWindow_13

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  Think about the limited instruction repertoire of the
                  CPU.

      .. container:: modal fade
         :name: ModalWindow_14

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  The difference in code concerns the loading of the
                  32-bit constant (jump target address). The CPU does
                  not have an instruction that can load an entire 32-bit
                  constant in one go (because the instruction itself
                  must fit into 32 bits), hence two instructions are
                  used. The assembly code uses a shorthand notation so
                  that the programmer does not have to perform this
                  trivial conversion.

      .. container:: modal fade
         :name: ModalWindow_15

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  Think about virtual and physical addresses.

      .. container:: modal fade
         :name: ModalWindow_16

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  The code uses virtual addresses, but the simulator
                  configuration uses physical addresses (exactly what a
                  real hardware would see). In the kernel segment,
                  virtual addresses are mapped to physical addresses
                  simply by masking the highest bit - virtual address
                  0x80000000 therefore corresponds to physical address
                  0, and so on. The mapping is intentionally simple
                  because the kernel must run even before more complex
                  mapping structures, such as page tables, can be set
                  up.

                  An important note: you probably noticed that we print
                  the characters one by one instead of using ``printf``
                  or ``puts``. That is because we are in our own kernel
                  and we do not have any of these functions (yet). As a
                  matter of fact, **you will always have only functions
                  that you implement yourself**. So no ``printf``, no
                  ``fopen``, no ``malloc`` and so on unless you write
                  your own.

      .. container:: modal fade
         :name: ModalWindow_17

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  Just replace ``'.'`` with ``'!'`` in ``main.c`` :-).

                  Make should recompile only ``main.c`` into ``main.o``
                  and re-link the ``kernel.*`` files. Files related to
                  the bootloader should remain without change.

      .. container:: modal fade
         :name: ModalWindow_18

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  The answer is obvious: ``*.disasm`` contains the code
                  in its static form while the trace represents the true
                  execution - jumps are taken, loop bodies are executed
                  repeatedly etc.

      .. container:: modal fade
         :name: ModalWindow_19

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  The ``pc`` register is the program counter telling the
                  (virtual) address where the CPU decodes the next
                  instruction.

      .. container:: modal fade
         :name: ModalWindow_2

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  The difference in code concerns the loading of the
                  32-bit constant (jump target address). The CPU does
                  not have an instruction that can load an entire 32-bit
                  constant in one go (because the instruction itself
                  must fit into 32 bits), hence two instructions are
                  used. The assembly code uses a shorthand notation so
                  that the programmer does not have to perform this
                  trivial conversion.

      .. container:: modal fade
         :name: ModalWindow_20

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  Imagine what the code looks like when ``print_char``
                  is actually inlined into ``kernel_main``.

      .. container:: modal fade
         :name: ModalWindow_21

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  Without ``volatile``, the source is actually this:

                  ::

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

      .. container:: modal fade
         :name: ModalWindow_22

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  Dump the registers.

      .. container:: modal fade
         :name: ModalWindow_23

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  The ``PC`` register will contain values around
                  ``0x80000460``, hence it is function ``endless_two``.

      .. container:: modal fade
         :name: ModalWindow_24

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  The printer number is the last but one digit in the
                  *Run id*.

                  Tracing the instructions would be enough, somewhere in
                  the registers we would see the address of the printer.

                  Other option is to look into the disassembly and we
                  would see that ``print_char`` was not inlined. Hence
                  we can watch until program counter becomes
                  ``0x80000430`` and then inspect the ``v0`` register
                  (it is the only register used with ``SB`` for
                  addressing).

      .. container:: modal fade
         :name: ModalWindow_3

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  Think about virtual and physical addresses.

      .. container:: modal fade
         :name: ModalWindow_4

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  The code uses virtual addresses, but the simulator
                  configuration uses physical addresses (exactly what a
                  real hardware would see). In the kernel segment,
                  virtual addresses are mapped to physical addresses
                  simply by masking the highest bit - virtual address
                  0x80000000 therefore corresponds to physical address
                  0, and so on. The mapping is intentionally simple
                  because the kernel must run even before more complex
                  mapping structures, such as page tables, can be set
                  up.

                  An important note: you probably noticed that we print
                  the characters one by one instead of using ``printf``
                  or ``puts``. That is because we are in our own kernel
                  and we do not have any of these functions (yet). As a
                  matter of fact, **you will always have only functions
                  that you implement yourself**. So no ``printf``, no
                  ``fopen``, no ``malloc`` and so on unless you write
                  your own.

      .. container:: modal fade
         :name: ModalWindow_5

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  Just replace ``'.'`` with ``'!'`` in ``main.c`` :-).

                  Make should recompile only ``main.c`` into ``main.o``
                  and re-link the ``kernel.*`` files. Files related to
                  the bootloader should remain without change.

      .. container:: modal fade
         :name: ModalWindow_6

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  The answer is obvious: ``*.disasm`` contains the code
                  in its static form while the trace represents the true
                  execution - jumps are taken, loop bodies are executed
                  repeatedly etc.

      .. container:: modal fade
         :name: ModalWindow_7

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  The ``pc`` register is the program counter telling the
                  (virtual) address where the CPU decodes the next
                  instruction.

      .. container:: modal fade
         :name: ModalWindow_8

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  Imagine what the code looks like when ``print_char``
                  is actually inlined into ``kernel_main``.

      .. container:: modal fade
         :name: ModalWindow_9

         .. container:: modal-dialog

            .. container:: modal-content

               .. container:: modal-body

                  ×
                  Without ``volatile``, the source is actually this:

                  ::

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
