    # Mini-RISC comprehensive test
    # RAM base: 0x80000000
    # Charout: 0x10000000

    # -------------------------
    # Setup base address in x1
    # -------------------------
    lui x1, 0x80000      # x1 = 0x80000000
    addi x1, x1, 0       # x1 = 0x80000000

    # -------------------------
    # LUI / ADDI / AUIPC
    # -------------------------
    lui x2, 0x12345      # x2 = 0x12345000
    addi x3, x2, 0x678   # x3 = 0x12345678
    auipc x4, 0x10       # x4 = PC + (0x10 << 12)

    # -------------------------
    # SW / LW / SH / SB
    # -------------------------
    sw x2, 0(x1)         # RAM[0] = x2
    addi x1, x1, 4
    sw x3, 0(x1)         # RAM[4] = x3
    addi x1, x1, 4
    sb x3, 0(x1)         # store lower byte of x3 in RAM[8]
    addi x1, x1, 1
    sh x3, 0(x1)         # store lower half-word of x3 in RAM[9]

    # Reload addresses
    lui x5, 0x80000
    addi x5, x5, 0
    lw x6, 0(x5)         # x6 = RAM[0] = x2
    addi x5, x5, 4
    lw x7, 0(x5)         # x7 = RAM[4] = x3
    addi x5, x5, 4
    lbu x8, 0(x5)        # x8 = RAM[8] = x3 & 0xFF
    addi x5, x5, 1
    lhu x9, 0(x5)        # x9 = RAM[9] = x3 & 0xFFFF

    # -------------------------
    # Branch instructions
    # -------------------------
    addi x10, x0, 5      # x10 = 5
    addi x11, x0, 5      # x11 = 5
    addi x12, x0, 10     # x12 = 10

    beq x10, x11, label_beq     # branch taken
    addi x13, x0, 0x111        # skipped
label_beq:
    bne x10, x12, label_bne     # branch taken
    addi x14, x0, 0x222        # skipped
label_bne:
    blt x10, x12, label_blt     # branch taken
    addi x15, x0, 0x333        # skipped
label_blt:
    bge x12, x10, label_bge     # branch taken
    addi x16, x0, 0x444        # skipped
label_bge:
    bltu x10, x12, label_bltu   # branch taken
    addi x17, x0, 0x555        # skipped
label_bltu:
    bgeu x12, x10, label_bgeu   # branch taken
    addi x18, x0, 0x666        # skipped
label_bgeu:

    # -------------------------
    # Jump instructions
    # -------------------------
    jal x19, label_jal_target    # x19 = PC+4
    addi x20, x0, 0x777        # skipped
label_jal_target:
    jalr x21, x19, 0             # x21 = PC+4, jump to x19+0

    # -------------------------
    # Halt the emulator
    # -------------------------
    ebreak
