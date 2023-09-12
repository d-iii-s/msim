RISC-V TLB
==========

To simulate more realistic processors, the Translation Lookaside Buffer (TLB)
is simulated. It caches virtual address translation results.

The TLB is a fully-asociatived cache, with a true Least-Recently-Used
eviction strategy. The cache can be flushed using the standard
``SFENCE.VMA`` instruction (with all of its variants).
If there would be two or more entries that could be applied to the same
virtual address, the most recently used one will be used.
