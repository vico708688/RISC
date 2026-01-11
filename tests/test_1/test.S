li t0,0x10000000 # écrit l'adresse de charout dans le registre t0
li t1,'H'        # écrit le caractère 'H' (soit 0x48) dans t1
sw t1,0(t0)      # écrit la valeur de t1 à l'adresse t0 + 0
li t1,'e'
sw t1,0(t0)
li t1,'l'
sw t1,0(t0)
sw t1,0(t0)
li t1,'o'
sw t1,0(t0)
li t1,' '
sw t1,0(t0)
li t1,'!'
sw t1,0(t0)
li t1,'\n'
sw t1,0(t0)
ebreak           # stope le simulateur