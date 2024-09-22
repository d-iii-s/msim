RISC-V Virtual memory tutorial
==============================

This page will introduce you to several ways in which MSIM can help you
work with RISC-V virtual memory translation, be it with setting up pagetables,
checking that the translation works as expected
or figuring out issues with the TLB.

We expect you are already already familiar know how the Sv32
adressing mode works. If you are not, please consider reading the
appropriate chapter of the
`RISC-V Privileged Specification <https://github.com/riscv/riscv-isa-manual/releases/download/20240411/priv-isa-asciidoc.pdf>`__.

We also highly encourage you complete the :doc:`Mini-kernel tutorial <mini-kernel>`
first, as the same build process and project structure is used
for this tutorial.

.. contents:: Here is an overview of this tutorial
    :local:

Setting up
----------

We have prepared a small example project in ``contrib/virtmem-tutorial-riscv32``.
You will find it is setup in the same way as the excercises in the :doc:`Mini-kernel tutorial <mini-kernel>`.
But there's a crucial difference, we have also prepared a pagetable in ``kernel/pagetable.bin``,
that gets loaded by MSIM and which is ready to be used.

.. quiz::

    How many non-empty **P**\ age **T**\ able **E**\ ntries can be found in this pagetable?

    .. collapse:: Hint

        Use ``hexdump`` to display the contents.
        (Note that ``hexdump`` omits long zero-only segments of the file)

    .. collapse:: Solution

        There are 9 in total. One for each 4 B non-zero word in the file.

Brave developers can try to decode this pagetable by hand, if you intend to do so,
consult ``msim.conf`` first and look at which physical address this pagetable gets loaded.

If/After you've finished, let's try to run the example program.
There are several breakpoints breakpoints set in the ``kernel_main()`` function.
Compile the project and run ``msim``, you should hit the first breakpoint labeled ``Still in BARE mode``.

BARE mode
---------

Here we are still in ``BARE`` translation mode.
Make sure of this by displaying the content of the ``satp`` CSR.

.. code:: msim

    [msim] cpu0 csrd satp
    satp (0x180):
    satp 0x00000000 [ Mode: Bare ]

In this mode, no translation is made between virtual and physical addresses.
The same address you use in your code is the one which will be accessed in memory.
There's not much interesting happening with translation now, but you can return here
after you learn about the different commands of MSIM and test how they behave when running in ``BARE`` mode.

Switching to Sv32
-----------------

The next line after the break is where all of the magic happens.
We setup the CPU to use the pagetable by writing the correct value to the ``satp`` CSR.
Continue the execution of MSIM (type ``continue`` and press Enter),
you'll hit the next breakpoint labeled ``Switched to Sv32``.

Let's try displaying the content of ``satp`` again.

.. code:: msim

    [msim] cpu0 csrd satp
    satp (0x180):
    satp 0x800a0000 [ Mode: Sv32 ASID: 0 PPN: 0x0a0000 (Physical address: 0x0a0000000) ]

A lot more is happening here now, we are now using the ``Sv32`` translation scheme, with an ASID of 0
and the current pagetable sits at physical address ``0xA0000000``.

.. quiz::

    Why is there an extra ninth ``0`` in front of the PPN and physical address?

    .. collapse:: Solution

        The ``Sv32`` translation scheme actually allows for 34 bit physical addresses, which results in 9 hex digits.

What's inside the pagetable
---------------------------

Let's look inside the pagetable now.
If you have tried to decode the pagetable manually, now's the time to check your results.
For this, we will use the ``ptd`` command

.. code:: msim

    [msim] cpu0 ptd
    satp 0x800a0000 [ Mode: Sv32 ASID: 0 PPN: 0x0a0000 (Physical address: 0x0a0000000) ]
    0x800: [ PPN: 0x080000 RSW: 00 -AG- XWRV ] [ Megapage ]
    0x900: [ PPN: 0x090000 RSW: 00 --G- -WRV ] [ Megapage ]
    0xa00: [ PPN: 0x0a0000 RSW: 00 --G- -WRV ] [ Megapage ]
    0xb00: [ PPN: 0x0a0001 RSW: 00 ---- ---V ]
      0x000: [ PPN: 0x0c0000 RSW: 00 ---- XWRV ]
      0x008: [ PPN: 0x0c0000 RSW: 00 ---- --RV ]
      0x00c: [ PPN: 0x0c0000 RSW: 00 --G- XWRV ]
      0x010: [ PPN: 0x0c0000 RSW: 00 ---U XWRV ]

This command first displays the content of ``satp`` so we can check with pagetable we are working with.
Then it traverses the pagetable, displaying all valid PTEs and for each non-leaf PTE it descends
and traverses the second-level pagetable.

Each line thus corresponds to one PTE.
It starts with the address offset of this particular PTE in its page, then it displays the stored PPN and RSW bits.
It ends with the individual bitfields ``DAGU XWRV``. A corresponding letter is displayed if this bit is ``1``,
a dash is present instead if this bit is ``0``.
PTEs representing a megapage are denoted as such, second level PTEs are indented with two spaces.

