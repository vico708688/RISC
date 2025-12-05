.globl _start
.section .text

_start:
    li t0, 0x10000000
    li t1, 0x80000000

    li t2, 0x2
    li t3, 0x3

    mul t4, t2, t3
    sw t4, 0(t0)
    j stop
    

youpi_auipc:
    li t4, 'U'
    sw t4, 0(t0)
    j  stop

stop:
    ebreak