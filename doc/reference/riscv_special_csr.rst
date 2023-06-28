Special RISC-V CSR
==================

To support deterministic timer interrupts, there is a non-standard 32-bit
wide CSR named ``scyclecmp``.
When the value in ``mcycle`` (same as value in ``cycle``) is greater than or
equal to the value stored in ``scyclecmp`` a Supervisor Timer Interrupt is
raised.
This interrupt is cleared by setting either of these two CSRs so that
``mcycle`` is greater than scyclecmp.
