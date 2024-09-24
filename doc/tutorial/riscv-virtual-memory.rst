RISC-V memory commands tutorial
===============================

This page will introduce you to several ways in which MSIM can help you
work with RISC-V virtual memory translation, be it with setting up pagetables,
checking that the translation works as expected
or figuring out issues caused by the TLB.

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
You will find it is setup in the same way as the excercises in the :doc:`Mini-kernel tutorial <mini-kernel>`,
but here's a crucial difference. You can find a pagetable in ``kernel/pagetable.bin``,
which gets loaded by MSIM to address ``0xA0000000`` and is ready to be used out-of-the-box.

.. quiz::

    How many non-zero **P**\ age **T**\ able **E**\ ntries can be found in this pagetable?

    .. collapse:: Hint

        Use ``hexdump`` to display the contents.
        (Note that ``hexdump`` omits long zero-only segments of the file)

    .. collapse:: Solution

        There are 9 in total. One for each 4 B non-zero word in the file.

Brave developers might try to decode this pagetable by hand.

After/If you've finished, let's run the example program.
There are several breakpoints in the ``kernel_main()`` function,
each of them labeled by a comment.
Compile the project and run MSIM (``make && msim``).
You should hit the first breakpoint labeled ``Still in BARE mode``.

BARE mode
---------

At this point in the program we are using the ``BARE`` translation mode.
Let's make sure of this by displaying the content of the ``satp`` CSR.

.. code:: msim

    [msim] cpu0 csrd satp
    satp (0x180):
    satp 0x00000000 [ Mode: Bare ]

In this mode, no translation is made between virtual and physical addresses.
The same address you use in your code is the one which will be accessed in memory.
There aren't many interesting things happening regarding translation for now,
but you can return here after you learn about the different commands of MSIM
and compare how they behave when using ``BARE`` mode.

Switching to Sv32
-----------------

The line after the first break is where all of the magic happens.
We call the ``set_pagetable(unsigned)`` function where 
the CPU is set up to use our pagetable.
We start by composing the new ``satp`` value to be of the required format,
then we write the value with the ``csrw`` instruction.
Continue the execution of MSIM (type ``continue`` and press Enter),
you'll hit the next breakpoint labeled ``Switched to Sv32``.

Let's try displaying the content of ``satp`` again.

.. code:: msim

    [msim] cpu0 csrd satp
    satp (0x180):
    satp 0x800a0000 [ Mode: Sv32 ASID: 0 PPN: 0x0a0000 (Physical address: 0x0a0000000) ]

A lot more is happening here now.
You can see that we are now using the ``Sv32`` translation scheme,
the current ASID is set to 0,
and the active pagetable sits at physical address ``0xA0000000``.

.. quiz::

    Why is there an extra ``0`` in front of the PPN and physical address?

    .. collapse:: Solution

        The ``Sv32`` translation scheme actually allows for 34-bit physical addresses.
        This means we need 9 hex digits to display the address and 6 digits for the PPN,
        instead of the 8/5 for 32-bit virtual addresses.
        Note that since only 2 bits are used in this added digit, it can at most be equal to ``3``.

What's inside the pagetable
---------------------------

Now that we are using the pagetable, let's diplay its content.
If you have tried to decode the pagetable manually, it's time to check your results.
We can use the ``ptd`` command to dump the currently used pagetable:

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

This command first displays the content of ``satp``
so we can check which pagetable we are working with.
Then it traverses the pagetable, displaying all valid PTEs.
For each non-leaf PTE it descends into the second-level pagetable
and displays its valid PTEs.

Each line thus corresponds to one PTE.
It starts with the address offset of this particular PTE in its page,
then it displays the stored PPN and RSW bits.
It ends with the individual bitfields ``DAGU XWRV``.
A corresponding letter is displayed if this bit is ``1``,
a dash is present instead if this bit is ``0``.
PTEs representing a megapage are denoted as such,
second level PTEs are indented with two spaces.

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

If you want to display invalid PTEs in addition to the valid ones,
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

