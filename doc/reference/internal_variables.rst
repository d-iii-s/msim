Internal variables
==================

MSIM defines several internal variables which control its behaviour
during execution. The variables can be set via the ``set`` and
unset via the ``unset`` commands.

Following internal variables are available:

``trace``
   Enable trace mode
``iaddr``
   Enable addresses in disassembler
``iopc``
   Enable opcodes in disassembler
``icmt``
   Enable additional comments (such as number conversions) in
   disassembler
``iregch``
   Show register changes in disassembler
   Supported only for MIPS R4000
``r4k_ireg``
   Set type of MIPS register names
``r4k_ireg``
   Set type of RISC-V register names

Disassembler configuration
--------------------------

The following table and examples demonstrate how the various variables
affect the fields which are printed in MIPS and RISC-V disassembler.

.. table:: MIPS Instruction disassembling overview

   ============= ============= ============= ======================== ================ ====================
   \             Address       Opcode        Instruction              Conversion       Changes
   ============= ============= ============= ======================== ================ ====================
   Variable      ``iaddr``     ``iopc``      ``r4k_ireg``             ``icmt``         ``iregch``
   Sample output ``80000F04``  ``24840100``  ``addiu a0, a0, 0x100``  ``# 0x100=256``  ``, a0: 0x4->0x104``
   ============= ============= ============= ======================== ================ ====================

.. table:: RISC-V Instruction disassembling overview

   ============= =============== ============== ==================== ====================
   \             Address         Opcode         Instruction          Conversion
   ============= =============== ============== ==================== ====================
   Variable      ``iaddr``       ``iopc``       ``rv_ireg``          ``icmt``
   Sample output ``0xf0000008``  ``00b50633``   ``add a2, a0, a1``   ``[ a2 = a0 + a1 ]``
   ============= =============== ============== ==================== ====================

Addresses in disassembler:

.. code:: msim

   [msim] unset iaddr
   [msim] id 0x0040121c 1
         lw    a0, 0x1694(gp)    # 0x1694=5780
   [msim] set iaddr
   [msim] id 0x0040121c 1
       0040121C    lw    a0, 0x1694(gp)    # 0x1694=5780
   [msim]

Opcodes in disassembler:

.. code:: msim

   [msim] unset iopc
   [msim] id 0x0040121c 1
       0040121C    lw    a0, 0x1694(gp)    # 0x1694=5780
   [msim] set iopc
   [msim] id 0x0040121c 1
       0040121C  8F841694    lw    a0, 0x1694(gp)    # 0x1694=5780
   [msim]

Additional disassembler information (e.g. hex to decimal number
conversions):

.. code:: msim

   [msim] unset icmt
   [msim] id 0x00400ef8 1
       00400EF8    sw    a0, 0x1694(gp)
   [msim] set icmt
   [msim] id 0x00400ef8 1
       00400EF8    sw    a0, 0x1694(gp)    # 0x1694=5780
   [msim]

Register changes in disassembler:

.. code:: msim

   [msim] unset iregch
   [msim] s 5
       0  80400ECC    addu  v0, v0, a1
       0  80400ED0    sll   v0, v0, 0x06
       0  80400ED4    subu  v0, v0, a1
       0  80400ED8    sll   v1, v0, 0x02
       0  80400EDC    addu  v0, v0, v1
   [msim] set iregch
   [msim] s 5
       0  80400EE0    sll   v0, v0, 0x04      # v0: 0xe12018d9->0xe83550,
                                              # v1: 0xe03fd900->0xb9c44,
                                              # a0: 0x6d9b->0xe9e3a6c0,
                                              # a1: 0xa9->0xaf, a2: 0xda->0xdb,
                                              # loreg: 0x394745fa->0x7844ddc0,
                                              # hireg: 0x7116dba5->0x7544c9eb
       0  80400EE4    addu  v0, v0, a1        # v0: 0xe83550->0xe835ff
       0  80400EE8    sll   v1, v0, 0x08      # v1: 0xb9c44->0xe835ff00
       0  80400EEC    addu  v0, v0, v1        # v0: 0xe835ff->0xe91e34ff
       0  80400EF0    subu  a0, a0, v0        # a0: 0xe9e3a6c0->0xc571c1
   [msim]

