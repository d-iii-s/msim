.text
li a0, 0
li a1, 1
li t0, 'S'
li t1, 'F'
li t2, 0x90000000
j first
sw t1, 0(t2)
simulator_halt
first:
sw t0, 0(t2)
auipc a3, 0
jalr zero, 0x10(a3)
sw t1, 0(t2)
simulator_halt
second:
sw t0, 0(t2)
simulator_halt
