.globl _start
.section .text

_start:
    li t0, 0x10000000
    li t1, 0x80000000

    li t2, 0x7
    li t3, 0x-3

    div t4, t2, t3
    sw t4, 4(t0)
    j stop

stop:
    ebreak