If you don't want to dump the content of the currently active pagetable,
but would rather specify it by its (physical) address,
you can use the ``sptd`` command.

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

Let's continue the execution of MSIM, some text will get printed to the console,
after which another breakpoint will be hit.

.. quiz::

    Dump the pagetable again, how has it changed?

    .. collapse:: Solution

        The PTE corresponding to the printer device has the ``DA`` bits set now.
        The ``A`` bits shows us that the page as been accessed (either read or written),
        while the ``D`` bit specifies more closely that is has been written to.

We have hit the breakpoint labeled ``After printing to console``,
the ``play_with_memory()`` function will get called when we continue MSIM.
Here the character ``'A'`` is written to some address,
from where it's read back into ``value0``.
A character from another address is read into ``value2``.
Both ``value0`` and ``value2`` get printed, which prints ``'A'`` twice.

.. quiz::

    Where did the ``'A'`` loaded into ``value2`` come from?

    .. collapse:: Hint

        Inspect the second level pagetable.

    .. collapse:: Solution

        The virtual pages staring at ``0xB0000000`` and ``0xB0002000``
        are both mapped to the same physical page starting at ``0xC0000000``.

.. quiz::

    Some code is commented out in this function, try to uncomment it and see what happens.

    Experiment with writing and reading from different addresses in this function.
    You can use the ``char read_from_address(unsigned)``
    and ``void write_to_address(unsigned, char)`` functions.
    How do the ``XWRV`` bits change the behavior?

    Observe how do the ``DA`` bits change, do you notice anything interesting?

    .. collapse:: Solution

        As is required by the specification,
        accessing a page with the ``V`` bit equal to ``0`` will raise a pagefault.
        So will reading a page without the ``R`` permission
        and writing to a page without the ``W`` permission.

        When you read from a page the ``A`` bit gets set
        for the PTE through which this memory has been accessed only.
        This works the same for the ``D`` bit and writing.
        These bits do not change for the other pages which map to the same physical memory.

When that one translation does not work
---------------------------------------

While using virtual memory translation,
you might encounter a situation,
when some address you thought will get translated correctly doesn't
or vice versa. For these cases, MSIM offers the ``tr`` command,
which perform the virtual address translation using the active pagetable
and describes the individual steps it took.

Suppose we have ran our example program up to the
``After printing to console`` labeled breakpoint.
Let's see how the different memory accesses in
``play_with_memory()`` are translated.

.. code:: msim

    [msim] cpu0 tr 0xB0000000
    satp 0x800a0000 [ Mode: Sv32 ASID: 0 PPN: 0x0a0000 (Physical address: 0x0a0000000) ]
    VPN[1]: 0x2c0 VPN[0]: 0x000 page offset: 0x000
    PTE1: [ PPN: 0x0a0001 RSW: 00 ---- ---V ]
      This entry ^ physical address: 0x0a0000b00 = 0x0a0000000 + 0x2c0 * 4
    PTE2: [ PPN: 0x0c0000 RSW: 00 ---- XWRV ]
      This entry ^ physical address: 0x0a0001000 = 0x0a0001000 + 0x000 * 4

    OK: 0xb0000000 => 0x0c0000000

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

    OK: 0xb0002000 => 0x0c0000000

The first line again shows us the content of ``satp``
which is the start point of memory translation.
It then shows how the virtual address gets split into the three parts
``VPN[1]``, ``VPN[0]`` and ``offset``.
The PTEs used for the translation are showed next,
together with their (physical) address.
The last line either describes the successful translation
or displays the reason why the translation failed.
Note that access rights are not taken into account here,
but you can deduce them from the last displayed PTE.

.. quiz::

    Try to dump how an address of some instruction would get translated.
    How does this translation differ from the previous ones?

    .. collapse:: Hint

        Look into ``kernel/kernel.disasm`` and pick an address of any instruction you see.

    .. collapse:: Solution

        The translation is found in the TLB.
        Clear the TLB by executing ``cpu0 tlbflush`` (more on this later) and try again.
        
        How does the translation differ now?

        .. collapse:: Solution 2

            Only one level of the pagetable is used.
            This is because the code is mapped using a megapage.

