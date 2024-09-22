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

BARE mode
---------

Switching to Sv32
-----------------

What's inside the pagetable
---------------------------

``ptd``
^^^^^^^

``sptd``
^^^^^^^^

When that one translation does not work
---------------------------------------

``tr``
^^^^^^

``str``
^^^^^^^

What even is the TLB?
---------------------

.. quiz:: 

    That is a good question, what even is the TLB?

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
