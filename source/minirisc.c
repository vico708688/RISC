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
    cpu->regs[0] = 0;
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

    platform_read(minirisc->platform, ACCESS_WORD, PC, &minirisc->IR);

    minirisc->next_PC = PC + 4;
}

void minirisc_decode_and_execute(struct minirisc_t *minirisc)
{
    uint32_t instr = minirisc->IR;

    uint32_t opcode = instr & 0x7F; /* apply a mask to get the 7 LSB */

    uint32_t rd = (instr >> 7) & 0x1F;
    uint32_t rs_1 = (instr >> 12) & 0x1F;
    uint32_t rs2 = (instr >> 17) & 0x1F;

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
        minirisc->regs[rd] = imm_lui << 12;

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
        // printf("JALR\n");
        extend_sign(&imm, 11);

        uint32_t target = minirisc->regs[rs_1] + imm;
        target &= ~0x1; /* LSB à 0 */
        minirisc->regs[rd] = minirisc->PC + 4;
        // printf("rd, minirisc->regs[rd]: %x, %x\n", rd, minirisc->regs[rd]);

        minirisc->PC = target;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case BEQ_CODE:
    {
        imm <<= 1;
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
    /* OK */
    case BLT_CODE:
    {
        imm <<= 1;
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
    /* OK */
    case BGE_CODE:
    {
        // printf("BGE\n");
        imm <<= 1;
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
    /* OK */
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
    /* OK */
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
        // printf("SW\n");
        extend_sign(&imm, 11);
        uint32_t addr = minirisc->regs[rs_1] + imm;
        uint32_t data = minirisc->regs[rd];

        platform_write(minirisc->platform, ACCESS_WORD, addr, data);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case SH_CODE:
    {
        // printf("SH\n");
        extend_sign(&imm, 11);
        uint32_t addr = minirisc->regs[rs_1] + imm;
        uint32_t data = minirisc->regs[rd];

        platform_write(minirisc->platform, ACCESS_HALF, addr, data);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case SB_CODE:
    {
        // printf("SB\n");
        extend_sign(&imm, 11);
        uint32_t addr = minirisc->regs[rs_1] + imm;
        uint32_t data = minirisc->regs[rd];

        platform_write(minirisc->platform, ACCESS_BYTE, addr, data);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case LW_CODE:
    {
        // printf("LW\n");
        extend_sign(&imm, 11);
        uint32_t target_data = minirisc->regs[rs_1] + imm;

        if (platform_read(minirisc->platform, ACCESS_WORD, target_data, &minirisc->regs[rd]) == -1)
        {
            minirisc->halt = 1;
            break;
        }

        printf("%x\n", minirisc->regs[rd]);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* A revoir */
    case LH_CODE:
    {
        // printf("LH\n");
        extend_sign(&imm, 11);
        uint32_t target = minirisc->regs[rs_1] + imm;
        uint32_t addr = minirisc->regs[rd];

        platform_read(minirisc->platform, ACCESS_HALF, target, &addr);
        extend_sign(&addr, 15);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case LHU_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t target = minirisc->regs[rs_1] + imm;
        uint32_t addr = minirisc->regs[rd];

        platform_read(minirisc->platform, ACCESS_HALF, target, &addr);
        addr &= 0xFFFF;

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case LB_CODE:
    {
        // printf("LB\n");
        extend_sign(&imm, 11);
        uint32_t mem_addr = minirisc->regs[rs_1] + imm;
        uint32_t ptr_addr = minirisc->regs[rd];

        platform_read(minirisc->platform, ACCESS_BYTE, mem_addr, &ptr_addr);
        extend_sign(&ptr_addr, 7);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK: a tester */
    case LBU_CODE:
    {
        // printf("LBU\n");
        extend_sign(&imm, 11);
        uint32_t mem_addr = minirisc->regs[rs_1] + imm;
        uint32_t ptr_addr = minirisc->regs[rd];

        platform_read(minirisc->platform, ACCESS_BYTE, mem_addr, &ptr_addr);
        ptr_addr &= 0xFF;

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
    case ECALL_CODE:
    {
        minirisc->regs[10] = -1;

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case MUL_CODE:
    {
        minirisc->regs[rd] = minirisc->regs[rs_1] * minirisc->regs[rs2];

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
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
    // minirisc->regs[0] = 0; /* Le registre x0 est cablé en dur à 0 */
}

void extend_sign(uint32_t *imm, int n)
{
    *imm = (uint32_t)((int32_t)(*imm << (32 - (n + 1))) >> (32 - (n + 1)));
}

void set_reg(int reg, uint32_t value, struct minirisc_t *minirisc)
{
    minirisc->regs[reg] = value & ((reg == 0) - 1);
}

void minirisc_run(struct minirisc_t *minirisc)
{
    while (!minirisc->halt)
    {
        minirisc_fetch(minirisc);
        printf("\nInstruction %x at PC: %x\n", minirisc->IR, minirisc->PC);
        minirisc_decode_and_execute(minirisc);
        /* A modifier: Ajouter minirisc->PC = minirisc->next_PC */
    }
}