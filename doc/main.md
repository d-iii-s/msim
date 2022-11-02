# MSIM Developement Manual

For inforamtion about the project and user documentation see the [Reference](reference.html).

## Structure

The entry point and main event loop are specified in [main.c](@ref main.c).

Input parsing is located int [input.c](@ref input.c) and [parser.c](@ref parser.c) while command execution is located in [cmd.c](@ref cmd.c).

MSIM environment variables are handled in [env.c](@ref env.c).

Physical memory logic is in [physmem.c](@ref physmem.c).

Some usefull utilities can be found in [utils.c](@ref utils.c), [fault.c](@ref fault.c), [assert.h](@ref assert.h), [endian.c](@ref endian.h) and [list.c](@ref list.c).

String constants are located in [text.c](@ref text.c).

Architecture dependant code is located in arch directory.

Debugging features, including work-in-progress GDB support can be found in debug directory.

The device interface is specified in [device/device.h](@ref device.h).

All the devices are specified in the device directory.

The general cpu interface in located in [device/cpu/general_cpu.h](@ref general_cpu.h).

The cpu architectures are implemented in their own directories device/cpu/mips_r4000 and device/cpu/riscv_rv32ima.


List of all source files can be found [here](files.html).
