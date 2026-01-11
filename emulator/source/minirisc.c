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

    /* Should I use platform_read ? */
    cpu->IR = platform->memory[initial_PC - RAM_BASE];

    return cpu;
}

void minirisc_free(struct minirisc_t *minirisc)
{
    free(minirisc);
}

void minirisc_fetch(struct minirisc_t *minirisc)
{
    platform_read(minirisc->platform, ACCESS_WORD, minirisc->PC, &minirisc->IR);
}

void minirisc_decode_and_execute(struct minirisc_t *minirisc)
{
    uint32_t instr = minirisc->IR;

    uint32_t opcode = instr & 0x7F;

    uint32_t rd = (instr >> 7) & 0x1F;
    uint32_t rs1 = (instr >> 12) & 0x1F;
    uint32_t rs2 = (instr >> 17) & 0x1F;

    uint32_t imm = (instr >> 20) & 0xFFF;
    uint32_t imm_lui = (instr >> 12) & 0xFFFFF;

    uint32_t shamt = (instr >> 20) & 0x1F;

    minirisc->next_PC = minirisc->PC + 4;

    switch (opcode)
    {
    case LUI_CODE:
    {
        uint32_t value = imm_lui << 12;
        set_reg(minirisc, rd, value);
        break;
    }
    case AUIPC_CODE:
    {
        uint32_t value = minirisc->PC + (imm_lui << 12);
        set_reg(minirisc, rd, value);
        break;
    }
    case JAL_CODE:
    {
        imm_lui <<= 1;
        extend_sign(&imm_lui, 20);

        uint32_t target = minirisc->PC + imm_lui;
        uint32_t value = minirisc->PC + 4;
        set_reg(minirisc, rd, value);

        minirisc->next_PC = target;
        break;
    }
    case JALR_CODE:
    {
        extend_sign(&imm, 11);

        uint32_t target = minirisc->regs[rs1] + imm;
        target &= ~0x1; /* LSB à 0 */
        uint32_t value = minirisc->PC + 4;
        set_reg(minirisc, rd, value);

        minirisc->next_PC = target;
        break;
    }
    case BEQ_CODE:
    {
        imm <<= 1;
        extend_sign(&imm, 12);

        uint32_t target = minirisc->PC + imm;

        if ((int32_t)minirisc->regs[rs1] == (int32_t)minirisc->regs[rd])
        {
            minirisc->next_PC = target;
        }
        break;
    }
    case BNE_CODE:
    {
        imm <<= 1;
        extend_sign(&imm, 12);

        uint32_t target = minirisc->PC + imm;

        if ((int32_t)minirisc->regs[rs1] != (int32_t)minirisc->regs[rd])
        {
            minirisc->next_PC = target;
        }
        break;
    }
    case BLT_CODE:
    {
        imm <<= 1;
        extend_sign(&imm, 12);

        uint32_t target = minirisc->PC + imm;

        if ((int32_t)minirisc->regs[rs1] < (int32_t)minirisc->regs[rd])
        {
            minirisc->next_PC = target;
        }
        break;
    }
    case BGE_CODE:
    {
        imm <<= 1;
        extend_sign(&imm, 12);

        uint32_t target = minirisc->PC + imm;

        if ((int32_t)minirisc->regs[rs1] >= (int32_t)minirisc->regs[rd])
        {
            minirisc->next_PC = target;
        }
        break;
    }
    case BLTU_CODE:
    {
        imm <<= 1;
        extend_sign(&imm, 12);

        uint32_t target = minirisc->PC + imm;

        if (minirisc->regs[rs1] < minirisc->regs[rd])
        {
            minirisc->next_PC = target;
        }
        break;
    }
    case BGEU_CODE:
    {
        imm <<= 1;
        extend_sign(&imm, 12);

        uint32_t target = minirisc->PC + imm;

        if (minirisc->regs[rs1] >= minirisc->regs[rd])
        {
            minirisc->next_PC = target;
        }

        break;
    }
    case LB_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t mem_addr = minirisc->regs[rs1] + imm;

        platform_read(minirisc->platform, ACCESS_BYTE, mem_addr, &minirisc->regs[rd]);
        extend_sign(&minirisc->regs[rd], 7);
        break;
    }
    case LH_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t target = minirisc->regs[rs1] + imm;

        platform_read(minirisc->platform, ACCESS_HALF, target, &minirisc->regs[rd]);
        extend_sign(&minirisc->regs[rd], 15);
        break;
    }
    case LW_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t target_data = minirisc->regs[rs1] + imm;

        if (platform_read(minirisc->platform, ACCESS_WORD, target_data, &minirisc->regs[rd]) == -1)
        {
            minirisc->halt = 1;
            break;
        }
        break;
    }
    case LBU_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t mem_addr = minirisc->regs[rs1] + imm;

        platform_read(minirisc->platform, ACCESS_BYTE, mem_addr, &minirisc->regs[rd]);
        minirisc->regs[rd] &= 0xFF;
        break;
    }
    case LHU_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t target = minirisc->regs[rs1] + imm;

        platform_read(minirisc->platform, ACCESS_HALF, target, &minirisc->regs[rd]);
        minirisc->regs[rd] &= 0xFFFF;
        break;
    }
    case SB_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t addr = minirisc->regs[rs1] + imm;

        platform_write(minirisc->platform, ACCESS_BYTE, addr, minirisc->regs[rd]);
        break;
    }
    case SH_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t addr = minirisc->regs[rs1] + imm;
        uint32_t data = minirisc->regs[rd];

        platform_write(minirisc->platform, ACCESS_HALF, addr, data);
        break;
    }
    case SW_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t addr = minirisc->regs[rs1] + imm;
        uint32_t data = minirisc->regs[rd];

        platform_write(minirisc->platform, ACCESS_WORD, addr, data);
        break;
    }
    case ADDI_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t value = minirisc->regs[rs1] + imm;
        set_reg(minirisc, rd, value);
        break;
    }
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
    case XORI_CODE:
    {
        extend_sign(&imm, 11);

        uint32_t value = (minirisc->regs[rs1] & (0xFFFFFFFF - imm)) | ((0xFFFFFFFF - minirisc->regs[rs1]) & imm);
        set_reg(minirisc, rd, value);
        break;
    }
    case ORI_CODE:
    {
        extend_sign(&imm, 11);

        uint32_t value = minirisc->regs[rs1] | imm;
        set_reg(minirisc, rd, value);
        break;
    }
    case ANDI_CODE:
    {
        extend_sign(&imm, 11);

        uint32_t value = minirisc->regs[rs1] & imm;
        set_reg(minirisc, rd, value);
        break;
    }
    case SLLI_CODE:
    {
        uint32_t value = minirisc->regs[rs1] << shamt;
        set_reg(minirisc, rd, value);
        break;
    }
    case SRLI_CODE:
    {
        uint32_t value = minirisc->regs[rs1] >> shamt;
        set_reg(minirisc, rd, value);
        break;
    }
    case SRAI_CODE:
    {
        uint32_t value = (uint32_t)((int32_t)minirisc->regs[rs1] >> shamt);
        set_reg(minirisc, rd, value);
        break;
    }
    case ADD_CODE:
    {
        uint32_t value = minirisc->regs[rs1] + minirisc->regs[rs2];
        set_reg(minirisc, rd, value);
        break;
    }
    case SUB_CODE:
    {
        uint32_t value = minirisc->regs[rs1] - minirisc->regs[rs2];
        set_reg(minirisc, rd, value);
        break;
    }
    case SLL_CODE:
    {
        uint32_t value = minirisc->regs[rs1] << (minirisc->regs[rs2] & 0x1F);
        set_reg(minirisc, rd, value);
        break;
    }
    case SRL_CODE:
    {
        uint32_t value = minirisc->regs[rs1] >> (minirisc->regs[rs2] & 0x1F);
        set_reg(minirisc, rd, value);
        break;
    }
    case SRA_CODE:
    {
        uint32_t value = (uint32_t)((int32_t)minirisc->regs[rs1] >> (minirisc->regs[rs2] & 0x1F));
        set_reg(minirisc, rd, value);
        break;
    }
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
    case XOR_CODE:
    {
        uint32_t value = (minirisc->regs[rs1] & (0xFFFFFFFF - minirisc->regs[rs2])) | ((0xFFFFFFFF - minirisc->regs[rs1]) & minirisc->regs[rs2]);
        set_reg(minirisc, rd, value);
        break;
    }
    case OR_CODE:
    {
        uint32_t value = minirisc->regs[rs1] | minirisc->regs[rs2];
        set_reg(minirisc, rd, value);
        break;
    }
    case AND_CODE:
    {
        uint32_t value = minirisc->regs[rs1] & minirisc->regs[rs2];
        set_reg(minirisc, rd, value);
        break;
    }
    case ECALL_CODE:
    {
        set_reg(minirisc, 10, -1);
        break;
    }
    case EBREAK_CODE:
    {
        minirisc->halt = 1;
        break;
    }
    case MUL_CODE:
    {
        uint32_t value = minirisc->regs[rs1] * minirisc->regs[rs2];
        set_reg(minirisc, rd, value);
        break;
    }
    case MULH_CODE:
    {
        uint32_t value = ((int64_t)minirisc->regs[rs1] * (int64_t)minirisc->regs[rs2]) >> 32;
        set_reg(minirisc, rd, value);
        break;
    }
    case MULHU_CODE:
    {
        uint32_t value = ((uint64_t)minirisc->regs[rs1] * (uint64_t)minirisc->regs[rs2]) >> 32;
        set_reg(minirisc, rd, value);
        break;
    }
    case MULHSU_CODE:
    {
        uint32_t value = (((int64_t)minirisc->regs[rs1] * (uint64_t)minirisc->regs[rs2]) >> 32);
        set_reg(minirisc, rd, value);
        break;
    }
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
        printf("\n========================================\n");
        printf("Unknown opcode   : 0x%02x (%d)\n", opcode, opcode);
        printf("Full instruction : 0x%08x\n", instr);
        printf("At PC            : 0x%08x\n", minirisc->PC);
        printf("rd=%d, rs1=%d, rs2=%d\n", rd, rs1, rs2);
        printf("funct3=%d, funct7=%d\n", (instr >> 12) & 0x7, (instr >> 25) & 0x7F);
        printf("========================================\n");
        fflush(stdout);
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
    while (!minirisc->halt)
    {
        minirisc_fetch(minirisc);
        minirisc_decode_and_execute(minirisc);
        minirisc->PC = minirisc->next_PC;
    }
}