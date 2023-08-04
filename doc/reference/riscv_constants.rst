RISC-V pre-defined constants
============================

Memory-mapped registers
-----------------------

mtime
   ``0xFF000000``
   64-bit
mtimecmp
   ``0xFF000008``
   64-bit

Start address (reset vector)
----------------------------

``0xF0000000``

Custom CSR numbers
------------------

``scyclecmp``
   ``0x5C0``

HPM counter events
------------------

no event
   0
User-level cycles
   1
   Cycles that the CPU has ended in U-mode
Supervisor-level cycles
   2
   Cycles that the CPU has ended in S-mode
Machine-level cycles
   4
   Cycles that the CPU has ended in M-mode
Waiting cycles
   5
   Cycles that the CPU has spent idling

Default TLB size
----------------

Kilo-TLB - 256 entries
Mega-TLB - 32 entries
