    .section .text
    .globl _start

_start:
    li      t0, 0x10000000       # Adresse GPIO / sortie
    li      t1, 5
    li      t2, 10

    #########################################
    # TEST 1 : JAL
    #########################################
    jal     ra, test_jal         # saut vers label + ra = _start + 4
    li      t3, 'X'              # Ne doit PAS s'exécuter
    sw      t3, 0(t0)

after_jal:

    #########################################
    # TEST 2 : JALR
    #########################################
    la      t4, test_jalr        # charger l’adresse de test_jalr
    jalr    ra, 0(t4)            # saut indirect
    li      t3, 'Y'              # Ne doit PAS s'exécuter
    sw      t3, 0(t0)

after_jalr:

    #########################################
    # TEST 3 : BLT / BGE
    #########################################

    blt     t1, t2, less_case
    li      t3, 'N'              # ne doit pas s'exécuter
    sw      t3, 0(t0)

less_case:
    li      t3, 'L'              # L comme "Less"
    sw      t3, 0(t0)
    j       continue_blt_bge

continue_blt_bge:
    bge     t2, t1, greater_equal_case
    li      t3, 'N'              # ne doit pas s'exécuter
    sw      t3, 0(t0)
    j       finish

greater_equal_case:
    li      t3, 'G'              # G comme "Greater-or-equal"
    sw      t3, 0(t0)
    j finish

finish:
    ebreak


###################################################
# Sous-routines appelées par JAL et JALR
###################################################

test_jal:
    li      t3, 'J'
    sw      t3, 0(t0)
    j       after_jal                   # retour

test_jalr:
    li      t3, 'R'
    sw      t3, 0(t0)
    j       after_jalr                   # retour

