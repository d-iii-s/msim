#define ehalt .word 0x8C000073
#define edump .word 0x8C100073
.text
// tests m-extension
li a0, 0
li a1, 1
li a2, 2
li a3, 3
li a4, -1
// INT32_MIN
li a5, 0x80000000
li a6, -2

// Saves the result of each operation to separate registers
// Dumps all registers at the end of the test

// negative * negative = positive
mul s0, a4, a6

// tests high 32 bits of -1 * -1 based on the signed/unsigned interpretation
mulh s1, a4, a4
mulhsu s2, a4, a4
mulhu s3, a4, a4

// 3 / 2 = 1
div s4, a3, a2
// 3 / -2 = -1
div s5, a3, a6

// INT32_MIN (but unsigned) / 2
divu s6, a5, a2

// 3 % 2 = 1
rem s7, a3, a2
// 3 % -2 = 1
rem s8, a3, a6
// INT32_MIN (but unsigned) % 2 = 0
remu s9, a5, a2

// Edge cases with 0
div t0, a1, a0
rem t1, a1, a0

// Edge cases with INT32_MAX
div t2, a5, a4
div t3, a5, a5

// Edge cases with 0
divu t4, a1, a0
remu t5, a1, a0
edump
ehalt