.. quiz::

    What do the individual letters in ``DAGU XWRV`` stand for?

    .. collapse:: Hint

        Look at the `RISC-V Privileged Specification <https://github.com/riscv/riscv-isa-manual/releases/download/20240411/priv-isa-asciidoc.pdf>`__
        Chaper 10.3. Sv32: Page-Based 32-bit Virtual-Memory Systems.

    .. collapse:: Solution

        - **D**\ irty
        - **A**\ ccessed
        - **G**\ lobal
        - **U**\ ser
        - e\ **X**\ ecute
        - **W**\ rite
        - **R**\ ead
        - **V**\ alid

The ``ptd`` displays only valid PTEs. If you also want to display invalid ones,
you can use the verbose flag:

.. code:: msim

    [msim] cpu0 ptd v
    satp 0x800a0000 [ Mode: Sv32 ASID: 0 PPN: 0x0a0000 (Physical address: 0x0a0000000) ]
    0x800: [ PPN: 0x080000 RSW: 00 -AG- XWRV ] [ Megapage ]
    0x900: [ PPN: 0x090000 RSW: 00 --G- -WRV ] [ Megapage ]
    0xa00: [ PPN: 0x0a0000 RSW: 00 --G- -WRV ] [ Megapage ]
    0xb00: [ PPN: 0x0a0001 RSW: 00 ---- ---V ]
      0x000: [ PPN: 0x0c0000 RSW: 00 ---- XWRV ]
      0x004: [ PPN: 0x0c0000 RSW: 00 ---- XWR- ]
      0x008: [ PPN: 0x0c0000 RSW: 00 ---- --RV ]
      0x00c: [ PPN: 0x0c0000 RSW: 00 --G- XWRV ]
      0x010: [ PPN: 0x0c0000 RSW: 00 ---U XWRV ]

This way, all non-zero PTEs are displayed (and indeed, there are 9 of them ;-) ).

If you don't want to dump the content of the currently active pagetable, but would rather
specify it by address, you can use the ``sptd`` command, which also supports the verbose flag.

.. code:: msim

    [msim] cpu0 sptd 0xA0000000
    0x800: [ PPN: 0x080000 RSW: 00 -AG- XWRV ] [ Megapage ]
    0x900: [ PPN: 0x090000 RSW: 00 --G- -WRV ] [ Megapage ]
    0xa00: [ PPN: 0x0a0000 RSW: 00 --G- -WRV ] [ Megapage ]
    0xb00: [ PPN: 0x0a0001 RSW: 00 ---- ---V ]
      0x000: [ PPN: 0x0c0000 RSW: 00 ---- XWRV ]
      0x008: [ PPN: 0x0c0000 RSW: 00 ---- --RV ]
      0x00c: [ PPN: 0x0c0000 RSW: 00 --G- XWRV ]
      0x010: [ PPN: 0x0c0000 RSW: 00 ---U XWRV ]

You can now again continue the execution of MSIM, some text will get printed to the console,
after which another breakpoint will be hit.

.. quiz::

    Dump the pagetable again, how has it changed?

    .. collapse:: Solution

        The PTE corresponding to the printer device has the ``DA`` bits set now.
        This signifies that this (mega-)page has been written to.

After this breakpoint the ``play_with_memory()`` function gets called.
Here the byte corresponding to the letter ``A`` is written to some address, from where it's read back into ``value0``.
A byte from another address is read into ``value2``, both of these values get printed, which results in the letter ``A`` being printed twice.

.. quiz::

    Where did the ``A`` loaded into ``value2`` come from?

    .. collpse:: Hint

        Inspect the second level pagetable.

    .. collapse:: Solution

        The virtual pages ``0xB0000`` and ``0xB0002`` are both mapped to the same physical page ``0xC0000``, so the ``A`` written to one can be read from the second.

.. quiz::

    Some code is commented out in this function, try to uncomment it and see what happens.
    Try to experiment in this function with writing to and reading from different addresses. 
    How do the ``RWXV`` bits change the behavior?
    You can also observe how do the ``DA`` bits change, do you notice anything interesting?

    .. collapse:: Solution

        As is required by the specification, accessing a page with the ``V`` bit equal to ``0`` will raise a pagefault.
        So will reading a page without ``R`` permission and writing to a page without a ``W`` permission.

        When you read from a page the ``A`` bit gets set only for the PTE through which this memory has been accessed, this works the same for the ``D`` bit and writing.
        These bits do not change for the other pages which map to the same memory, even if the backing memory behind them has been read/written to.

When that one translation does not work
---------------------------------------

While using virtual memory translation, you might encounter a situation, when some address you thought will get translated correctly doesn't or vice versa.
For these cases, MSIM offers the ``tr`` command, which perform the virtual address translation and describes the individual steps it took.

Let's say we have run our example program up to the ``After printing to console`` labeled breakpoint,
and let's see how the different memory accesses in ``play_with_memory()`` are translated.

