    # Adresse de charout = 0x10000000
    li t0, 0x10000000

    # -----------------------------------------
    # Test : JAL -> saut vers print_hello
    # -----------------------------------------
    jal ra, print_hello     # ra = PC+4 ; saute à print_hello

    # -----------------------------------------
    # Programme continue après retour JALR
    # -----------------------------------------
after_return:
    li t1, ' '              # espace
    sw t1, 0(t0)

    li t1, 'W'
    sw t1, 0(t0)
    li t1, 'o'
    sw t1, 0(t0)
    li t1, 'r'
    sw t1, 0(t0)
    li t1, 'l'
    sw t1, 0(t0)
    li t1, 'd'
    sw t1, 0(t0)
    li t1, '!'
    sw t1, 0(t0)
    li t1, '\n'
    sw t1, 0(t0)

    ebreak                  # stop le simulateur

# -----------------------------------------
# Routine appelée par JAL
# -----------------------------------------
print_hello:
    li t1, 'H'
    sw t1, 0(t0)
    li t1, 'e'
    sw t1, 0(t0)
    li t1, 'l'
    sw t1, 0(t0)
    sw t1, 0(t0)
    li t1, 'o'
    sw t1, 0(t0)

    # Retour au caller
    jalr zero, 0(ra)        # PC = ra (adresse sauvegardée par JAL)
