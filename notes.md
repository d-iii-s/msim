# implementation notes

## hooking into msim

### dcpu

- dcpu is an adapter between MIPS R4000 and devices in msim
- similar device would be nice for risc-v
- debug expects that all cpus are this device

### debug

#### breakpoints

`breakpoint_check_for_code_breakpoints` needs to include risc-v processors

### dump/print

rename to r4k debug

similar features would be nice in risc

#### gdb

gdb reads physical memory, but does not specify cpu

TODO: look into physmem read and find why is the cpu there.

Done: physmem calls r/w on memory mapped IO, and some devices need
the proc id.

reg dumps - 64 bit registers are implemented, convert or implement 32 bit?

handle event - dumps registers of r4k

`gdb_read_registers` coupled with r4k

`gdb_write registers` coupled with r4k

`gdb_cmd_mem_operation` converts virtual to physical addresses (using dcpu)

`gdb_cmd_step` can set the `pc`

insert/remove breakpoint works with r4k

gdb sends 32-bit addresses

### interface needed from outside

I would suggest using a separate device type for risc-v cpus, as mips and risc should support different commands, but the cpu-structures should implement a common interface.
They both need to be used from gdb, and they should share the same proc/hart id namespace.

common interface should be:

- add breakpoint
- remove breakpoint
- convert_addr
- reg dump
- reg set/read
- set_pc
- interrupt_up
- interrupt_down
- lr(ll) check (checks if given address is tracked, if so, triggers the ll and then returns if it was tracked)

The interface can take the proc id as an argument, or take the cpu* directly...

I think, that using the proc/hart id is better, as it hides the implementation details of cpus and delegates the dispatching to the cpu managing code.

~~this interface should be returned from the call to dcpu_find_no(id) (maybe rename this function)~~

this interface will be accessed via the cpu no.

### device type

we need to distiguish between r4k, rv and general cpu - rename the device type

(also change in `device.c`, in the filtering utilities)

device memory read and write expect r4k_cpu*

device read/write 32/64 do not need to work with cpu*, but could as well work with cpu id
(the only one that uses the cpu* at all is dorder read32)

### physmem

physmem should be moved to a different module, changing the interface (mostly so it takes cpu id instead of cpu*)

Each physmem frame remembered a buffer of decoded instructions.
This does not work for a system of multiple architectures.
Although, the two processors should not execute from same piece of memory in practice.
For now, I have changed it, so the processor holds itself 1 frame of decoded data, but it could be changed later, that the frames themselves hold the decoded data with information, which architecture are they decoded into.

This is untested as of now on mips (!!!)

### mem

`mem.h` and `mem.c` are named unfortunately, they should be named dmem.

### LR and SC

they need to cooperate with mips cpu, so the SC control should work in general physmem manager.

The update needs to be tested.

### exceptions

## interface viewpoint

`device` is an interface, that the cpu implements (partially)

`cpu` can a another interface, that the processors will implement.
The implementation will be similar to the device abstraction,
but with specific functionality.

the `general_cpu_t` type provides the abstraction, and the `general_cpu.h` file
provides a better interface for calling the functions.

### device->data

The `general_cpu` has to be allocated and freed, somewhere, somehow.

I would suggest, that the device (dr4kcpu,...) alloc and free them

## data types

36 bit physical space can be supported. RISC-V has only 34-bit physical addresses, but the spec states, that they can be zero-extended to any length used by the implementation, so we can satisfy shis interface.

## environment

msim allows different names of registers, this needs to be abstracted,
or separate functionality needs to be added.

## random msim facts

- all device interrupts are handled on cpu with id 0
- LL and SC only do checks on the address and are not based on size, so 64 bit write on address 0x00 would not trigger an 32 bit LL on address 0x04.