Register naming schemes
-----------------------

The ``r4k_ireg`` and ``rv_ireg`` selects the scheme for register names
used by the disassembler:

.. csv-table:: Basic MIPS processor registers
   :header: "``r4k_ireg``", "Description", "List of register names"

   0, "Processor-oriented", "r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 r11 r12 r13 r14 r15 r16 r17 r18 r19 r20 r21 r22 r23 r24 r25 r26 r27 r28 r29 r30 r31"
   1, "AT&T assembler", "$0 $1 $2 $3 $4 $5 $6 $7 $8 $9 $10 $11 $12 $13 $14 $15 $16 $17 $18 $19 $20 $21 $22 $23 $24 $25 $26 $27 $28 $29 $30 $31"
   2, "Compiler convention", "0 at v0 v1 a0 a1 a2 a3 t0 t1 t2 t3 t4 t5 t6 t7 s0 s1 s2 s3 s4 s5 s6 s7 t8 t9 k0 k1 gp sp fp ra"


.. csv-table:: MIPS CP0 registers
   :header: "``r4k_ireg``", "Description", "List of register names"

   0, "Processor-oriented", "r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 r11 r12 r13 r14 r15 r16 r17 r18 r19 r20 r21 r22 r23 r24 r25 r26 r27 r28 r29 r30 r31"
   1, "AT&T assembler", "$0 $1 $2 $3 $4 $5 $6 $7 $8 $9 $10 $11 $12 $13 $14 $15 $16 $17 $18 $19 $20 $21 $22 $23 $24 $25 $26 $27 $28 $29 $30 $31"
   2, "Compiler convention", "index random entrylo0 entrylo1 context pagemask wired res_7 badvaddr count entryhi compare status cause epc prid config lladdr watchlo watchhi xcontext res_21 res_22 res_23 res_24 res_25 res_26 res_27 res_28 res_29 errorepc res_31"


.. csv-table:: Basic RISC-V processor registers
   :header: "``rv_ireg``", "Description", "List of register names"

   0, "Numerical names", "x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 x10 x11 x12 x13 x14 x15 x16 x17 x18 x19 x20 x21 x22 x23 x24 x25 x26 x27 x28 x29 x30 x31"
   1, "ABI names", "zero ra sp gp tp t0 t1 t2 s0/fp s1 a0 a1 a2 a3 a4 a5 a6 a7 s2 s3 s4 s5 s6 s7 s8 s9 s10 s11 t3 t4 t5 t6"


Sample of usage:

.. code:: msim

   [msim] set r4k_ireg=0
   [msim] id 0x00400efc 4
       00400EFC    srl   r4, r4, 0x08
       00400F00    andi  r4, r4, 0x000f    # 0xfh=15
       00400F04    addiu r4, r4, 0x100     # 0x100=256
       00400F08    sltu  r4, r6, r4
   [msim] set r4k_ireg=1
   [msim] id 0x00400efc 4
       00400EFC    srl   $4, $4, 0x08
       00400F00    andi  $4, $4, 0x000f    # 0xfh=15
       00400F04    addiu $4, $4, 0x100     # 0x100=256
       00400F08    sltu  $4, $6, $4
   [msim] set r4k_ireg=2
   [msim] id 0x00400efc 4
       00400EFC    srl   a0, a0, 0x08
       00400F00    andi  a0, a0, 0x000f    # 0xfh=15
       00400F04    addiu a0, a0, 0x100     # 0x100=256
       00400F08    sltu  a0, a2, a0
   [msim]


Simulation trace
----------------

The ``trace`` variable switches the simulator into the trace mode. In
trace mode, all the executed instructions are disassembled and
immediately displayed on the screen.

.. code:: msim

   [msim] unset trace
   [msim] s
   [msim] s
   [msim] s
   [msim] set trace
   [msim] s
       0  80400EC8    sll   v0, a1, 0x04      # v0: 0x1fcec0c2->0x2f0,
                                             # a1: 0x1ffc0227->0x2f
   [msim] s
       0  80400ECC    addu  v0, v0, a1        # v0: 0x2f0->0x31f
   [msim] s
       0  80400ED0    sll   v0, v0, 0x06      # v0: 0x31f->0xc7c0
   [msim]
