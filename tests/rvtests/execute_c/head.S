#define ehalt .word 0x8C000073
.section __start
start:
    jal main
    ehalt
.end start