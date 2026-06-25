Devices
=======

MSIM can be configured with any number of device instances.
Every device instance has a given type.
This section describes the available types of devices and their properties.

.. contents:: Overview
   :local:
   :depth: 1



MIPS Processor ``dr4kcpu``
--------------------------

The ``dr4kcpu`` device encapsulates a MIPS R4000 processor.

Initialization parameters: none
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Commands
^^^^^^^^

``help [cmd]``
   Display a help text for the specified command or a list of available commands.
``info``
   Display the processor configuration
``stat``
   Display processor statistics
``cp0d [rn]``
   Dump contents of CP0 register(s)
``tlbd``
   Dump the content of TLB
``md saddr size``
   Dump specified TLB mapped memory block
``id sa cnt``
   Dump instructions from specified TLB mapped memory
``rd``
   Dump contents of CPU general registers
``goto addr``
   Go to address
``break addr``
   Add code breakpoint
``bd``
   Dump configured code breakpoints
``br addr``
   Remove configured code breakpoint

Examples
^^^^^^^^

Example of the ``help`` command:

.. code:: msim

   [msim] add dr4kcpu mips1
   [msim] mips1 help
   Command & arguments  Description
   -------------------- -------------------->
   help [cmd]           Display this help text
   info                 Display configuration information
   stat                 Display processor statistics
   cp0d [rn]            Dump contents of the coprocessor 0 register(s)
   tlbd                 Dump content of TLB
   md saddr size        Dump specified TLB mapped memory block
   id saddr cnt         Dump instructions from specified TLB mapped memory
   rd                   Dump contents of CPU general registers
   goto addr            Go to address
   break addr           Add code breakpoint
   bd                   Dump code breakpoints
   br addr              Remove code breakpoint
   [msim]

Example of the ``info`` command:

.. code:: msim

   [msim] mips1 info
   type:R4000.32
   [msim]

Example of the ``stat`` command:

.. code:: msim

   [msim] mips1 stat
   cycles total:853037 in kernel:853037 in user:0 in stdby:0 tlb refill:0
                         invalid: 0 modified:0
                         interrupts 0:0 1:0 2:0 3:0 4:0 5:0 6:0 7:0
   [msim]

Example of the ``cp0d`` command:

.. code:: msim

   [msim] mips1 cp0d
     no name       hex dump  readable dump
     00 Index      0000002F  index: 2F res: 0 p: 0
     01 Random     00000003  random: 03, res: 0000000
     02 EntryLo0   00000006  g: 0 v: 1 d: 1 c: 0 pfn: 000000 res: 0
     03 EntryLo1   00000000  g: 0 v: 0 d: 0 c: 0 pfn: 000000 res: 0
     04 Context    00000000  res: 0 badvpn2: 00000 ptebase: 000
     05 PageMask   001FE000  res1: 0000 mask: 0FF (1M) res2: 00
     06 Wired      00000001  wired: 1 res: 0000000
     08 BadVAddr   00000000  badvaddr: 00000000
     09 Count      0005A993  count: 5a993
     0a EntryHi    00000000  asid: 00 res: 0 vpn2: 00000
     0b Compare    0005ACDC  compare: 5acdc
     0c Status     00008001  ie: 1 exl: 0 erl: 0 ksu: 0 ux: 0 sx: 0 kx: 0
                             im: 80 de: 0 ce: 0 ch: 0 res1: 0 sr: 0 ts: 0
                             bev: 0 res2: 0 re: 0 fr: 0 rp: 0 cu: 0
     0d Cause      00000000  res1: 0 exccode: 00 res2: 0 ip: 00 res3: 00
                             ce: 0 res4: 0 bd: 0
     0e EPC        80401344  epc: 80401344
     0f PRId       00000200  rev: 00 imp: 02 res: 0000
     10 Config     00000000  k0: 0 cu: 0 db: 0 b: 0 dc: 0 ic: 0 res: 0 eb: 0
                             em: 0 be: 0 sm: 0 sc: 0 ew: 0 sw: 0 ss: 0 sb: 0
                             ep: 0 ec: 0 cm: 0
     11 LLAddr     00000000  lladdr: 00000000
     12 WatchLo    00000000  w: 0 r: 0 res: 0 paddr0: 00000000
     13 WatchHi    00000000  res: 00000000 paddr1: 0
     14 XContext
     1e ErrorEPC   00000000  errorepc: 00000000
   [msim]

Example of the ``tlbd`` command:

.. code:: msim

   [msim] mips1 tlbd
    [             general             ][    subp 0    ][    subp 1    ]
     no    vpn      mask        g asid  v d   pfn    c  v d   pfn    c
     00  00000000 FFE00000:1M   0  00   1 1 00000000 0  0 0 00000000 0
     01  00000000 FFE00000:1M   0  ff   1 1 00000000 0  0 0 00000000 0
   [msim]

Example of the ``rd`` command:

.. code:: msim

   [msim] mips1 rd
   processor p0
      0 00000000   at 81000000   v0 62935B2A   v1 62312A00   a0 00000001
     a1 0000004A   a2 0000002E   a3 0000002D   t0 00000000   t1 00000000
     t2 00000000   t3 00000000   t4 00000000   t5 00000000   t6 00000000
     t7 00000000   s0 000D5427   s1 81000000   s2 805ACE55   s3 00000000
     s4 00000000   s5 00000000   s6 00000000   s7 00000000   t8 00000000
     t9 00000000   k0 00000000   k1 00000000   gp 00000000   sp 00013050
     fp 00000000   ra 80401308   pc 80401310   lo 40689065   hi 320BBCB7
   [msim]




32-bit RISC-V Processor ``drvcpu``
----------------------------------

The ``drvcpu`` device encapsulates a RISC-V RV32IMA processor.

Initialization parameters: none
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Commands
^^^^^^^^

``help [cmd]``
   Display a help text for the specified command or a list of available commands.
``info``
   Display the processor configuration
``rd``
   Dump contents of CPU general registers
``csrd [name|number|subcommand]``
   Dump the contents of the specified CSR or some CSRs.
      Dumps contents of predefined CSRs without a parameter.
      Specifying a CSR number or name dump the selected CSR.
      Selecting a subcommand prints one of the following subsets of CSRs:

      - ``mmode`` - dumps all M-mode CSRs
      - ``smode`` - dumps all S-mode CSRs
      - ``counters`` - dumps all counter and counter setup CSRs
      - ``all`` - dumps all CSRs
``tr <addr>``
   Translates the specified virtual address based on the current CPU state and describes the translation process.
      PTEs are dumped in the following format: ``PTE: [ PPN: <ppn> RSW: <rsw> DAGU XWRV]``,
      where ``DAGU`` and ``XWRV`` are either these letters when the corresponding bits are set (i.e. equal to 1),
      or ``-`` when the corresponding bits are clear (i.e. equal to 0).
      Execute ``tlbflush`` before this command when using the TLB for the translation is not desired (but note that flushing the TLB can change the behavior of the simulated program.)
``str <root phys> <addr>``
   Translates the specified virtual address using the pagetable specified by the physical address of its root and describes the translation process.
      Output format is the same as for ``tr``.
      The translation does not use the TLB and ignores the CPU state.
      Note that the root physical address has to be aligned to the size of a page (``4096``).
``ptd [verbose|v]``
   Prints out all valid PTEs in the pagetable currently pointed to by the satp CSR.
   Adding the ``verbose`` parameter (or simply ``v``) prints out all nonzero PTEs.
