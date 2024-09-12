
Hints and solutions
===================

.. _riscv-hint-1:

Hint
~~~~

Think about the limited instruction repertoire of the CPU.

.. _riscv-solution-1:

Solution
~~~~~~~~

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

.. _riscv-solution-3:

Solution
~~~~~~~~

Just replace ``'.'`` with ``'!'`` in ``main.c`` :-).

Make should recompile only ``main.c`` into ``main.o`` and re-link the
``kernel.*`` files. Files related to the bootloader should remain
without change.

.. _riscv-solution-4:

Solution
~~~~~~~~

The answer is obvious: ``*.disasm`` contains the code in its static form
while the trace represents the true execution - jumps are taken, loop
bodies are executed repeatedly etc.

.. _riscv-solution-5:

Solution
~~~~~~~~

The ``pc`` register is the program counter telling the (virtual) address
where the CPU decodes the next instruction.

.. _riscv-hint-3:

Hint
~~~~

Imagine what the code looks like when ``print_char`` is actually inlined
into ``kernel_main``.

.. _riscv-solution-6:

Solution
~~~~~~~~

Without ``volatile``, the source is actually this:

.. code:: c

       char *printer = (char*)(0x90000000);
       *printer = 'H';
       *printer = 'e';
       ...
       *printer = '.';

Any decent compiler will recognize that we are overwriting the same
variable without reading the values. When optimizing code, the compiler
is only required to preserve an externally visible behavior, and a write
that nobody reads is not externally visible - hence all writes but the
last are removed by the compiler. This means only ``*printer = '\n'``
remains.

Using ``volatile`` informs the compiler that someone else (here it is
the console device of the simulator, but it can also be another thread)
can read or write the variable and therefore accesses to it must not be
optimized away.

.. _riscv-hint-4:

Hint
~~~~

Dump the registers.

.. _riscv-solution-7:

Solution
~~~~~~~~

The ``PC`` register will contain values around ``0x80000460``, hence it
is function ``endless_two``.

.. _riscv-solution-8:

Solution
~~~~~~~~

The printer number is the last but one digit in the *Run id*.

Tracing the instructions would be enough, somewhere in the registers we
would see the address of the printer.

Other option is to look into the disassembly and we would see that
``print_char`` was not inlined. Hence we can watch until program counter
becomes ``0x80001068`` and then inspect the ``a5`` register (it is the
only register used with ``sb`` for addressing).
