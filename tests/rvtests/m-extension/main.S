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

mul s0, a4, a6
mulh s1, a4, a4
mulhsu s2, a4, a4
mulhu s3, a4, a4
div s4, a3, a2
div s5, a3, a6
divu s6, a5, a2
rem s7, a3, a2
rem s8, a3, a6
remu s9, a5, a2

div t0, a1, a0
rem t1, a1, a0
div t2, a5, a4
div t3, a5, a5
divu t4, a1, a0
remu t5, a1, a0

ebreak