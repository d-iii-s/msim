import sys

# region | Hack to import from parent directory, otherwise we would have to install the library as package
try:
    from ..dap_lib import *  # noqa # This fails at runtime but makes static analysis pass
except ImportError:
    import os

    sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
    from dap_lib import *  # type: ignore # IDEs don't like this, but it works
# endregion

# MSIM RISC-V32 and RISC-V64 Configuration
RST_VEC = 0xF0000000  # Reset vector address
PROGRAM_LEN = 8  # Length of the test program in instructions, not including the halt instruction
INSTR_LEN = 4  # Length of a single instruction in bytes
REG_COUNT = 32  # Number of general-purpose registers in RISC-V (both 32 and 64)
NOP_INSTR = 0x00000013  # NOP instruction in RISC-V (ADDI x0, x0, 0)

DEFAULT_CPU = 0  # Default CPU used in DAP
CPU0 = 0
CPU1 = 1

at = make_at(RST_VEC, INSTR_LEN)
"""address by instruction number for both RISC-V32 and RISC-V64"""