.text
.set noat
.set noreorder
nop

li $a0, 0xdeadbeef
nop
.insn
.word 0x35
nop

.insn
simulator_halt
