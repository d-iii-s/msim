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




RISC-V Processor ``drvcpu``
---------------------------

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
``ptd [verbose|v]``
   Prints out all valid PTEs in the pagetable currently pointed to by the satp CSR.
   Adding the ``verbose`` parameter (or simply ``v``) prints out all nonzero PTEs.
``sptd phys [verbose|v]``
   Prints out all valid PTEs in the pagetable with its root pagetable located at ``phys`` (physical address).
   Note that this address has to be aligned to the size of a page (``4096``).
   Adding the ``verbose`` parameter (or simply ``v``) prints out all nonzero PTEs.
``tlbd``
   Dump the contents of the TLB, split by page size.
``tlbresize size``
   Resize the TLB by specifying its new size.
``tlbflush``
   Removes all entries from the TLB.
``asidlen length``
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

Example
^^^^^^^

Example of the ``info`` command:

.. code:: msim

   [msim] add dprinter printer 0x10000000
   [msim] printer info
   address:0x01000000
   [msim]

Example of the ``stat`` command:

.. code:: msim

   [msim] printer stat
   count:11385
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
        :header: 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4,3,2,1 0

        r1,e,i,r0

    ``r0``, ``r1``
        (reserved)
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
