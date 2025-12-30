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
    cpu->next_PC = initial_PC;
    cpu->platform = platform;
    cpu->regs[0] = 0;
    cpu->halt = 0;

    // if (platform_read(platform, ACCESS_WORD, initial_PC - RAM_BASE, &cpu->IR) == 0)
    // {
    //     free(cpu);
    //     return NULL;
    // }
    cpu->IR = platform->memory[initial_PC - RAM_BASE];

    return cpu;
}

void minirisc_free(struct minirisc_t *minirisc)
{
    free(minirisc);
}

void minirisc_fetch(struct minirisc_t *minirisc)
{
    /* DEBUG */
    // printf("[FETCH] PC=%08x\n", minirisc->PC);
    // for (int reg = 0; reg < 29; reg++)
    // {
    //     printf("Registre x%d: %x\n", reg, minirisc->regs[reg]);
    // }

    platform_read(minirisc->platform, ACCESS_WORD, minirisc->PC, &minirisc->IR);
}

void minirisc_decode_and_execute(struct minirisc_t *minirisc)
{
    uint32_t instr = minirisc->IR;

    uint32_t opcode = instr & 0x7F;

    uint32_t rd = (instr >> 7) & 0x1F;
    uint32_t rs = (instr >> 12) & 0x1F;
    uint32_t rs1 = (instr >> 12) & 0x1F;
    uint32_t rs2 = (instr >> 17) & 0x1F;

    uint32_t imm = (instr >> 20) & 0xFFF;
    uint32_t imm_lui = (instr >> 12) & 0xFFFFF;

    uint32_t shamt = (instr >> 20) & 0x1F;

    minirisc->next_PC = minirisc->PC + 4;

    switch (opcode)
    {
    /* OK */
    case LUI_CODE:
    {
        uint32_t value = imm_lui << 12;
        set_reg(minirisc, rd, value);
        break;
    }
    /* OK */
    case AUIPC_CODE:
    {
        uint32_t value = minirisc->PC + (imm_lui << 12);
        set_reg(minirisc, rd, value);
        break;
    }
    /* OK */
    case JAL_CODE:
    {
        imm_lui <<= 1;
        extend_sign(&imm_lui, 20);

        uint32_t target = minirisc->PC + imm_lui;
        uint32_t value = minirisc->PC + 4;
        set_reg(minirisc, rd, value);
        // printf("next_PC: %d\n", value);

        minirisc->next_PC = target;
        break;
    }
    /* OK */
    case JALR_CODE:
    {
        extend_sign(&imm, 11);

        uint32_t target = minirisc->regs[rs1] + imm;
        target &= ~0x1; /* LSB à 0 */
        uint32_t value = minirisc->PC + 4;
        set_reg(minirisc, rd, value);
        // printf("next_PC: %d\n", value);

        minirisc->next_PC = target;
        break;
    }
    /* OK */
    case BEQ_CODE:
    {
        // printf("BEQ\n");
        imm <<= 1;
        extend_sign(&imm, 12);

        uint32_t target = minirisc->PC + imm;

        if ((int32_t)minirisc->regs[rs1] == (int32_t)minirisc->regs[rd])
        {
            minirisc->next_PC = target;
        }
        break;
    }
    /* OK */
    case BNE_CODE:
    {
        // printf("BNE\n");
        imm <<= 1;
        extend_sign(&imm, 12);

        uint32_t target = minirisc->PC + imm;

        if ((int32_t)minirisc->regs[rs1] != (int32_t)minirisc->regs[rd])
        {
            minirisc->next_PC = target;
        }
        break;
    }
    /* OK */
    case BLT_CODE:
    {
        // printf("BLT\n");
        imm <<= 1;
        extend_sign(&imm, 12);

        uint32_t target = minirisc->PC + imm;

        if ((int32_t)minirisc->regs[rs1] < (int32_t)minirisc->regs[rd])
        {
            minirisc->next_PC = target;
        }
        break;
    }
    /* OK */
    case BGE_CODE:
    {
        // printf("BGE\n");
        imm <<= 1;
        extend_sign(&imm, 12);

        uint32_t target = minirisc->PC + imm;

        if ((int32_t)minirisc->regs[rs1] >= (int32_t)minirisc->regs[rd])
        {
            minirisc->next_PC = target;
        }
        // printf("target: %d\n", target);
        break;
    }
    /* OK */
    case BLTU_CODE:
    {
        // printf("BLTU\n");
        // printf("%x, %x\n", minirisc->regs[rs1], minirisc->regs[rd]);
        // printf("imm before: %x\n", imm);
        imm <<= 1;
        extend_sign(&imm, 12);
        // printf("imm after: %x\n", imm);

        uint32_t target = minirisc->PC + imm;
        // printf("target: %x\n", target);

        if (minirisc->regs[rs1] < minirisc->regs[rd])
        {
            minirisc->next_PC = target;
        }
        break;
    }
    /* OK */
    case BGEU_CODE:
    {
        // printf("BGEU\n");
        // printf("t1 (x%d), a2 (x%d) : %x, %x\n", rs1, rs2, minirisc->regs[rs1], minirisc->regs[rd]);
        imm <<= 1;
        extend_sign(&imm, 12);

        uint32_t target = minirisc->PC + imm;

        if (minirisc->regs[rs1] >= minirisc->regs[rd])
        {
            minirisc->next_PC = target;
        }

        break;
    }
    /* OK */
    case LB_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t mem_addr = minirisc->regs[rs1] + imm;

        platform_read(minirisc->platform, ACCESS_BYTE, mem_addr, &minirisc->regs[rd]);
        extend_sign(&minirisc->regs[rd], 7);
        break;
    }
    /* OK */
    case LH_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t target = minirisc->regs[rs1] + imm;

        platform_read(minirisc->platform, ACCESS_HALF, target, &minirisc->regs[rd]);
        extend_sign(&minirisc->regs[rd], 15);
        break;
    }
    /* OK */
    case LW_CODE:
    {
        // printf("LW\n");
        extend_sign(&imm, 11);
        uint32_t target_data = minirisc->regs[rs1] + imm;
        // printf("rs1, minirisc->regs[rs1], imm, target data : %d, %x, %x, %x\n", rs1, minirisc->regs[rs1], imm, target_data);

        if (platform_read(minirisc->platform, ACCESS_WORD, target_data, &minirisc->regs[rd]) == -1)
        {
            minirisc->halt = 1;
            break;
        }
        break;
    }
    /* OK: a tester */
    case LBU_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t mem_addr = minirisc->regs[rs1] + imm;

        platform_read(minirisc->platform, ACCESS_BYTE, mem_addr, &minirisc->regs[rd]);
        minirisc->regs[rd] &= 0xFF;
        break;
    }
    /* OK */
    case LHU_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t target = minirisc->regs[rs1] + imm;

        platform_read(minirisc->platform, ACCESS_HALF, target, &minirisc->regs[rd]);
        minirisc->regs[rd] &= 0xFFFF;
        break;
    }
    /* OK */
    case SB_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t addr = minirisc->regs[rs1] + imm;
        uint32_t data = minirisc->regs[rd];

        platform_write(minirisc->platform, ACCESS_BYTE, addr, minirisc->regs[rd]);
        break;
    }
    /* OK */
    case SH_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t addr = minirisc->regs[rs1] + imm;
        uint32_t data = minirisc->regs[rd];

        platform_write(minirisc->platform, ACCESS_HALF, addr, data);
        break;
    }
    /* OK */
    case SW_CODE:
    {
        // printf("SW\n");
        extend_sign(&imm, 11);
        uint32_t addr = minirisc->regs[rs1] + imm;
        uint32_t data = minirisc->regs[rd];
        // printf("rs1, rs2, imm : %d, %d, %d\n", rs1, rd, imm);
        // printf("addr: %x\n", addr);
        // printf("data: %d\n", data);

        platform_write(minirisc->platform, ACCESS_WORD, addr, data);
        break;
    }
    /* OK */
    case ADDI_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t value = minirisc->regs[rs1] + imm;
        set_reg(minirisc, rd, value);
        // printf("value: %d\n", value);
        break;
    }
    /* A revoir */
    case SLTI_CODE:
    {
        extend_sign(&imm, 11);

        if ((int32_t)minirisc->regs[rs1] < (int32_t)imm)
        {
            uint32_t value = 1;
            set_reg(minirisc, rd, value);
        }
        else
        {
            uint32_t value = 0;
            set_reg(minirisc, rd, value);
        }
        break;
    }
    /* A revoir */
    case SLTIU_CODE:
    {
        extend_sign(&imm, 11);

        if (minirisc->regs[rs1] < (uint32_t)imm)
        {
            uint32_t value = 1;
            set_reg(minirisc, rd, value);
        }
        else
        {
            uint32_t value = 0;
            set_reg(minirisc, rd, value);
        }
        break;
    }
    /* OK */
    case XORI_CODE:
    {
        extend_sign(&imm, 11);

        uint32_t value = (minirisc->regs[rs1] & (0xFFFFFFFF - imm)) | ((0xFFFFFFFF - minirisc->regs[rs1]) & imm);
        set_reg(minirisc, rd, value);
        break;
    }
    /* OK */
    case ORI_CODE:
    {
        extend_sign(&imm, 11);

        uint32_t value = minirisc->regs[rs1] | imm;
        set_reg(minirisc, rd, value);
        break;
    }
    /* OK */
    case ANDI_CODE:
    {
        extend_sign(&imm, 11);

        uint32_t value = minirisc->regs[rs1] & imm;
        set_reg(minirisc, rd, value);
        break;
    }
    /* OK */
    case SLLI_CODE:
    {
        uint32_t value = minirisc->regs[rs1] << shamt;
        set_reg(minirisc, rd, value);
        break;
    }
    /* OK */
    case SRLI_CODE:
    {
        uint32_t value = minirisc->regs[rs1] >> shamt;
        set_reg(minirisc, rd, value);
        break;
    }
    /* OK */
    case SRAI_CODE:
    {
        uint32_t value = (uint32_t)((int32_t)minirisc->regs[rs1] >> shamt);
        set_reg(minirisc, rd, value);
        break;
    }
    /* OK */
    case ADD_CODE:
    {
        uint32_t value = minirisc->regs[rs1] + minirisc->regs[rs2];
        set_reg(minirisc, rd, value);
        break;
    }
    /* OK */
    case SUB_CODE:
    {
        uint32_t value = minirisc->regs[rs1] - minirisc->regs[rs2];
        set_reg(minirisc, rd, value);
        break;
    }
    /* OK */
    case SLL_CODE:
    {
        uint32_t value = minirisc->regs[rs1] << (minirisc->regs[rs2] & 0x1F);
        set_reg(minirisc, rd, value);
        break;
    }
    /* OK */
    case SRL_CODE:
    {
        uint32_t value = minirisc->regs[rs1] >> (minirisc->regs[rs2] & 0x1F);
        set_reg(minirisc, rd, value);
        break;
    }
    /* OK */
    case SRA_CODE:
    {
        uint32_t value = (uint32_t)((int32_t)minirisc->regs[rs1] >> (minirisc->regs[rs2] & 0x1F));
        set_reg(minirisc, rd, value);
        break;
    }
    /* OK */
    case SLT_CODE:
    {
        if ((int32_t)minirisc->regs[rs1] < (int32_t)minirisc->regs[rs2])
        {
            uint32_t value = 1;
            set_reg(minirisc, rd, value);
        }
        else
        {
            uint32_t value = 0;
            set_reg(minirisc, rd, value);
        }
        break;
    }
    /* OK */
    case SLTU_CODE:
    {
        if (minirisc->regs[rs1] < minirisc->regs[rs2])
        {
            uint32_t value = 1;
            set_reg(minirisc, rd, value);
        }
        else
        {
            uint32_t value = 0;
            set_reg(minirisc, rd, value);
        }
        break;
    }
    /* OK */
    case XOR_CODE:
    {
        uint32_t value = (minirisc->regs[rs1] & (0xFFFFFFFF - minirisc->regs[rs2])) | ((0xFFFFFFFF - minirisc->regs[rs1]) & minirisc->regs[rs2]);
        set_reg(minirisc, rd, value);
        break;
    }
    /* OK */
    case OR_CODE:
    {
        uint32_t value = minirisc->regs[rs1] | minirisc->regs[rs2];
        set_reg(minirisc, rd, value);
        break;
    }
    /* OK */
    case AND_CODE:
    {
        uint32_t value = minirisc->regs[rs1] & minirisc->regs[rs2];
        set_reg(minirisc, rd, value);
        break;
    }
    /* OK */
    case ECALL_CODE:
    {
        set_reg(minirisc, 10, -1);
        break;
    }
    /* OK */
    case EBREAK_CODE:
    {
        minirisc->halt = 1;
        break;
    }
    /* OK */
    case MUL_CODE:
    {
        uint32_t value = minirisc->regs[rs1] * minirisc->regs[rs2];
        set_reg(minirisc, rd, value);
        // printf("value: %d\n", value);
        break;
    }
    /* A revoir */
    case MULH_CODE:
    {
        uint32_t value = ((int64_t)minirisc->regs[rs1] * (int64_t)minirisc->regs[rs2]) >> 32;
        set_reg(minirisc, rd, value);
        break;
    }
    /* A revoir */
    case MULHU_CODE:
    {
        uint32_t value = ((uint64_t)minirisc->regs[rs1] * (uint64_t)minirisc->regs[rs2]) >> 32;
        set_reg(minirisc, rd, value);
        break;
    }
    /* A revoir */
    case MULHSU_CODE:
    {
        uint32_t value = (((int64_t)minirisc->regs[rs1] * (uint64_t)minirisc->regs[rs2]) >> 32);
        set_reg(minirisc, rd, value);
        break;
    }
    /* A revoir */
    case DIV_CODE:
    {
        if (minirisc->regs[rs2] == 0)
        {
            uint32_t value = -1;
            set_reg(minirisc, rd, value);
        }
        else if (minirisc->regs[rs1] == -INT_MAX && minirisc->regs[rs2] == (uint32_t)-1)
        {
            uint32_t value = -INT_MAX;
            set_reg(minirisc, rd, value);
        }
        else
        {
            uint32_t value = minirisc->regs[rs1] / minirisc->regs[rs2];
            set_reg(minirisc, rd, value);
        }
        break;
    }
    /* A revoir */
    case REM_CODE:
    {
        if (minirisc->regs[rs2] == 0)
        {
            uint32_t value = minirisc->regs[rs1];
            set_reg(minirisc, rd, value);
        }
        else if (minirisc->regs[rs1] == -INT_MAX && minirisc->regs[rs2] == (uint32_t)-1)
        {
            uint32_t value = 0;
            set_reg(minirisc, rd, value);
        }
        else
        {
            uint32_t value = minirisc->regs[rs1] - minirisc->regs[rs2] * ((int32_t)minirisc->regs[rs1] / (int32_t)minirisc->regs[rs2]);
            set_reg(minirisc, rd, value);
        }
        break;
    }
    /* A revoir */
    case DIVU_CODE:
    {
        if (minirisc->regs[rs2] == 0)
        {
            uint32_t value = 2 * INT_MAX - 1;
            set_reg(minirisc, rd, value);
        }
        else
        {
            uint32_t value = (int32_t)(minirisc->regs[rs1] / minirisc->regs[rs2]);
            set_reg(minirisc, rd, value);
        }
        break;
    }
    /* A revoir */
    case REMU_CODE:
    {
        if (minirisc->regs[rs2] == 0)
        {
            uint32_t value = minirisc->regs[rs1];
            set_reg(minirisc, rd, value);
        }
        else
        {
            uint32_t value = minirisc->regs[rs1] - minirisc->regs[rs2] * (int32_t)((int32_t)minirisc->regs[rs1] / (int32_t)minirisc->regs[rs2]);
            set_reg(minirisc, rd, value);
        }
        break;
    }

    default:
    {
        printf("Unknown opcode : %x\n", opcode);
        minirisc->halt = 1;
        break;
    }
    }
}