``sptd <root phys> [verbose|v]``
   Prints out all valid PTEs in the pagetable with its root pagetable located at ``phys`` (physical address).
   Note that this address has to be aligned to the size of a page (``4096``).
   Adding the ``verbose`` parameter (or simply ``v``) prints out all nonzero PTEs.
``tlbd``
   Dump the contents of the TLB, split by page size.
``tlbresize <size>``
   Resize the TLB by specifying its new size.
``tlbflush``
   Removes all entries from the TLB.
``asidlen <length>``
   Changes the bit-length of ASIDs.

Examples
^^^^^^^^

Example of the ``rd`` command:

.. code:: msim

   [msim] risc1 rd
   processor 0
     zero:        0    ra:        0    sp:        0    gp:        0
       tp:        0    t0:        0    t1:        0    t2:        0
    s0/fp:        0    s1:        0    a0:        0    a1:        0
       a2:        0    a3:        0    a4:        0    a5:        0
       a6:        0    a7:        0    s2:        0    s3:        0
       s4:        0    s5:        0    s6:        0    s7:        0
       s8:        0    s9:        0   s10:        0   s11:        0
       t3:        0    t4:        0    t5:        0    t6:        0
       pc: f0000000
   [msim]

Example of the ``csrd`` command:

.. code:: msim

   [msim] risc1 csrd sstatus
   sstatus (0x100):
   sstatus 0x00000000 [ SD 0, MXR 0, SUM 0, XS 00, FS 00, VS 00, SPP U, UBE 0, SPIE 0, SIE 0 ]
   [msim]


Example of the ``tlbd`` command:

.. code:: msim

   [msim] risc1 tlbd
   TLB    size: 48 entries    Entries shown in LRU order.
      index:       virt => phys        [ info ]
          0: 0x00400000 => 0x000000000 [ ASID: 0, GLOBAL: F, MEGAPAGE: T ]
          1: 0xf0000000 => 0x0f0000000 [ ASID: 1, GLOBAL: T, MEGAPAGE: F ]
          2: 0x00400000 => 0x000400000 [ ASID: 2, GLOBAL: F, MEGAPAGE: T ]
   [msim]


64-bit RISC-V Processor ``drv64cpu``
------------------------------------

The ``drv64cpu`` device encapsulates a RISC-V RV64IMA processor.

Initialization parameters: none
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Commands
^^^^^^^^

The command and behavior is virtually identical to ``drvcpu`` but the CPU is
emulating a 64-bit RISC-V processor.

SuperH SH-2E Processor ``dsh2ecpu``
-----------------------------------

The ``dsh2ecpu`` device encapsulates a SuperH SH-2E processor.
The device supports a system on-chip (SoC) configuration with other SH-2E peripheral devices (``dsh2edmac``, ``dsh2ecmt`` and ``dsh2ewdt``) but it can be used without them as well.
By adding peripheral devices to the CPU, it is possible to inform the attached devices about interrupts and resets so they can update their state accordingly.

Initialization parameters: none
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Commands
^^^^^^^^

``help [cmd]``
   Display a help text for the specified command or a list of available commands.
``info``
   Display the processor configuration
``md saddr count``
   Dump specified physical memory block
``id saddr count``
   Dump instructions from physical memory location
``rd``
   Dump contents of CPU general registers
``frd``
   Dump contents of FPU registers
``goto addr``
   Go to address
``assertint intno``
   Assert an interrupt (the interrupt number needs to be configured in the configuration file)
``setintc intc_name``
   Assign the interrupt controller to the CPU
``addperipheral dev_name``
   Add a peripheral device to the CPU (currently supported only for the ``dsh2edmac``, ``dsh2ecmt`` and ``dsh2ewdt`` devices)

Examples
^^^^^^^^

Example of the ``help`` command:

