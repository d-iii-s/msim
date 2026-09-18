.text
li a0, 12
li a1, 52
add a2, a0, a1

li t0, 0x90000000
sw a2, 0(t0)

simulator_halt
