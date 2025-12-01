li     t0, 0x10000040
li     t1, 0x80000000

li t2, 0xdeadbe1f
sh t2, 34(t1)

lhu t3, 34(t1)

ebreak