In addition to ``tr`` MSIM also supports the ``str`` command.
Similarly to ``sptd``, you specify the used pagetable by its physical address.
Also, ``str`` completely ignores the TLB.

.. code:: msim

    [msim] cpu0 str 0xA0000000 0xB0000000
    VPN[1]: 0x2c0 VPN[0]: 0x000 page offset: 0x000
    PTE1: [ PPN: 0x0a0001 RSW: 00 ---- ---V ]
      This entry ^ physical address: 0x0a0000b00 = 0x0a0000000 + 0x2c0 * 4
    PTE2: [ PPN: 0x0c0000 RSW: 00 ---- XWRV ]
      This entry ^ physical address: 0x0a0001000 = 0x0a0001000 + 0x000 * 4

    OK: 0xb0000000 => 0x0c0000000

What even is the TLB?
---------------------

.. quiz:: 

    You know, that is a good question. What even is the TLB?

    .. collapse:: Hint

        TLB stands for **T**\ ranslation **L**\ ookaside **B**\ uffer.

    .. collapse:: Solution

        TLB is a cache used to store virtual translation results.
        It works on the level of pages (either 4 KiB or 4 MiB megapages).
        
        If we were to translate ``0x12345000 => 0x6789A000`` using a pagetable
        (and thus reading twice from memory), we cache that the ``0x12345`` VPN is mapped
        to the ``0x6789A`` PPN. Let's say we want to translate the address ``0x123450F0`` next.
        We start by looking into the TLB and notice, that we have an entry for its VPN.
        We can translate this address without looking inside of the pagetable. 
        We do so and translate it to ``0x6789A0F0``.

        The TLB entries are added automatically to a finite TLB, if there is not a free space for the new
        entry, the **L**\ east **R**\ ecently **U**\ sed entry is evicted.
        The ``sfence.vma`` instruction serves for manual eviction.
        It can either clear the whole TLB or you can use it to evict based on the ASID, virtual address or both.

The size of the RISC-V TLB is configurable in MSIM (using the ``tlbresize`` command),
but using the default count of 48 entries should be reasonable for most applications.

When translating an address the TLB is first searched for an entry
which maps the given virtual address and which is either global
or has the currently active ASID. Thus if you intend to use the same ASID
for different address spaces, you have to carefully flush the TLB.
It's always safe to flush the cache, but in the real world,
you would try to flush out only the problematic entries, so that the TLB
is used as much as possible and the CPU does not have to wait for slow
memory reads.

You can view the content of the TLB with the command ``tlbd``:

.. code:: msim

    [msim] cpu0 tlbd
    TLB    size: 48 entries
       index:       virt => phys        [ info ]
           0: 0x90000000 => 0x090000000 [ ASID: 0, GLOBAL: T, MEGAPAGE: T ]
           1: 0x80000000 => 0x080000000 [ ASID: 0, GLOBAL: T, MEGAPAGE: T ]
           2: 0xb0002000 => 0x0c0000000 [ ASID: 0, GLOBAL: F, MEGAPAGE: F ]
           3: 0xb0000000 => 0x0c0000000 [ ASID: 0, GLOBAL: F, MEGAPAGE: F ]

The entries are dumped in the order of the time they were last used,
the more recent ones being higher up - index ``0`` being the most
and ``47`` the least recently used ones.
The mapping from virtual to physical address is shown
as well as additional information containing the ASID,
whether this entry is global and if it maps a page or megapage.

The TLB can be flushed manually using the ``tlbflush`` command.
This removes all of the entries, behaving the same as the ``sfence.vma``
instruction without any parameters.

If we want to inspect how an address already present in the TLB has been translated
(as we did in one of the excercises), we can first flush the TLB by executing
``tlbflush`` and then dump the translation with ``tr``.