.. code:: msim

   [msim] sh2e1 help
   [Command and arguments       ] [Description
   help [<cmd>]                   Display this help text
   info                           Display configuration information
   md <saddr> <size>              Dump specified physical memory block
   id <saddr> <cnt>               Dump instructions from physical memory location
   rd                             Dump contents of CPU registers
   frd                            Dump contents of FPU registers
   goto <addr>                    Go to address
   stat                           Display CPU statistics
   assertint <interrupt_source>   Assert interrupt
   setintc <intc_device_name>     Set interrupt controller
   addperipheral <device_name>    Add a peripheral to the CPU's on-chip peripherals
   [msim]

Example of the ``info`` command:

.. code:: msim

   [msim] sh2e1 info
   SH-2E
   [msim]

Example of the ``stat`` command:

.. code:: msim

   [msim] sh2e1 stat
   [Total cycles            ] [Program execution cycles] [Power down cycles       ]
                        11002                      11002                          0

   [msim]

Example of the ``md`` command:

.. code:: msim

   [msim] sh2e1 md 0x400 10
     0x000400    d001400b 00098200 00000de4 2fe67ff0
     0x000410    6ef361e3 71d0114e 625361e3 71d0116c
     0x000420    61e36023 801461e3
   [msim]

Example of the ``id`` command:

.. code:: text

   [msim] sh2e1 id 0x400 10
   cpu0   00000400  D001  mov.l   @(0x00000408), r0                    ; [PC + ZE(disp) × 4] → Rn
   cpu0   00000402  400B  jsr     @r0                                  ; PC + 4 → PR, Rm → PC (delayed)
   cpu0   00000404  0009  nop                                          ; (no operation)
   cpu0   00000406  8200  halt                                         ; (halt machine)
   cpu0   00000408  0000  <ILLEGAL>
   cpu0   0000040A  0DE4  mov.b   r14, @(r0, r13)                      ; Rm{7:0} → [R0 + Rn]
   cpu0   0000040C  2FE6  mov.l   r14, @-sp                            ; Rn - 4 → Rn, Rm → [Rn]
   cpu0   0000040E  7FF0  add     -16, sp                              ; Rn + SE(#imm) → Rn
   cpu0   00000410  6EF3  mov     sp, r14                              ; Rm → Rn
   cpu0   00000412  61E3  mov     r14, r1                              ; Rm → Rn
   [msim]

Example of the ``rd`` command:

.. code:: msim

   [msim] sh2e1 rd
   processor 0
       r0:       65    r1: ffff9fc8    r2: 90000000    r3:        0
       r4:       65    r5:        0    r6:        0    r7:        0
       r8:        0    r9:        0   r10:        0   r11:        0
      r12:        0   r13:        0   r14: ffff9fc8    sp: ffff9fc8
       pc:      48e    pr:      4da  mach:        0  macl:        0
       sr:       f0   gbr:        0   vbr:        0
   [msim]

Example of the ``frd`` command:

.. code:: msim

   [msim] sh2e1 frd
   processor 0
      fr0: +3.5           fr1: -3.5           fr2: -8.75          fr3: +0
      fr4: +0             fr5: +0             fr6: +0             fr7: +0
      fr8: +0             fr9: +0            fr10: +0            fr11: +0
     fr12: +0            fr13: +0            fr14: +0            fr15: +0
     fpul: nan           fpul: ffffffff     fpscr: 40001
   [msim]

Example of the ``addperipheral`` command:

.. code:: msim

   [msim] add dsh2ecpu cpu0
   [msim] add dsh2ewdt wdt
   [msim] cpu0 addperipheral wdt


Read/write memory ``rwm``
-------------------------

The ``rwm`` device represents a read/write random access memory region.
The memory block could be a general memory or mapped from a file.

Initialization parameters: ``address``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``address``
   Starting physical address of the memory block.

Commands
^^^^^^^^

``help [cmd]``
   Print a help on the command specified of a list of available commands.
``info``
   Print the device information (block address, size and type)
``stat``
   Print device statistics (none).
``generic size``
   Set the size of the memory block.
``fmap filename``
   Map the contents of the memory block from a file specified.
``fill [value]``
   Fill the memory block with zeros or the specified word value.
``load filename``
   Load the contents of the memory block from a file specified.
``save filename``
   Save the contents of the memory block to a file specified.

Examples
^^^^^^^^

Example of the ``info`` command.

.. code:: msim

   [msim] startmem info
   start:0x1fc00000 size:1k type:mem
   [msim]

In the following example, the ``add`` command creates a read/write memory
region ``mx`` starting at the physical address 0x00001000.
The size of the region is set to 256 KB via the ``generic`` command.

.. code:: msim

   [msim] add rwm mx 0x1000
   [msim] mx generic 256k
   [msim]

The content of the read-write memory can be changed as expected from the code
running in the simulator:

.. code:: c

    1 volatile char *p = (char * ) 0x1000;  /* assume 1:1 identity memory mapping */
    2 char c = *p;  /* c contains a byte value read from the address 0x00001000 */
    3 *p += 1;
    4 char d = *p;  /* d contains a byte value read from the address 0x00001001 */




Read-only memory ``rom``
------------------------

The ``rom`` device represents a read-only random access memory region.

Initialization parameters: ``address``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``address``
   Starting physical address of the memory block.

Commands
^^^^^^^^

``help [cmd]``
   Print a help on the command specified or a list of available commands.
``info``
   Print the device information (block address, size and type)
``stat``
   Print device statistics (none).
``generic size``
   Set the size of the memory block.
``fmap filename``
   Map the contents of the memory block from a file specified.
``fill [value]``
   Fill the memory block with zeros or the specified word value.
``load filename``
   Load the contents of the memory block from a file specified.
``save filename``
   Save the contents of the memory block to a file specified.


Examples
^^^^^^^^

In the following example, the ``add`` command creates a read-only memory
region ``mx`` starting at the physical address 0x00001000.
The size of the region is set to 256 KB via the ``generic`` command.

.. code:: msim

   [msim] add rom mx 0x1000
   [msim] mx generic 256k
   [msim]

All attempts to write into read-only memory are silently ignored.

.. code:: c

    1 volatile char *p = (char * ) 0x1000;  /* assume 1:1 identity memory mapping */
    2 char c = *p;  /* c contains a byte value read from the address 0x00001000 */
    3 *p = ~c;      /* write access is ignored */
    4 char d = *p;  /* c == d */




No-access memory ``dnomem``
---------------------------

The ``dnomem`` device represents a memory region where any reads or writes
terminates with simulation failure.

The device is meant as a debugging aid to ensure our code is not accessing
non-existent memory.

The device operates in three modes. In the default ``warn`` mode, attempts
to access are printed to console but the simulation continues.
In ``break`` mode, the simulation is switched to interactive mode after each
access.
And in ``halt`` mode, the simulation is immediately terminated.

It is possible to dump registers of the CPU that caused the violation
automatically by setting ``rd yes``.


Initialization parameters: ``address`` ``size``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``address``
   Starting physical address of the memory block.
``size``
   Memory size.

Commands
^^^^^^^^

``help [cmd]``
   Print a help on the command specified or a list of available commands.
``info``
   Print the device information (block address, size and mode)
``mode``
   Set device mode (either ``warn``, ``halt`` or ``break``).
``rd``
   Whether to dump registers on violation (``yes`` or ``no``).


Examples
^^^^^^^^

In the following example, the ``add`` command creates a protected memory
region ``nomem`` starting at the physical address 0x08000000.
The size of the region is set to 4 KB.

.. code:: msim

   [msim] add dnomem nomem 0x08000000 0x1000
   [msim]

All attempts to read/write into this memory are reported.

.. code:: asm

    la $a0, 0x88000004
    lw $a1, 0($a0)
    /* <msim> Alert: Ignoring READ (at 0x008000004, 0x4 inside nomem). */

If we configure the device to ``mode`` ``halt``, the simulation is halted upon
reaching the ``lw`` instruction. By using ``nomem rd yes``, registers are
dumped before the machine is halted.

.. code:: msim

   [msim] nomem rd yes
   [msim] nomem mode halt
   [msim]
   # la $a0, 0x88000004
   # lw $a1, 0($a0)
   # <msim> Alert: Halting after forbidden READ (at 0x008000004, 0x4 inside nomem).




Character output device ``dprinter``
------------------------------------

The character output device simulates a simple character printer or a serial console.

Initialization parameters: ``address``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``address``
   Physical address of the printer register

Registers
^^^^^^^^^

.. csv-table:: ``dprinter`` programming registers
   :header: Offset, Size, Name, Operation, Description

   "+0",4,character,read,"(ignored)"
   ,,,write,"Character to be printed on the standard output of MSIM"



Commands
^^^^^^^^

``help [cmd]``
   Print a help text to the specified command or a list of allowed commands.
``info``
   Print basic configuration information (register address).
``stat``
   Print printer statistics (number of characters printed).
``redir filename``
   Redirect the output to the file specified.
``stdout``
   Redirect the output to the standard output.
``endian``
   Change the endianness of the device.

Example
^^^^^^^

Example of the ``info`` command:

.. code:: msim

   [msim] add dprinter printer 0x10000000
   [msim] printer info
   [address  ] [endianness]
    0x10000000       little
   [msim]

Example of the ``stat`` command:

.. code:: msim

   [msim] printer stat
   [count             ]
                  11385
   [msim]

Example of the ``endian`` command:

.. code:: msim

   [msim] add dprinter printer 0x10000000
   [msim] printer endian big
   [msim] printer info
   [address  ] [endianness]
    0x10000000          big
   [msim]

Example of a simple output implementation in the MIPS code:

.. code:: c

    /*
     * VIDEOADDR is the address of the memory-mapped register of dprinter.
     * It has to correspond with the definition in the configuration file.
     *
     * Configuration file always contains physical addresses as we are
     * configuring the hardware. Here we need to have virtual address
     * as C code cannot access physical addresses directly.
     */
    #define VIDEOADDR 0x90000000

    /* Write one character on the screen. */
    void putchar(char c) {
        *((volatile char *) VIDEOADDR) = c;
    }

    /* Write a NULL-terminated string on the screen. */
    void putstring(char* c) {
        while (*c) {
            *((volatile char *) VIDEOADDR) = *c;
            c++;
        }
    }




Character input device ``dkeyboard``
------------------------------------

The character input device simulates a keyboard connected to the machine.
When a key is pressed, the keyboard asserts an interrupt and the ASCII key
code can be read from the memory-mapped register.
Any read operation on the register automatically deasserts the pending
interrupt.

Initialization parameters: ``address`` ``intno``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``address``
   Physical address of the keyboard register.
``intno``
   Interrupt number which will be asserted on keypress.

Registers
^^^^^^^^^

.. csv-table:: ``dkeyboard`` programming registers
   :header: Offset, Size, Name, Operation, Description

   "+0",4,keycode,read,"Key code of the pressed key (any read operation deasserts the pending interrupt)"
   ,,,write,"(ignored)"

Commands
^^^^^^^^

``help [cmd]``
   Print a help on the command specified or a list of available commands.
``info``
   Print configuration information (register address, interrupt number and a keycode pending).
``stat``
   Print device statistics (number of interrupts, pressed keys and overrun keys).
``gen keycode``
   Synthetically generates a key press event.


Examples
^^^^^^^^

The following command adds a keyboard ``kb`` to the machine.
The keyboard register is mapped physical address 0x10000000.

.. code:: msim

   [msim] add dkeyboard kb 0x1000000
   [msim]

Example of the ``info`` command:

.. code:: msim

   [msim] kb info
   address:0x10000000 intno:3 regs(key:0x00 ig:0)
   [msim]

Example of the ``stat`` command.

.. code:: msim

   [msim] kb stat
   intrc:11 keycount:11 overrun:0
   [msim]




Block device ``ddisk``
----------------------

The block device simulates a simple hard disk with DMA capabilities.
The device can be accesses by linearly ordered 512 B-sized sectors.
The device allows to access the contents of a file from the host system
running MSIM.

Initialization parameters: ``address`` ``intno``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``address``
   Physical address of the hard disk register.
``intno``
   DMA Interrupt number.
``cpuname``
   Name of the CPU device to which interrupts will be sent.

Registers
^^^^^^^^^

.. csv-table:: ``ddisk`` programming registers
    :header: Offset, Size, Name, Operation, Description
    :widths: auto

    "+0",4,"DMA buffer address (lower 32 bits)",read,"
    Get the current physical address of the DMA buffer (lower 32 bits)
    "
    ,,,write,"
    Set the physical address of the DMA buffer (lower 32 bits).

    * performing a read operation will store the data read from the block device to this buffer
    * performing a write operation will fetch the data to be written to the block device from this buffer
    "
    "+4",4,"sector number",read,"
    Get the current sector number
    "
    ,,,write,"
    Set the sector number which be used by the next **read** or **write** operation
    "
    "+8",4,"status/command",read,"
    Get a bitfield representing the current status of the device:

    .. csv-table::
        :header: 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5,4,3,2,1 0

        r1,b,e,i,r0

    ``r0``, ``r1``
        (reserved)
    ``b``
        0: the device is not busy
        1: the device is busy
    ``e``
        0: no error
        1: the previous operation caused an error
    ``i``
        0: no DMA interrupt pending
        1: DMA interrupt pending
    "
    ,,,write,"
    Set a bitfield representing requested operation:

    .. csv-table::
        :header: 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3,2,1,0

        r,i,w,r

    ``r0``
        (reserved)
    ``i``
        if set to 1 then the DMA interrupt is deasserted
    ``w``
        if set to 1 then a write operation is initiated
        (fetching the data from the DMA buffer and writing it to the sector set)
    ``r``
        if set to 1 then a read operation is initiated
        (reading the data from the sector set and storing it to the DMA buffer)

    initiating both read and write operation at the same time
    results with an error
    "
    "+12",4,"disk size (lower 32 bits)",read,"
    Get the size of the block device in bytes (lower 32 bits)
    "
    ,,,write,"(ignored)"
    "+16",4,"DMA buffer address (higher 4 bits)",read/write,"
    Higher 4 bits for DMA physical address.
    See description of **DMA buffer address (lower 32 bits)** for further details.
    "
    "+20",4,reserved,read/write,"
    (Reserved for future extensions)
    "
    "+24",4,"disk size (higher 32 bits)",read/write,"
    Higher 32 bits of block device size in bytes.
    See description of **disk size (lower 32 bits)** for further details.
    "

Commands
^^^^^^^^

``help [cmd]``
   Print a help on the command specified or a list of available commands.
``info``
   Print the device information.
``stat``
   Print device statistics.
``generic size``
   Allocate a block device of the given size from host memory.
``fmap name``
   Map the block device to a file specified.
``fill [value]``
   Fill the block device with zeros or the specified word value.
``load fname``
   Load the contents of the block device from a file specified.
``save fname``
   Save the contents of the block device to a file specified.




Interprocessor communication device ``dorder``
----------------------------------------------

This device allows to obtain a processor serial number in a multiprocessor
configuration and asserting an interprocessor interrupt on a specified processor.

Initialization parameters: ``address`` ``intno``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``address``
   Physical address of the device register.
``intno``
   Interprocessor communication interrupt number.

Registers
^^^^^^^^^

.. csv-table:: ``dorder`` programming registers
    :header: Offset, Size, Name, Operation, Description
    :widths: auto

    +0,4,processor number,read,"
    Get the unique number of the processor performing the read operation
    "
    ,,interrupt up,write,"
    Setting any bit causes an interrupt pending on the processor specified by
    the bit index
    "
    +4,4,interrupt down,read,"(ignored)"
    ,,,write,"
    Setting any bit acknowledges an interrupt pending on the processor
    specified by the bit index (the interrupt is deasserted)
    "

Commands
^^^^^^^^

``help [cmd]``
   Print a help on the command specified or a list of available commands.
``info``
   Print configuration information (register address and interrupt number).
``stat``
   Print device statistics (number of interrupts).
``synchup mask``
   Simulate a write operation on the "interrupt up" register.
``synchdown mask``
   Simulate a write operation on the "interrupt down" register.

.. _examples-5:

Examples
^^^^^^^^

The following example adds a new ``dorder`` device named ``order`` and
maps its register on the physical address 0x10000000.

.. code:: msim

   [msim] add dorder order 0x1000000
   [msim]

Simple usage in MIPS code:

.. code:: c

    /* ORDER is the address of the memory-mapped register of dorder.
     * It has to correspond with the definition in the configuration file.
     */
    #define ORDER 0x90000000

    /* Get current CPU identification. */
    unsigned int cpu_id(void) {
        return *((volatile unsigned int *) ORDER);
    }

    /* Send interprocessor interrupt to the given CPU */
    void cpu_send_ipi(unsigned int id) {
        *((volatile unsigned int *) ORDER) = (1 << id);
    }

    /* Send interprocessor interrupt to all CPUs */
    void cpu_send_ipi_all(void) {
        *((volatile unsigned int *) ORDER) = 0xffffffff;
    }

    /* Deassert interprocessor interrupt for the given CPU */
    void cpu_ack_ipi(unsigned int id) {
        *((volatile unsigned int *) (ORDER + 4)) = (1 << id);
    }




Real-time clock ``dtime``
-------------------------

This device passes the system time of the host machine to the simulated environment.

Initialization parameters: ``address``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``address``
   Physical address of the device register.

Registers
^^^^^^^^^

.. table:: ``dtime`` programming registers

   +-----------------------+-----------------------+-----------------------+-----------------------+-----------------------+
   | Offset                | Size                  | Name                  | Operation             | Description           |
   +=======================+=======================+=======================+=======================+=======================+
   | +0                    | 4                     | seconds               | read                  | number of seconds     |
   |                       |                       |                       |                       | since the epoch,      |
   |                       |                       |                       |                       | i. e. 00:00:00        |
   |                       |                       |                       |                       | January 1st 1970 UTC  |
   |                       |                       |                       |                       | (equivalent to the    |
   |                       |                       |                       |                       | ``tv_sec`` field      |
   |                       |                       |                       |                       | returned by POSIX     |
   |                       |                       |                       |                       | ``gettimeofday()``    |
   |                       |                       |                       |                       | function)             |
   +-----------------------+-----------------------+-----------------------+-----------------------+-----------------------+
   |                       |                       |                       | write                 | (ignored)             |
   +-----------------------+-----------------------+-----------------------+-----------------------+-----------------------+
   | +4                    | 4                     | useconds              | read                  | number of             |
   |                       |                       |                       |                       | microseconds past the |
   |                       |                       |                       |                       | number of seconds     |
   |                       |                       |                       |                       | since the epoch       |
   |                       |                       |                       |                       | (equivalent to the    |
   |                       |                       |                       |                       | ``tv_usec`` field     |
   |                       |                       |                       |                       | returned by POSIX     |
   |                       |                       |                       |                       | ``gettimeofday()``    |
   |                       |                       |                       |                       | function)             |
   +-----------------------+-----------------------+-----------------------+-----------------------+-----------------------+
   |                       |                       |                       | write                 | (ignored)             |
   +-----------------------+-----------------------+-----------------------+-----------------------+-----------------------+
   | Offset                | Size                  | Name                  | Operation             | Description           |
   +-----------------------+-----------------------+-----------------------+-----------------------+-----------------------+
   | +0                    | 8                     | time                  | read                  | number of seconds and |
   |                       |                       |                       |                       | microseconds since    |
   |                       |                       |                       |                       | the epoch, i. e.      |
   |                       |                       |                       |                       | 00:00:00 January 1st  |
   |                       |                       |                       |                       | 1970 UTC              |
   |                       |                       |                       |                       |                       |
   |                       |                       |                       |                       | bits ``0`` – ``31``   |
   |                       |                       |                       |                       |    equivalent to the  |
   |                       |                       |                       |                       |    ``tv_sec`` field   |
   |                       |                       |                       |                       |    returned by POSIX  |
   |                       |                       |                       |                       |    ``gettimeofday()`` |
   |                       |                       |                       |                       |    function           |
   |                       |                       |                       |                       | bits ``32`` – ``63``  |
   |                       |                       |                       |                       |    equivalent to the  |
   |                       |                       |                       |                       |    ``tv_usec`` field  |
   |                       |                       |                       |                       |    returned by POSIX  |
   |                       |                       |                       |                       |    ``gettimeofday()`` |
   |                       |                       |                       |                       |    function           |
   +-----------------------+-----------------------+-----------------------+-----------------------+-----------------------+
   |                       |                       |                       | write                 | (ignored)             |
   +-----------------------+-----------------------+-----------------------+-----------------------+-----------------------+

Commands
^^^^^^^^

``help [cmd]``
   Print a help on the command specified or a list of available commands.
``info``
   Print configuration information (assigned register address).
``stat``
   Print device statistics (none).




CPU cycle counter ``dcycle``
----------------------------

This device returns a 64-bit CPU cycle counter.

Initialization parameters: ``address``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``address``
   Physical address of the device register.

Registers
^^^^^^^^^

.. table:: ``dcycle`` programming registers (32 bit registers)

   ====== ==== ======================= ========= =================================================================
   Offset Size Name                    Operation Description
   ====== ==== ======================= ========= =================================================================
   +0     4    cycles (lower 32 bits)  read      number of CPU cycles since device initialization (lower 32 bits)
   \                                   write     (ignored)
   +4     4    cycles (higher 32 bits) read      number of CPU cycles since device initialization (higher 32 bits)
   \                                   write     (ignored)
   ====== ==== ======================= ========= =================================================================


.. table:: ``dcycle`` programming registers (64 bit register)

   ====== ==== ======================= ========= =================================================================
   Offset Size Name                    Operation Description
   ====== ==== ======================= ========= =================================================================
   Offset Size Name                    Operation Description
   +0     8    cycles (64 bits)        read      number of CPU cycles since device initialization
   \                                   write     (ignored)
   ====== ==== ======================= ========= =================================================================


Commands
^^^^^^^^

``help [cmd]``
   Print a help on the command specified or a list of available commands.
``info``
   Print configuration information (assigned register address).
``stat``
   Print device statistics (current cycle counter).




LCD display ``dlcd``
--------------------

The LCD display device simulates a simplified version of the `HD44780U <https://cdn.sparkfun.com/assets/9/5/f/7/b/HD44780.pdf>`_
character LCD display with memory-mapped control and data registers.

Initialization parameters: ``cols`` ``rows`` ``address``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``cols``
   Number of columns, or characters per row. Typical values are 16 or 20.

``rows``
   Number of rows. Typical values are 2 or 4.

``address``
   Physical address of the device register.

Registers
^^^^^^^^^

Offsets in the following tables are bits relative to the base address specified.

.. table:: ``data`` register

   ====== ==== ======================= ========= =================================================================
   Offset Size Name                    Operation Description
   ====== ==== ======================= ========= =================================================================
   +0     8    Data                    read      (not used)
   \                                   write     Write a character/command to the display
   ====== ==== ======================= ========= =================================================================


.. table:: ``control`` register

    ====== ==== ======================= ========= =================================================================
    Offset Size Name                    Operation Description
    ====== ==== ======================= ========= =================================================================
    +8     1    RS (Register Select)    read      (not used)
    \                                   write     Selects the purpose of the data register: 0: command register, 1: data register
    +9     1    RW (Read/Write)         read      (not used)
    \                                   write     0 = write to the display, 1 = read from the display (not supported)
    +10    1    E (Enable)              read      (not used)
    \                                   write     Causes the display to process the data written to the data register. This happens on a falling edge.
    +11    5    Reserved                read      Reserved
    \                                   write     Reserved
    ====== ==== ======================= ========= =================================================================


Commands
^^^^^^^^

``help [cmd]``
    Print a help on the command specified or a list of available commands.
``info``
    Print configuration information (number of columns, number of rows, assigned register address).


SuperH SH-2E Interrupt controller ``dsh2eintc``
------------------------------------------------

The ``dsh2eintc`` device simulates an interrupt controller for the SuperH SH-2E processor based on the `SH-2E SH7055S F-ZTAT Hardware Manual <https://www.renesas.com/en/document/mah/sh-2e-sh7055s-hardware-manual?language=en>`_.


Initialization parameters: ``address`` (optional)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``address``
   Starting physical address of device registers. (default: ``0xFFFFED00``)

Commands
^^^^^^^^

``help [cmd]``
   Display a help text for the specified command or a list of available commands.

``info``
   Display the interrupt controller configuration

``stat``
   Display interrupt controller statistics.

``rd``
   Dump contents of interrupt controller registers.

``addintsrc source_id priority_pool_index priority``
   Add a new interrupt source to the controller. Required parameters are:

   :source_id: interrupt source ID
   :priority_pool_index: index of the priority pool (0-31)
   :priority: interrupt priority (0-14)

``conf``
   Display the current interrupt source configuration.

Examples
^^^^^^^^

Simple usage example with the SuperH SH-2E CPU:

.. code:: msim

   [msim] add dsh2ecpu cpu0
   [msim] add dsh2eintc intc0
   [msim] cpu0 setintc intc0

Example of the ``info`` command:

.. code:: msim

   [msim] intc0 info
   SH-2E INTC

Example of the ``stat`` command:

.. code:: msim

   [msim] intc0 stat
   [Number of accepted interrupts] [Number of accepted resets    ] [Total accepted               ]
                                 0                               1                               1

Example of the ``rd`` command:

.. code:: msim

   [msim] intc0 rd
   intc 0
     ipra: e000
     iprb: 0000
     iprc: 0000
     iprd: 0000
     ipre: 0000
     iprf: 0000
     iprg: 0000
     iprh: 0000
     ipri: 0000
     iprj: 0000
     iprk: 0000
     iprl: 0000
      icr:    0
      isr:    0

Example of the ``addintsrc`` command:

.. code:: msim

   [msim] intc0 addintsrc 190 0 14
   [msim] intc0 rd
   intc 0
     ipra: e000
     iprb: 0000
     iprc: 0000
     iprd: 0000
     ipre: 0000
     iprf: 0000
     iprg: 0000
     iprh: 0000
     ipri: 0000
     iprj: 0000
     iprk: 0000
     iprl: 0000
      icr:    0
      isr:    0

Snippet from the `msim.conf` which configures the SH-2E INTC to mimic the SH7055 interrupt mapping:

.. code:: text

   add dsh2eintc intc0

   # power-on reset (external)
   intc0 addintsrc 0 0 0
   # power-on reset (internal)
   intc0 addintsrc 1 0 0
   # manual reset
   intc0 addintsrc 2 0 0

   # UBC (fixed priority 15)
   intc0 addintsrc 12 0 0
   # H-UDI (fixed priority 15)
   intc0 addintsrc 14 0 0

   # IRQ0-IRQ7
   intc0 addintsrc 64 0 14 # IPRA (15-12), priority 14
   intc0 addintsrc 65 1 13 # IPRA (11-8), priority 13
   intc0 addintsrc 66 2 12 # IPRA (7-4), priority 12
   intc0 addintsrc 67 3 11 # IPRA (3-0), priority 11
   intc0 addintsrc 68 4 10 # IPRB (15-12), priority 10
   intc0 addintsrc 69 5 9 # IPRB (15-12), priority 9
   intc0 addintsrc 70 6 8 # IPRB (15-12), priority 8
   intc0 addintsrc 71 7 7 # IPRB (15-12), priority 7

   # DMAC0-DMAC3
   intc0 addintsrc 72 8 6 # IPRC (15-12), priority 6
   intc0 addintsrc 74 8 6 # IPRC (15-12), priority 6
   intc0 addintsrc 76 9 6 # IPRC (11-8), priority 6
   intc0 addintsrc 78 9 6 # IPRC (11-8), priority 6

   # ATU
   intc0 addintsrc 80 10 5 # IPRC (7-4), priority 5
   intc0 addintsrc 84 11 5 # IPRC (3-0), priority 5
   intc0 addintsrc 86 11 5 # IPRC (3-0), priority 5
   intc0 addintsrc 88 12 5 # IPRD (15-12), priority 5
   intc0 addintsrc 90 12 5 # IPRD (15-12), priority 5
   intc0 addintsrc 92 13 5 # IPRD (11-8), priority 5
   intc0 addintsrc 96 14 5 # IPRD (7-4), priority 5
   intc0 addintsrc 97 14 5 # IPRD (7-4), priority 5
   intc0 addintsrc 98 14 5 # IPRD (7-4), priority 5
   intc0 addintsrc 99 14 5 # IPRD (7-4), priority 5
   intc0 addintsrc 100 15 5 # IPRD (3-0), priority 5
   intc0 addintsrc 101 15 5 # IPRD (3-0), priority 5
   intc0 addintsrc 102 15 5 # IPRD (3-0), priority 5
   intc0 addintsrc 103 15 5 # IPRD (3-0), priority 5
   intc0 addintsrc 104 16 5 # IPRE (15-12), priority 5
   intc0 addintsrc 108 17 5 # IPRE (11-8), priority 5
   intc0 addintsrc 109 17 5 # IPRE (11-8), priority 5
   intc0 addintsrc 110 17 5 # IPRE (11-8), priority 5
   intc0 addintsrc 111 17 5 # IPRE (11-8), priority 5
   intc0 addintsrc 112 18 5 # IPRE (7-4), priority 5
   intc0 addintsrc 113 18 5 # IPRE (7-4), priority 5
   intc0 addintsrc 114 18 5 # IPRE (7-4), priority 5
   intc0 addintsrc 115 18 5 # IPRE (7-4), priority 5
   intc0 addintsrc 116 19 5 # IPRE (3-0), priority 5
   intc0 addintsrc 120 20 5 # IPRF (15-12), priority 5
   intc0 addintsrc 121 20 5 # IPRF (15-12), priority 5
   intc0 addintsrc 122 20 5 # IPRF (15-12), priority 5
   intc0 addintsrc 123 20 5 # IPRF (15-12), priority 5
   intc0 addintsrc 124 21 5 # IPRF (11-8), priority 5
   intc0 addintsrc 128 22 5 # IPRF (7-4), priority 5
   intc0 addintsrc 129 22 5 # IPRF (7-4), priority 5
   intc0 addintsrc 130 22 5 # IPRF (7-4), priority 5
   intc0 addintsrc 131 22 5 # IPRF (7-4), priority 5
   intc0 addintsrc 132 23 5 # IPRF (3-0), priority 5
   intc0 addintsrc 136 24 5 # IPRG (15-12), priority 5
   intc0 addintsrc 137 24 5 # IPRG (15-12), priority 5
   intc0 addintsrc 138 24 5 # IPRG (15-12), priority 5
   intc0 addintsrc 139 24 5 # IPRG (15-12), priority 5
   intc0 addintsrc 140 25 5 # IPRG (11-8), priority 5
   intc0 addintsrc 144 26 5 # IPRG (7-4), priority 5
   intc0 addintsrc 145 26 5 # IPRG (7-4), priority 5
   intc0 addintsrc 146 26 5 # IPRG (7-4), priority 5
   intc0 addintsrc 147 26 5 # IPRG (7-4), priority 5
   intc0 addintsrc 148 27 5 # IPRG (3-0), priority 5
   intc0 addintsrc 149 27 5 # IPRG (3-0), priority 5
   intc0 addintsrc 150 27 5 # IPRG (3-0), priority 5
   intc0 addintsrc 151 27 5 # IPRG (3-0), priority 5
   intc0 addintsrc 152 28 5 # IPRH (15-12), priority 5
   intc0 addintsrc 153 28 5 # IPRH (15-12), priority 5
   intc0 addintsrc 154 28 5 # IPRH (15-12), priority 5
   intc0 addintsrc 155 28 5 # IPRH (15-12), priority 5
   intc0 addintsrc 156 29 5 # IPRH (11-8), priority 5
   intc0 addintsrc 157 29 5 # IPRH (11-8), priority 5
   intc0 addintsrc 158 29 5 # IPRH (11-8), priority 5
   intc0 addintsrc 159 29 5 # IPRH (11-8), priority 5
   intc0 addintsrc 160 30 5 # IPRH (7-4), priority 5
   intc0 addintsrc 161 30 5 # IPRH (7-4), priority 5
   intc0 addintsrc 162 30 5 # IPRH (7-4), priority 5
   intc0 addintsrc 163 30 5 # IPRH (7-4), priority 5
   intc0 addintsrc 164 31 5 # IPRH (3-0), priority 5
   intc0 addintsrc 165 31 5 # IPRH (3-0), priority 5
   intc0 addintsrc 166 31 5 # IPRH (3-0), priority 5
   intc0 addintsrc 167 31 5 # IPRH (3-0), priority 5
   intc0 addintsrc 168 32 5 # IPRI (15-12), priority 5
   intc0 addintsrc 169 32 5 # IPRI (15-12), priority 5
   intc0 addintsrc 170 32 5 # IPRI (15-12), priority 5
   intc0 addintsrc 171 32 5 # IPRI (15-12), priority 5
   intc0 addintsrc 172 33 5 # IPRI (11-8), priority 5
   intc0 addintsrc 174 33 5 # IPRI (11-8), priority 5
   intc0 addintsrc 176 34 5 # IPRI (7-4), priority 5
   intc0 addintsrc 178 34 5 # IPRI (7-4), priority 5
   intc0 addintsrc 180 35 5 # IPRI (3-0), priority 5
   intc0 addintsrc 184 36 5 # IPRJ (15-12), priority 5
   intc0 addintsrc 186 36 5 # IPRJ (15-12), priority 5
   intc0 addintsrc 187 36 5 # IPRJ (15-12), priority 5

   # CMT0
   intc0 addintsrc 188 37 4 # IPRJ (11-8), priority 4

   # A/D0
   intc0 addintsrc 190 37 4 # IPRJ (11-8), priority 4

   # CMT1
   intc0 addintsrc 192 38 4 # IPRJ (7-4), priority 4

   # A/D1-A/D2
   intc0 addintsrc 194 38 4 # IPRJ (7-4), priority 4
   intc0 addintsrc 196 39 4 # IPRJ (3-0), priority 4

   # SCI0-SCI4
   intc0 addintsrc 200 40 3 # IPRK (15-12), priority 3
   intc0 addintsrc 201 40 3 # IPRK (15-12), priority 3
   intc0 addintsrc 202 40 3 # IPRK (15-12), priority 3
   intc0 addintsrc 203 40 3 # IPRK (15-12), priority 3
   intc0 addintsrc 204 41 3 # IPRK (11-8), priority 3
   intc0 addintsrc 205 41 3 # IPRK (11-8), priority 3
   intc0 addintsrc 206 41 3 # IPRK (11-8), priority 3
   intc0 addintsrc 207 41 3 # IPRK (11-8), priority 3
   intc0 addintsrc 208 42 3 # IPRK (7-4), priority 3
   intc0 addintsrc 209 42 3 # IPRK (7-4), priority 3
   intc0 addintsrc 210 42 3 # IPRK (7-4), priority 3
   intc0 addintsrc 211 42 3 # IPRK (7-4), priority 3
   intc0 addintsrc 212 43 3 # IPRK (3-0), priority 3
   intc0 addintsrc 213 43 3 # IPRK (3-0), priority 3
   intc0 addintsrc 214 43 3 # IPRK (3-0), priority 3
   intc0 addintsrc 215 43 3 # IPRK (3-0), priority 3
   intc0 addintsrc 216 44 3 # IPRL (15-12), priority 3
   intc0 addintsrc 217 44 3 # IPRL (15-12), priority 3
   intc0 addintsrc 218 44 3 # IPRL (15-12), priority 3
   intc0 addintsrc 219 44 3 # IPRL (15-12), priority 3

   # HCAN0
   intc0 addintsrc 220 45 2 # IPRL (11-8), priority 2
   intc0 addintsrc 221 45 2 # IPRL (11-8), priority 2
   intc0 addintsrc 222 45 2 # IPRL (11-8), priority 2
   intc0 addintsrc 223 45 2 # IPRL (11-8), priority 2

   # WDT
   intc0 addintsrc 224 46 2 # IPRL (7-4), priority 2

   # HCAN1
   intc0 addintsrc 228 47 1 # IPRL (3-0), priority 1
   intc0 addintsrc 229 47 1 # IPRL (3-0), priority 1
   intc0 addintsrc 230 47 1 # IPRL (3-0), priority 1
   intc0 addintsrc 231 47 1 # IPRL (3-0), priority 1

After this configuration, the INTC registers look like this:

.. code:: msim

   [msim] intc0 rd
   intc 0
      ipra: edcb
      iprb: a987
      iprc: 6655
      iprd: 5555
      ipre: 5555
      iprf: 5555
      iprg: 5555
      iprh: 5555
      ipri: 5555
      iprj: 5444
      iprk: 3333
      iprl: 3221
       icr:    0
       isr:    0

Example of the ``conf`` command:

.. code:: msim

   [msim] intc0 conf
   intc 0
   [Interrupt no] [Type of interrupt      ] [Pending] [Priority register] [IPR bits]
               0   External power-on reset     false                   -          -
               1   Internal power-on reset     false                   -          -
               2              Manual reset     false                   -          -
              11                       NMI     false                   -          -
              12        Internal interrupt     false                   -          -
              14        Internal interrupt     false                   -          -
              64                      IRQ0     false                IPRA      15-12
              65                      IRQ1     false                IPRA       11-8
              66                      IRQ2     false                IPRA        7-4
              67                      IRQ3     false                IPRA        3-0
              68                      IRQ4     false                IPRB      15-12
              69                      IRQ5     false                IPRB       11-8
              70                      IRQ6     false                IPRB        7-4
              71                      IRQ7     false                IPRB        3-0
              72        Internal interrupt     false                IPRC      15-12
              74        Internal interrupt     false                IPRC      15-12
              76        Internal interrupt     false                IPRC       11-8
              78        Internal interrupt     false                IPRC       11-8


SuperH SH-2E Watchdog Timer ``dsh2ewdt``
------------------------------------------------

The ``dsh2ewdt`` device simulates a watchdog timer based on the `SH-2E SH7055S F-ZTAT Hardware Manual <https://www.renesas.com/en/document/mah/sh-2e-sh7055s-hardware-manual?language=en>`_ (page 431).

The watchdog timer works in 2 modes:

* Watchdog timer mode: the timer generates a reset when it overflows. The reset can be either a power-on reset or a manual reset, depending on the configuration.
* Interval timer mode: the timer generates an interrupt when it overflows.

The device also has 8 different counter input clocks.


Initialization parameters: ``int_no``, ``address`` (optional)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``int_no``
   Interrupt number for the watchdog timer.

``address`` (optional)
   Starting physical address of device registers. (default: ``0xFFFFEC10``)

Commands
^^^^^^^^

``help [cmd]``
   Display a help text for the specified command or a list of available commands.

``info``
   Display the watchdog timer configuration

``stat``
   Display watchdog timer statistics.

``rd``
   Dump contents of watchdog timer registers.

``addcpu cpu_name``
   Add a CPU reference to the watchdog timer. Required parameter is:

   :cpu_name: name of the CPU device to which the watchdog timer will send interrupts.

Examples
^^^^^^^^

Simple usage example with the SuperH SH-2E CPU:

.. code:: msim

   [msim] add dsh2ecpu cpu0
   [msim] add dsh2eintc intc0
   [msim] cpu0 setintc intc0
   [msim] add dsh2ewdt wdt 224
   [msim] intc0 addintsrc 224 0 10
   [msim] cpu0 addperipheral wdt
   [msim] wdt addcpu cpu0

Example of the ``info`` command:

.. code:: msim

   [msim] wdt info
   SH-2E Watchdog Timer (WDT)

Example of the ``stat`` command:

.. code:: msim

   [msim] wdt stat
   [WDT interrupts] [WDT power-on resets] [WDT manual resets]
                  0                     0                   0

Example of the ``rd`` command:

.. code:: msim

   [msim] wdt rd
   WDT
      tcsr: 38
      tcnt: 02
    rstcsr: 1f

Example of the ``addcpu cpu_name`` command:

.. code:: msim

   [msim] add dsh2ecpu cpu0
   [msim] cpu0 addperipheral wdt
   [msim] wdt addcpu cpu0


SuperH SH-2E Compare Match Timer ``dsh2ecmt``
------------------------------------------------

The ``dsh2ecmt`` device simulates a compare match timer based on the `SH-2E SH7055S F-ZTAT Hardware Manual <https://www.renesas.com/en/document/mah/sh-2e-sh7055s-hardware-manual?language=en>`_ (page 445).

The CMT has following features:

* 2 independent 16-bit timer channels (CMT0 and CMT1)
* 4 different internal clocks can be selected independently for each channel
* Each channel can generate an interrupt when the timer value matches the value in the compare register.

Initialization parameters: ``int_no_0``, ``int_no_1``, ``address`` (optional)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``int_no_0``
   Interrupt number for the CMT0 channel.

``int_no_1``
   Interrupt number for the CMT1 channel.

``address`` (optional)
   Starting physical address of device registers. (default: ``0xFFFFF710``)

Commands
^^^^^^^^

``help [cmd]``
   Display a help text for the specified command or a list of available commands.

``info``
   Display the compare match timer configuration

``stat``
   Display compare match timer statistics.

``rd``
   Dump contents of compare match timer registers.

``addcpu cpu_name``
   Add a CPU reference to the compare match timer. Required parameter is:

   :cpu_name: name of the CPU device to which the compare match timer will send interrupts.

Examples
^^^^^^^^

Simple usage example with the SuperH SH-2E CPU:

.. code:: msim

   [msim] add dsh2ecpu cpu0
   [msim] add dsh2eintc intc0
   [msim] cpu0 setintc intc0
   [msim] add dsh2ecmt cmt 188 192
   [msim] intc0 addintsrc 188 0 12
   [msim] intc0 addintsrc 192 1 11
   [msim] cpu0 addperipheral cmt
   [msim] cmt addcpu cpu0

Example of the ``info`` command:

.. code:: msim

   [msim] cmt info
   SH-2E Compare Match Timer (CMT)

Example of the ``stat`` command:

.. code:: msim

   [msim] cmt stat
   [CMT0 interrupts] [CMT1 interrupts] [Total interrupts]
                   0                 0                  0

Example of the ``rd`` command:

.. code:: msim

   [msim] cmt rd
   CMT
     cmstr: 0003
    cmcsr0: 0040
    cmcnt0: 0001
    cmcor0: ffff
    cmcsr1: 0041
    cmcnt1: 0000
    cmcor1: ffff

Example of the ``addcpu cpu_name`` command:

.. code:: msim

   [msim] add dsh2ecpu cpu0
   [msim] cpu0 addperipheral cmt
   [msim] cmt addcpu cpu0


SuperH SH-2E Direct Memory Access Controller ``dsh2edmac``
-----------------------------------------------------------

The ``dsh2edmac`` device simulates a direct memory access controller based on the `SH-2E SH7055S F-ZTAT Hardware Manual <https://www.renesas.com/en/document/mah/sh-2e-sh7055s-hardware-manual?language=en>`_ (page 157).

The DMAC has following features:

* Four channels
* 8-, 16-, or 32-bit selectable data transfer length
* Address modes:
   Both the transfer source and transfer destination are accessed by address. There are two transfer modes: direct address and indirect address.

   * Direct address transfer mode: Values set in a DMAC internal register indicate the accessed address for both the transfer source and transfer destination.
   * Indirect address transfer mode: The value stored at the location pointed to by the address set in the DMAC internal transfer source register is used as the address. Operation is otherwise the same as for direct access. This function can only be set for channel 3.

* Direct address transfer mode or indirect address transfer mode can be specified for channel 3.
* Reload function
   * Enables automatic reloading of the value set in the first source address register every fourth DMA transfer. This function can be executed on channel 2 only.

* Transfer requests
   There are two DMAC transfer activation requests, as indicated below.

   * Requests from on-chip peripheral modules: Transfer requests from on-chip modules
   * Auto-request: The transfer request is generated automatically within the DMAC.

* Transfer modes
   As the MSIM does not simulate the bus, the DMAC transfer modes are simplified as follows:

   * Cycle-steal mode: 1 transfer per 1 CPU cycle.
   * Burst-mode: 2 transfers per 1 CPU cycle.

* CPU can be interrupted when the specified number of data transfers are completed.
* Fixed DMAC channel priority ranking is 0 > 1 > 2 > 3.

Initialization parameters: ``int_no_0``, ``int_no_1``, ``int_no_2``, ``int_no_3``, ``address`` (optional)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``int_no_0``
   Interrupt number for the DMAC0 channel.

``int_no_1``
   Interrupt number for the DMAC1 channel.

``int_no_2``
   Interrupt number for the DMAC2 channel.

``int_no_3``
   Interrupt number for the DMAC3 channel.

``address`` (optional)
   Starting physical address of device registers. (default: ``0xFFFFECB0``)

Commands
^^^^^^^^

``help [cmd]``
   Display a help text for the specified command or a list of available commands.

``info``
   Display the DMAC configuration

``stat``
   Display the DMAC statistics.

``rd``
   Dump contents of DMAC registers.

``addcpu cpu_name``
   Add a CPU reference to the DMAC. Required parameter is:

   :cpu_name: name of the CPU device to which the compare match timer will send interrupts.

``addsource request_index request_type address``
   Add a new request source to the DMAC. Required parameters are:

   :request_index: index of the request source (0-31)
   :request_type: type of the request source (0: RECEIVE, 1: TRANSMIT, 2: DO_NOT_CARE)

      * RECEIVE: the transfer is valid and will proceed only if the SAR contains the same address as the ``address`` parameter of the ``addsource`` command.
      * TRANSMIT: the transfer is valid and will proceed only if the DAR contains the same address as the ``address`` parameter of the ``addsource`` command.
      * DO_NOT_CARE: the transfer is valid and will proceed regardless of the SAR and DAR values.

   :address: address to read from/write to for peripheral request sources (required only for RECEIVE and TRANSMIT request types)

Examples
^^^^^^^^

Simple usage example with the SuperH SH-2E CPU:

.. code:: msim

   [msim] add dsh2ecpu cpu0
   [msim] add dsh2eintc intc0
   [msim] cpu0 setintc intc0
   [msim] add dsh2edmac dmac 72 74 76 78
   [msim] intc0 addintsrc 72 8 10
   [msim] intc0 addintsrc 74 8 10
   [msim] intc0 addintsrc 76 8 10
   [msim] intc0 addintsrc 78 8 10
   [msim] cpu0 addperipheral dmac
   [msim] dmac addcpu cpu0

Example of the ``info`` command:

.. code:: msim

   [msim] dmac info
   SH-2E DMAC


Example of the ``stat`` command:

.. code:: msim

   [msim] dmac stat
   [Total transfers] [Total interrupts]
                   0                  0

Example of the ``rd`` command:

.. code:: msim

   [msim] dmac rd
   DMAC
      sar0: ffffa010
      dar0: ffffb010
      tcr0: 00000000
     chcr0: 001f1127
      sar1: 00000000
      dar1: 00000000
      tcr1: 00000000
     chcr1: 00000000
      sar2: ffffa01c
      dar2: ffffb01c
      tcr2: 00000001
     chcr2: 011f1125
      sar3: 00000000
      dar3: 00000000
      tcr3: 00000000
     chcr3: 00000000
     dmaor: 0001

Example of the ``addcpu cpu_name`` command:

.. code:: msim

   [msim] add dsh2ecpu cpu0
   [msim] cpu0 addperipheral dmac
   [msim] dmac addcpu cpu0


Example of the ``addsource`` command:

.. code:: msim

   [msim] dmac addsource 1 1 0xffffe000
   [msim] dmac addsource 2 2