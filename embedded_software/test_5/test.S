    .section .text
    .globl _start

_start:
    # Adresse GPIO
    LUI     t0, 0x10000          # t0 = 0x10000000

    ######################################################
    # Préparation des registres pour les 4 tests
    ######################################################
    ADDI    t1, x0, 5            # t1 = 5
    ADDI    t2, x0, 10           # t2 = 10
    ADDI    t3, x0, 5            # t3 = 5  (pour BEQ / BNE)
    ADDI    t4, x0, 0xFFFFFFF0   # t4 = 0xFFFFFFF0 (valeur unsigned grande)

    ######################################################
    # TEST 1 : BLTU  (5 < 10 en NON signé → vrai)
    ######################################################
    BLT     t1, t2, bltu_taken     # version signée OK pour miniRISC
    JAL     x0, bltu_not_taken

bltu_taken:
    ADDI    t5, x0, 'U'
    SW      t5, 0(t0)
    JAL     x0, after_bltu

bltu_not_taken:
    ADDI    t5, x0, 'u'
    SW      t5, 0(t0)

after_bltu:

    ######################################################
    # TEST 2 : BNE  (5 != 5 → faux)
    ######################################################
    BNE     t1, t3, bne_taken
    JAL     x0, bne_not_taken

bne_taken:
    ADDI    t5, x0, 'N'
    SW      t5, 0(t0)
    JAL     x0, after_bne

bne_not_taken:
    ADDI    t5, x0, 'n'
    SW      t5, 0(t0)

after_bne:

    ######################################################
    # TEST 3 : BEQ  (5 == 5 → vrai)
    ######################################################
    BEQ     t1, t3, beq_taken
    JAL     x0, beq_not_taken

beq_taken:
    ADDI    t5, x0, 'E'
    SW      t5, 0(t0)
    JAL     x0, after_beq

beq_not_taken:
    ADDI    t5, x0, 'e'
    SW      t5, 0(t0)

after_beq:

    ######################################################
    # TEST 4 : BGEU  (0xFFFFFFF0 >= 5 en NON SIGNÉ → vrai)
    ######################################################
    BGE     t4, t1, bgeu_taken      # version signée pour miniRISC
    JAL     x0, bgeu_not_taken

bgeu_taken:
    ADDI    t5, x0, 'G'
    SW      t5, 0(t0)
    JAL     x0, after_bgeu

bgeu_not_taken:
    ADDI    t5, x0, 'g'
    SW      t5, 0(t0)

after_bgeu:

    ######################################################
    # STOP
    ######################################################
finish:
    EBREAK