void extend_sign(uint32_t *imm, int n)
{
    *imm = (uint32_t)((int32_t)(*imm << (32 - (n + 1))) >> (32 - (n + 1)));
}

void set_reg(struct minirisc_t *minirisc, int reg, uint32_t value)
{
    minirisc->regs[reg] = value & ((reg == 0) - 1); /* We force the reg[0] to 0 during the writing */
}

void minirisc_run(struct minirisc_t *minirisc)
{
    int nb_instr = 1000000;
    int start = 10;
    int instr = 0;

    while (!minirisc->halt)
    {
        minirisc_fetch(minirisc);
        minirisc_decode_and_execute(minirisc);
        minirisc->PC = minirisc->next_PC;

        /* DEBUG */
        // if ((minirisc->IR & 0x7F) == BLTU_CODE)
        // {
        //     printf("BLTU :\n");
        //     printf("Instruction %x at PC: %x\n", minirisc->IR, minirisc->PC);
        // }
        // if (minirisc->PC == 0x8000004c)
        // {
        //     printf("[MAIN]");
        //     exit(0);
        // }

        // if ((instr >= start && instr <= start + nb_instr) && nb_instr != 0)
        // {
        //     printf("--------------------------------------------------------------");
        //     printf("\nInstruction %x at PC: %x\n", minirisc->IR, minirisc->PC);
        //     printf("Registres ----------------------------------------------------\n");
        //     for (int reg = 0; reg < 29; reg++)
        //     {
        //         printf("Registre %d: %x\n", reg, minirisc->regs[reg]);
        //     }
        // }
        // else if ((instr > start + nb_instr) && nb_instr != 0)
        // {
        //     exit(0);
        // }
        instr += 1;
        // printf("instruction : %d\n", instr);
    }
}