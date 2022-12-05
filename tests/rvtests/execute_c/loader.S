// This is loaded at address 0xF0000000
// We set the stack to start at the end of the area where this is loaded
// This area is 16 KiB
start:
    li t0, 0x80000000
    li sp, 0xF0004000
    jalr zero, 0(t0)
.end start