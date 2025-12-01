li     t0, 0x10000040
li     t1, 0x80000000

li t2, 0xdeadbe1f
sb t2, 33(t1)

lw t3, 32(t1)

ebreak