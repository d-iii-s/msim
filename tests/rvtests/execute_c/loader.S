// This is loaded at address 0xF0000000
// We set the stack to start at the end of this page
// That way, we have an almost entire page of stack space
start:
    li t0, 0x80000000
    li sp, 0xF0001000
    jalr zero, 0(t0)
.end start