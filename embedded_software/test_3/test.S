    .section .text
    .globl _start

_start:
    ################################################################
    # Test simple : AUIPC x5, 0x12345
    # AUIPC place dans x5 : PC + (0x12345 << 12)
    ################################################################

    auipc x5, 0x12345     # x5 = PC + 0x12345000

    # Sauvegarde du résultat en mémoire pour inspection
    # (adapter l’adresse selon ton banc mémoire)
    li    t0, 0x80000000
    sw    x5, 0(t0)

    ################################################################
    # Deuxième test : AUIPC suivi d'un ADDI pour reconstruire une adresse
    ################################################################
    auipc x6, 0x00002     # x6 = PC + 0x2000
    addi  x6, x6, 0x34    # x6 = PC + 0x2034

    sw    x6, 4(t0)

    ################################################################
    # Troisième test : Vérifier que PC a changé entre les deux AUIPC
    ################################################################

    auipc x7, 0           # x7 = PC

    sw    x7, 8(t0)

    ################################################################
    # Fin du programme
    ################################################################
stop:
    ebreak                # arrêt du simulateur
