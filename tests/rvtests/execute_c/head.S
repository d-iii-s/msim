#define ehalt .word 0x00200073
.section __start
start:
    jal main
    ehalt
.end start