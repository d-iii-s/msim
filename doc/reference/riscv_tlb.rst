RISC-V TLB
==========

To simulate more realistic processors, the Translation Lookaside Buffer (TLB)
is simulated.
It caches virtual address translation results.
The tlb is split into two parts: Kilo-TLB and Mega-TLB.

The Kilo-TLB (KTLB) caches translations of 4 KiB pages and the Mega-TLB (MTLB)
caches translations of 4 MiB megapages.
For more information about the virtual translation process,
see the Sv32 translation scheme definition in the RISC-V privileged
specification <https://riscv.org/technical/specifications/> .

The TLB is a directly mapped cache, which is tagged by the ASID
that has been used at the time the translation has been added to the cache.
The cache can be flushed using the standard ``SFENCE.VMA`` instruction.
If there would be two entries that could be applied to the same virtual address
(one in KTLB and one in MTLB, both of same ASID as the current one)
the entry from the MTLB will be used.
