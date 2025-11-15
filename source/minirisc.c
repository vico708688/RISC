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
    uint32_t instruction = minirisc->platform->memory[(PC - RAM_BASE) / 4]; /* uint8 to uint32 */

    minirisc->IR = instruction;
    minirisc->next_PC = PC + 4;
}

void minirisc_decode_and_execute(struct minirisc_t *minirisc)
{
    uint32_t instr = minirisc->IR;

    uint32_t opcode = instr & 0x0000007F; /* apply a mask to get the 7 LSB */

    uint32_t rd = (instr >> 7) & 0x0000001F; /* autre manière : ((instr >> ) >> n) & k; où k = nb bits (hexa) */
    uint32_t rs = (instr >> 12) & 0x0000001F;
    uint32_t rs2 = (instr >> 16) & 0x0000001F;

    uint32_t imm = (instr >> 20) & 0x00000FFF;
    uint32_t imm_lui = (instr >> 12) & 0x000FFFFF;

    uint32_t shamt = (instr >> 20) & 0x0000001F;
    uint32_t shamt_zeros = (instr >> 25) & 0x0000007F;

    uint32_t zeros_3 = (instr >> 16) & 0x00000007;
    uint32_t zeros_10 = (instr >> 22) & 0x000003FF;
    uint32_t zeros_25 = (instr >> 7) & 0x01FFFFFF;

    /* Depends of the opcode's value */
    switch (opcode)
    {
    case LUI_CODE:
        minirisc->regs[rd] = imm_lui << 12;
        break;

    case ADDI_CODE:
        extend_sign(&imm);
        minirisc->regs[rd] = minirisc->regs[rs] + imm;
        break;

    case SW_CODE:
        extend_sign(&imm);

        /* writes in the RAM */
        uint32_t addr = minirisc->regs[rs + imm];
        uint32_t data = minirisc->regs[rd];

        if (platform_write(minirisc->platform, 2, addr, data) == -1)
        {
            printf("An error occured while writing the data to memory.\n");
            minirisc->halt = 1;
            return;
        }

        break;

    case EBREAK_CODE:
        minirisc->halt = 1;
        break;

        /* TODO: Add every instructions */

    default:
        printf("Unknown opcode : %x.\n", opcode);
        break;
    }

    minirisc->PC = minirisc->next_PC;
}

void extend_sign(uint32_t *imm)
{
    /* sign extension */
    if (*imm & 0x800)
        *imm |= 0xFFFFF000;
}

void minirisc_run(struct minirisc_t *minirisc)
{
    while (!minirisc->halt)
    {
        minirisc_fetch(minirisc);
        minirisc_decode_and_execute(minirisc);
    }
}