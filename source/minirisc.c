#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "minirisc.h"
#include "platform.h"

struct minirisc_t *minirisc_new(uint32_t initial_PC, struct platform_t *platform)
{
    struct minirisc_t *cpu;

    if ((cpu = malloc(sizeof(struct minirisc_t))) == NULL)
    {
        printf("Error minirisc malloc.\n");
        return NULL;
    };

    cpu->PC = initial_PC;
    cpu->IR = platform->memory[initial_PC - RAM_BASE];
    cpu->next_PC = initial_PC;
    cpu->platform = platform;
    cpu->halt = 0;

    return cpu;
}

void minirisc_free(struct minirisc_t *minirisc)
{
    free(minirisc);
}

void minirisc_fetch(struct minirisc_t *minirisc)
{
    uint32_t PC = minirisc->PC;
    uint32_t instruction;

    platform_read(minirisc->platform, ACCESS_WORD, PC, &instruction);

    minirisc->IR = instruction;
    minirisc->next_PC = PC + 4;
}

void minirisc_decode_and_execute(struct minirisc_t *minirisc)
{
    uint32_t instr = minirisc->IR;

    uint32_t opcode = instr & 0x7F; /* apply a mask to get the 7 LSB */

    uint32_t rd = (instr >> 7) & 0x1F;
    uint32_t rs_1 = (instr >> 12) & 0x1F;
    uint32_t rs2 = (instr >> 16) & 0x1F;

    uint32_t imm = (instr >> 20) & 0xFFF;
    uint32_t imm_lui = (instr >> 12) & 0xFFFFF;

    uint32_t shamt = (instr >> 20) & 0x1F;
    uint32_t shamt_zeros = (instr >> 25) & 0x7F;

    /* next_PC update */
    minirisc->next_PC = minirisc->PC + 4;

    /* Depends of the opcode's value */
    switch (opcode)
    {
    /* OK */
    case LUI_CODE:
    {
        minirisc->regs[rd] = (imm_lui << 12);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case ADDI_CODE:
    {
        // printf("ADDI\n");
        extend_sign(&imm, 11);
        minirisc->regs[rd] = minirisc->regs[rs_1] + imm;

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case EBREAK_CODE:
    {
        minirisc->halt = 1;

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case AUIPC_CODE:
    {
        minirisc->regs[rd] = minirisc->PC + (imm_lui << 12);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case JAL_CODE:
    {
        imm_lui <<= 1;
        extend_sign(&imm_lui, 20);

        uint32_t target = minirisc->PC + imm_lui;
        minirisc->regs[rd] = minirisc->PC + 4;

        minirisc->PC = target;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case JALR_CODE:
    {
        printf("JALR\n");
        extend_sign(&imm_lui, 11);

        uint32_t target = minirisc->regs[rs_1] + imm_lui;
        target &= ~0x1; /* LSB à 0 */
        minirisc->regs[rd] = minirisc->PC + 4;
        // printf("rd, minirisc->regs[rd]: %x, %x\n", rd, minirisc->regs[rd]);
        printf("imm_lui: %x\n", imm_lui);
        printf("rs_1: %x\n", rs_1);
        printf("target: %x\n", target);

        minirisc->PC = target;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* Revoir types B: extend_sign(&imm, ??? 13 ???) */
    /* OK */
    case BEQ_CODE:
    {
        imm <<= 1;
        // Sign extension
        extend_sign(&imm, 13);

        uint32_t target = minirisc->PC + imm;

        minirisc->next_PC = minirisc->PC + 4;
        if ((int32_t)minirisc->regs[rs_1] == (int32_t)minirisc->regs[rd])
        {
            minirisc->PC = target;
        }
        else
        {
            minirisc->PC = minirisc->next_PC;
        }
        break;
    }
    /* OK */
    case BNE_CODE:
    {
        imm <<= 1;
        // Sign extension
        extend_sign(&imm, 13);

        uint32_t target = minirisc->PC + imm;

        minirisc->next_PC = minirisc->PC + 4;
        if ((int32_t)minirisc->regs[rs_1] != (int32_t)minirisc->regs[rd])
        {
            minirisc->PC = target;
        }
        else
        {
            minirisc->PC = minirisc->next_PC;
        }
        break;
    }

    case BLT_CODE:
    {
        imm <<= 1;
        // Sign extension
        extend_sign(&imm, 13);

        uint32_t target = minirisc->PC + imm;

        minirisc->next_PC = minirisc->PC + 4;
        if ((int32_t)minirisc->regs[rs_1] < (int32_t)minirisc->regs[rd])
        {
            minirisc->PC = target;
        }
        else
        {
            minirisc->PC = minirisc->next_PC;
        }
        break;
    }

    case BGE_CODE:
    {
        imm <<= 1;
        // Sign extension
        extend_sign(&imm, 13);

        uint32_t target = minirisc->PC + imm;

        minirisc->next_PC = minirisc->PC + 4;
        if ((int32_t)minirisc->regs[rs_1] >= (int32_t)minirisc->regs[rd])
        {
            minirisc->PC = target;
        }
        else
        {
            minirisc->PC = minirisc->next_PC;
        }
        break;
    }

    case BLTU_CODE:
    {
        imm <<= 1;
        // Sign extension
        extend_sign(&imm, 13);

        uint32_t target = minirisc->PC + imm;

        minirisc->next_PC = minirisc->PC + 4;
        if (minirisc->regs[rs_1] < minirisc->regs[rd])
        {
            minirisc->PC = target;
        }
        else
        {
            minirisc->PC = minirisc->next_PC;
        }
        break;
    }

    case BGEU_CODE:
    {
        imm <<= 1;
        // Sign extension
        extend_sign(&imm, 13);

        uint32_t target = minirisc->PC + imm;

        minirisc->next_PC = minirisc->PC + 4;
        if (minirisc->regs[rs_1] >= minirisc->regs[rd])
        {
            minirisc->PC = target;
        }
        else
        {
            minirisc->PC = minirisc->next_PC;
        }
        break;
    }
    /* OK */
    case SW_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t target = minirisc->regs[rs_1] + imm;
        uint32_t data = minirisc->regs[rd];

        platform_write(minirisc->platform, ACCESS_WORD, target, data);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* A revoir */
    case SH_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t addr = minirisc->regs[rs_1] + imm;
        uint32_t data = minirisc->regs[rd];

        platform_write(minirisc->platform, ACCESS_HALF, addr, data);
        break;
    }
    /* A revoir */
    case SB_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t addr = minirisc->regs[rs_1] + imm;
        uint32_t data = minirisc->regs[rd];

        platform_write(minirisc->platform, ACCESS_BYTE, addr, data);
        break;
    }
    /* A revoir */
    case LW_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t target = minirisc->regs[rs_1] + imm;
        uint32_t addr = minirisc->regs[rs_1];

        platform_read(minirisc->platform, ACCESS_WORD, target, &addr);
        break;
    }
    /* A revoir */
    case LH_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t target = minirisc->regs[rs_1] + imm;
        uint32_t addr = minirisc->regs[rd];

        platform_read(minirisc->platform, ACCESS_HALF, target, &addr);
        extend_sign(&addr, 11);
        break;
    }
    /* A revoir*/
    case LHU_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t target = minirisc->regs[rs_1] + imm;
        uint32_t addr = minirisc->regs[rs_1];

        platform_read(minirisc->platform, ACCESS_BYTE, target, &addr);
        break;
    }

        /* TODO: Add every instructions */

    default:
    {
        printf("Unknown opcode : %x.\n", opcode);
        minirisc->halt = 1;
        break;
    }
    }

    /* A modifier: comment forcer le registre x0 à 0 */
    minirisc->regs[0] = 0; /* Le registre x0 est cablé en dur à 0 */
}

void extend_sign(uint32_t *imm, int n)
{
    /* sign extension */
    uint32_t sign_bit = 1u << n; /* 1u to prevent undefined behavior */
    uint32_t mask = (1u << (n + 1)) - 1;

    if (*imm & sign_bit) /* Si le bit de signe est à 1 */
    {
        *imm |= ~mask;
    }
    else
    {
        *imm &= mask;
    }
}

void minirisc_run(struct minirisc_t *minirisc)
{
    while (!minirisc->halt)
    {
        minirisc_fetch(minirisc);
        printf("\nInstruction %x at PC: %x\n", minirisc->IR, minirisc->PC);
        minirisc_decode_and_execute(minirisc);
    }
}