.. code:: msim

    [msim] cpu0 tr 0xB0000000
    satp 0x800a0000 [ Mode: Sv32 ASID: 0 PPN: 0x0a0000 (Physical address: 0x0a0000000) ]
    VPN[1]: 0x2c0 VPN[0]: 0x000 page offset: 0x000
    PTE1: [ PPN: 0x0a0001 RSW: 00 ---- ---V ]
      This entry ^ physical address: 0x0a0000b00 = 0x0a0000000 + 0x2c0 * 4
    PTE2: [ PPN: 0x0c0000 RSW: 00 ---- XWRV ]
      This entry ^ physical address: 0x0a0001000 = 0x0a0001000 + 0x000 * 4

    OK: 0x08b0000000 => 0x0c0000000

.. code:: msim
    [msim] cpu0 tr 0xB0001000
    satp 0x800a0000 [ Mode: Sv32 ASID: 0 PPN: 0x0a0000 (Physical address: 0x0a0000000) ]
    VPN[1]: 0x2c0 VPN[0]: 0x001 page offset: 0x000
    PTE1: [ PPN: 0x0a0001 RSW: 00 ---- ---V ]
      This entry ^ physical address: 0x0a0000b00 = 0x0a0000000 + 0x2c0 * 4
    PTE2: [ PPN: 0x0c0000 RSW: 00 ---- XWR- ]
      This entry ^ physical address: 0x0a0001004 = 0x0a0001000 + 0x001 * 4

    PAGE FAULT - Invalid PTE in 2nd level

.. code:: msim
    [msim] cpu0 tr 0xB0002000
    satp 0x800a0000 [ Mode: Sv32 ASID: 0 PPN: 0x0a0000 (Physical address: 0x0a0000000) ]
    VPN[1]: 0x2c0 VPN[0]: 0x002 page offset: 0x000
    PTE1: [ PPN: 0x0a0001 RSW: 00 ---- ---V ]
      This entry ^ physical address: 0x0a0000b00 = 0x0a0000000 + 0x2c0 * 4
    PTE2: [ PPN: 0x0c0000 RSW: 00 ---- --RV ]
      This entry ^ physical address: 0x0a0001008 = 0x0a0001000 + 0x002 * 4

    OK: 0x08b0002000 => 0x0c0000000

The first line again shows us the content of ``satp`` which is the start point of memory translation.
It then shows how the virtual address gets split into the three parts ``VPN[1]``, ``VPN[0]`` and ``offset``.
The PTEs used for the translation are showed next, together with their (physical) address.
The last line either describes the successful translation or displays the reason why the translation failed.
Note that access rights are not taken into account here, but you can deduce them from the last displayed PTE.

.. quiz::

    Try to dump the translation of an address of some instruction.
    How does this translation differ from the previous ones?

    .. collapse:: Hint

        Look into ``kernel/kernel.disasm`` and pick any address you see.

    .. collapse:: Solution

        The translation is found in the TLB.
        Flush the TLB using the ``cpu0 tlbflush`` command and try again.
        How does the translation differ now?

        .. collapse:: Solution

            Only one level of the pagetable is used.
            This is because the code is mapped using a megapage.

In addition to ``tr`` MSIM also supports the ``str`` command, where similarly to ``sptd`` you specify the used pagetable by its physical address.
``str`` completely ignores the TLB.

.. code:: msim

    [msim] cpu0 str 0xA0000000 0xB0000000
    VPN[1]: 0x2c0 VPN[0]: 0x000 page offset: 0x000
    PTE1: [ PPN: 0x0a0001 RSW: 00 ---- ---V ]
      This entry ^ physical address: 0x0a0000b00 = 0x0a0000000 + 0x2c0 * 4
    PTE2: [ PPN: 0x0c0000 RSW: 00 ---- XWRV ]
      This entry ^ physical address: 0x0a0001000 = 0x0a0001000 + 0x000 * 4

    OK: 0x08b0000000 => 0x0c0000000

What even is the TLB?
---------------------

.. quiz:: 

    That is a good question; what even is the TLB?

    .. collapse:: Hint

        TLB stands for **T**\ ranslation **L**\ ookaside **B**\ uffer

    .. collapse:: Solution

        TLB is a cache used to store virtual translation results.
        It works on the level of pages (either 4 KiB or 4 MiB megapages).
        E.g. if we first translate ``0x12345000 -> 0x6789A000`` using a pagetable
        (and thus reading twice from memory), we cache that the ``0x12345`` ppn is mapped
        to ``0x6789A``. Let's say we want to translate ``0x123450F0`` next,
        we first look into the TLB and notice, that we know how to translate this address
        without even looking inside of the pagetable. So we do so and translate it to
        ``0x6789A0F0``.

        These entries are added automatically to a finite TLB, if there is not a free space for the new
        entry, the **L**\ east **R**\ ecently **U** sed entry is evicted.
        The ``sfence.vma`` instruction serves for manual eviction, of either the whole TLB,
        of all etries with a given ASID, all entries which map a given virtual address
        or based on both ASID and address.

``tlbd``
^^^^^^^^

``tlbflush``
^^^^^^^^^^^^
