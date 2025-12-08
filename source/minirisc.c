#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "minirisc.h"
#include "platform.h"

/* DEBUG */
void print(uint64_t value)
{
    printf("%ld\n", value);
}

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

    uint32_t opcode = instr & 0x7F;

    uint32_t rd = (instr >> 7) & 0x1F;
    uint32_t rs_1 = (instr >> 12) & 0x1F;
    uint32_t rs2 = (instr >> 17) & 0x1F;

    uint32_t imm = (instr >> 20) & 0xFFF;
    uint32_t imm_lui = (instr >> 12) & 0xFFFFF;

    uint32_t shamt = (instr >> 20) & 0x1F;

    switch (opcode)
    {
    /* OK */
    case LUI_CODE:
    {
        uint32_t value = imm_lui << 12;
        set_reg(minirisc, rd, value);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case AUIPC_CODE:
    {
        uint32_t value = minirisc->PC + (imm_lui << 12);
        set_reg(minirisc, rd, value);

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
        uint32_t value = minirisc->PC + 4;
        set_reg(minirisc, rd, value);

        minirisc->PC = target;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case JALR_CODE:
    {
        extend_sign(&imm, 11);

        uint32_t target = minirisc->regs[rs_1] + imm;
        target &= ~0x1; /* LSB à 0 */
        uint32_t value = minirisc->PC + 4;
        set_reg(minirisc, rd, value);

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
    case LB_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t mem_addr = minirisc->regs[rs_1] + imm;
        uint32_t ptr_addr = minirisc->regs[rd];

        platform_read(minirisc->platform, ACCESS_BYTE, mem_addr, &ptr_addr);
        extend_sign(&ptr_addr, 7);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case LH_CODE:
    {
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
    case LW_CODE:
    {
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
    /* OK: a tester */
    case LBU_CODE:
    {
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
    case SB_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t addr = minirisc->regs[rs_1] + imm;
        uint32_t data = minirisc->regs[rd];

        platform_write(minirisc->platform, ACCESS_BYTE, addr, data);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case SH_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t addr = minirisc->regs[rs_1] + imm;
        uint32_t data = minirisc->regs[rd];

        platform_write(minirisc->platform, ACCESS_HALF, addr, data);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case SW_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t addr = minirisc->regs[rs_1] + imm;
        uint32_t data = minirisc->regs[rd];

        platform_write(minirisc->platform, ACCESS_WORD, addr, data);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case ADDI_CODE:
    {
        extend_sign(&imm, 11);
        uint32_t value = minirisc->regs[rs_1] + imm;
        set_reg(minirisc, rd, value);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* A revoir */
    case SLTI_CODE:
    {
        extend_sign(&imm, 11);

        if (minirisc->regs[rs_1] < imm)
        {
            uint32_t value = 1;
            set_reg(minirisc, rd, value);
        }
        else
        {
            uint32_t value = 0;
            set_reg(minirisc, rd, value);
        }

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* A revoir */
    case SLTIU_CODE:
    {
        extend_sign(&imm, 11);

        if (minirisc->regs[rs_1] < (uint32_t)imm)
        {
            uint32_t value = 1;
            set_reg(minirisc, rd, value);
        }
        else
        {
            uint32_t value = 0;
            set_reg(minirisc, rd, value);
        }

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case XORI_CODE:
    {
        extend_sign(&imm, 11);

        uint32_t value = (minirisc->regs[rs_1] & (0xFFFFFFFF - imm)) | ((0xFFFFFFFF - minirisc->regs[rs_1]) & imm);
        set_reg(minirisc, rd, value);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case ORI_CODE:
    {
        extend_sign(&imm, 11);

        uint32_t value = minirisc->regs[rs_1] | imm;
        set_reg(minirisc, rd, value);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case ANDI_CODE:
    {
        extend_sign(&imm, 11);

        uint32_t value = minirisc->regs[rs_1] & imm;
        set_reg(minirisc, rd, value);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case SLLI_CODE:
    {
        uint32_t value = minirisc->regs[rs_1] << shamt;
        set_reg(minirisc, rd, value);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case SRLI_CODE:
    {
        uint32_t value = minirisc->regs[rs_1] >> shamt;
        set_reg(minirisc, rd, value);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case SRAI_CODE:
    {
        uint32_t value = (uint32_t)((int32_t)minirisc->regs[rs_1] >> shamt);
        set_reg(minirisc, rd, value);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case ADD_CODE:
    {
        uint32_t value = minirisc->regs[rs_1] + minirisc->regs[rs2];
        set_reg(minirisc, rd, value);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case SUB_CODE:
    {
        uint32_t value = minirisc->regs[rs_1] - minirisc->regs[rs2];
        set_reg(minirisc, rd, value);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case SLL_CODE:
    {
        uint32_t value = minirisc->regs[rs_1] << (minirisc->regs[rs2] & 0x1F);
        set_reg(minirisc, rd, value);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case SRL_CODE:
    {
        uint32_t value = minirisc->regs[rs_1] >> (minirisc->regs[rs2] & 0x1F);
        set_reg(minirisc, rd, value);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case SRA_CODE:
    {
        uint32_t value = (uint32_t)((int32_t)minirisc->regs[rs_1] >> (minirisc->regs[rs2] & 0x1F));
        set_reg(minirisc, rd, value);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case SLT_CODE:
    {
        if ((int32_t)minirisc->regs[rs_1] < (int32_t)minirisc->regs[rs2])
        {
            uint32_t value = 1;
            set_reg(minirisc, rd, value);
        }
        else
        {
            uint32_t value = 0;
            set_reg(minirisc, rd, value);
        }

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case SLTU_CODE:
    {
        if (minirisc->regs[rs_1] < minirisc->regs[rs2])
        {
            uint32_t value = 1;
            set_reg(minirisc, rd, value);
        }
        else
        {
            uint32_t value = 0;
            set_reg(minirisc, rd, value);
        }

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case XOR_CODE:
    {
        uint32_t value = (minirisc->regs[rs_1] & (0xFFFFFFFF - minirisc->regs[rs2])) | ((0xFFFFFFFF - minirisc->regs[rs_1]) & minirisc->regs[rs2]);
        set_reg(minirisc, rd, value);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case OR_CODE:
    {
        uint32_t value = minirisc->regs[rs_1] | minirisc->regs[rs2];
        set_reg(minirisc, rd, value);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case AND_CODE:
    {
        uint32_t value = minirisc->regs[rs_1] & minirisc->regs[rs2];
        set_reg(minirisc, rd, value);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* OK */
    case ECALL_CODE:
    {
        set_reg(minirisc, 10, -1);

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
    case MUL_CODE:
    {
        uint32_t value = minirisc->regs[rs_1] * minirisc->regs[rs2];
        set_reg(minirisc, rd, value);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* A revoir */
    case MULH_CODE:
    {
        uint32_t value = ((int64_t)minirisc->regs[rs_1] * (int64_t)minirisc->regs[rs2]) >> 32;
        set_reg(minirisc, rd, value);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* A revoir */
    case MULHU_CODE:
    {
        uint32_t value = ((uint64_t)minirisc->regs[rs_1] * (uint64_t)minirisc->regs[rs2]) >> 32;
        set_reg(minirisc, rd, value);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* A revoir */
    case MULHSU_CODE:
    {
        uint32_t value = (((int64_t)minirisc->regs[rs_1] * (uint64_t)minirisc->regs[rs2]) >> 32);
        set_reg(minirisc, rd, value);

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
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
        else if (minirisc->regs[rs_1] == -INT_MAX && minirisc->regs[rs2] == (uint32_t)-1)
        {
            uint32_t value = -INT_MAX;
            set_reg(minirisc, rd, value);
        }
        else
        {
            uint32_t value = minirisc->regs[rs_1] / minirisc->regs[rs2];
            set_reg(minirisc, rd, value);
        }

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* A revoir */
    case REM_CODE:
    {
        if (minirisc->regs[rs2] == 0)
        {
            uint32_t value = minirisc->regs[rs_1];
            set_reg(minirisc, rd, value);
        }
        else if (minirisc->regs[rs_1] == -INT_MAX && minirisc->regs[rs2] == (uint32_t)-1)
        {
            uint32_t value = 0;
            set_reg(minirisc, rd, value);
        }
        else
        {
            uint32_t value = minirisc->regs[rs_1] - minirisc->regs[rs2] * ((int32_t)minirisc->regs[rs_1] / (int32_t)minirisc->regs[rs2]);
            set_reg(minirisc, rd, value);
        }

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
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
            uint32_t value = (int32_t)(minirisc->regs[rs_1] / minirisc->regs[rs2]);
            set_reg(minirisc, rd, value);
        }

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }
    /* A revoir */
    case REMU_CODE:
    {
        if (minirisc->regs[rs2] == 0)
        {
            uint32_t value = minirisc->regs[rs_1];
            set_reg(minirisc, rd, value);
        }
        else
        {
            uint32_t value = minirisc->regs[rs_1] - minirisc->regs[rs2] * (int32_t)((int32_t)minirisc->regs[rs_1] / (int32_t)minirisc->regs[rs2]);
            set_reg(minirisc, rd, value);
        }

        minirisc->PC = minirisc->next_PC;
        minirisc->next_PC = minirisc->PC + 4;
        break;
    }

    default:
    {
        printf("Unknown opcode : %x\n", opcode);
        minirisc->halt = 1;
        break;
    }
    }

    /* next_PC update : a revoir */
    // minirisc->next_PC = minirisc->PC + 4;
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
        // printf("\nInstruction %x at PC: %x\n", minirisc->IR, minirisc->PC);
        minirisc_decode_and_execute(minirisc);
        /* A modifier: Ajouter minirisc->PC = minirisc->next_PC */
    }
}