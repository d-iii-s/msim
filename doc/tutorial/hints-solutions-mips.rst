Hints and solutions
===================

.. _mips-hint-1:

Hint
~~~~

Think about the limited instruction repertoire of the
CPU.


.. _mips-hint-13:

Hint
~~~~

Think about the limited instruction repertoire of the CPU.


.. _mips-solution-14:

Solution
~~~~~~~~

The difference in code concerns the loading of the
32-bit constant (jump target address). The CPU does
not have an instruction that can load an entire 32-bit
constant in one go (because the instruction itself
must fit into 32 bits), hence two instructions are
used. The assembly code uses a shorthand notation so
that the programmer does not have to perform this
trivial conversion.

.. _mips-hint-15:

Hint
~~~~

Think about virtual and physical addresses.

.. _mips-solution-16:

Solution
~~~~~~~~
      
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

.. _mips-solution-17:

Solution
~~~~~~~~

Just replace ``'.'`` with ``'!'`` in ``main.c`` :-).

Make should recompile only ``main.c`` into ``main.o``
and re-link the ``kernel.*`` files. Files related to
the bootloader should remain without change.

.. _mips-solution-18:

Solution
~~~~~~~~
        
The answer is obvious: ``*.disasm`` contains the code
in its static form while the trace represents the true
execution - jumps are taken, loop bodies are executed
repeatedly etc.

.. _mips-solution-19:

Solution
~~~~~~~~

The ``pc`` register is the program counter telling the
(virtual) address where the CPU decodes the next
instruction.

.. _mips-hint-20:

Hint
~~~~

Imagine what the code looks like when ``print_char``
is actually inlined into ``kernel_main``.

.. _mips-solution-21:

Solution
~~~~~~~~

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

.. _mips-hint-22:

Hint
~~~~
      
Dump the registers.

.. _mips-solution-23:

Solution
~~~~~~~~

The ``PC`` register will contain values around
``0x80000460``, hence it is function ``endless_two``.

.. _mips-solution-24:

Solution
~~~~~~~~
      
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
