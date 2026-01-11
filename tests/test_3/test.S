    .section .text
    .globl _start

_start:
    li    t0, 0x10000000
    auipc t1, 1
    li    t2, 0x80001004
    # addi  t2, t2, 0x100
    beq   t1, t2, youpi_auipc
    li    t3, 'E'
    sw    t3, 0(t0)
    j     stop

youpi_auipc:
    li t4, 'A'
    sw t4, 0(t0)
    j  stop

stop:
    ebreak

after_auipc: