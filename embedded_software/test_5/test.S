li     t0, 0x10000000
li     t1, 0x80000000

li t2, 0xdeadbe1f
sw t2, 32(t1)

lb t3, 33(t1)
sw t3, 0(t0)


ebreak