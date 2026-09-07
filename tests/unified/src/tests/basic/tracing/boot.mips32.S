.text
li $a0, 12
li $a1, 52
add $a2, $a0, $a1

li $k0, 0x90000000
sw $a2, 0($k0)

.insn
simulator_halt
