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

    uint32_t opcode = instr & 0x0000007F; /* apply a mask to get the 7 LSB */

    uint32_t rd = (instr >> 7) & 0x0000001F;
    uint32_t rs = (instr >> 12) & 0x0000001F;
    uint32_t rs2 = (instr >> 16) & 0x0000001F;

    uint32_t imm = (instr >> 20) & 0x00000FFF;
    uint32_t imm_lui = (instr >> 12) & 0x000FFFFF;

    uint32_t shamt = (instr >> 20) & 0x0000001F;
    uint32_t shamt_zeros = (instr >> 25) & 0x0000007F;

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
        break;
    }
    /* OK */
    case ADDI_CODE:
    {
        extend_sign(&imm, 11);
        minirisc->regs[rd] = minirisc->regs[rs] + imm;

        minirisc->PC = minirisc->next_PC;

        break;
    }
    /* OK */
    case EBREAK_CODE:
    {
        minirisc->halt = 1;
        minirisc->PC = minirisc->next_PC;
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
        // Sign extension
        extend_sign(&imm_lui, 20);

        uint32_t target = minirisc->PC + imm_lui;
        minirisc->regs[rd] = minirisc->PC + 4;
        minirisc->PC = target;

        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* A revoir */
    case JALR_CODE:
    {
        // Sign extension
        extend_sign(&imm_lui, 11);

        uint32_t target = minirisc->regs[rs] + imm_lui;
        target &= ~0x1;
        minirisc->regs[rd] = minirisc->PC + 4;
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
        if ((int32_t)minirisc->regs[rs] == (int32_t)minirisc->regs[rd])
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
        if ((int32_t)minirisc->regs[rs] != (int32_t)minirisc->regs[rd])
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
        if ((int32_t)minirisc->regs[rs] < (int32_t)minirisc->regs[rd])
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
        if ((int32_t)minirisc->regs[rs] >= (int32_t)minirisc->regs[rd])
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
        if (minirisc->regs[rs] < minirisc->regs[rd])
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
        if (minirisc->regs[rs] >= minirisc->regs[rd])
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
        uint32_t addr = minirisc->regs[rs] + imm;
        uint32_t data = minirisc->regs[rd];

        platform_write(minirisc->platform, ACCESS_WORD, addr, data);

        minirisc->PC = minirisc->next_PC;
        break;
    }
    /* A revoir */
    case SH_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t addr = minirisc->regs[rs] + imm;
        uint32_t data = minirisc->regs[rd];

        platform_write(minirisc->platform, ACCESS_HALF, addr, data);
        break;
    }
    /* A revoir */
    case SB_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t addr = minirisc->regs[rs] + imm;
        uint32_t data = minirisc->regs[rd];

        platform_write(minirisc->platform, ACCESS_BYTE, addr, data);
        break;
    }
    /* A revoir */
    case LW_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t target = minirisc->regs[rs] + imm;
        uint32_t addr = minirisc->regs[rs];

        platform_read(minirisc->platform, ACCESS_WORD, target, &addr);
        break;
    }
    /* A revoir */
    case LH_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t target = minirisc->regs[rs] + imm;
        uint32_t addr = minirisc->regs[rd];

        platform_read(minirisc->platform, ACCESS_HALF, target, &addr);
        extend_sign(&addr, 11);
        break;
    }
    /* A revoir*/
    case LHU_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t target = minirisc->regs[rs] + imm;
        uint32_t addr = minirisc->regs[rs];

        platform_read(minirisc->platform, ACCESS_BYTE, target, &addr);
        break;
    }

        /* TODO: Add every instructions */

    default:
    {
        printf("Unknown opcode : %x.\n", opcode);
        break;
    }
    }
}

void extend_sign(uint32_t *imm, int n)
{
    /* sign extension */
    uint32_t sign_bit = 1u << n; /* 1u to prevent undefined behavior */
    uint32_t mask = (1u >> (n + 1)) - 1;

    if (*imm & sign_bit)
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
    // int i = 0;
    while (!minirisc->halt)
    {
        minirisc_fetch(minirisc);
        minirisc_decode_and_execute(minirisc);
        // if (i == 20)
        // {
        //     minirisc->halt = 1;
        // }
        // i++;
    }
}