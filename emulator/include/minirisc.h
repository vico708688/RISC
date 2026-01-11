#ifndef H_MINIRISC
#define H_MINIRISC

#include <inttypes.h>
#include "platform.h"

struct minirisc_t
{
    uint32_t PC;       /* Program counter: address of the instruction IR */
    uint32_t IR;       /* Instruction register: instruction being executed */
    uint32_t next_PC;  /* Value used to update the PC after the exec stage */
    uint32_t regs[32]; /* Registers */
    struct platform_t *platform;
    int halt;
};

/**
 * Allocate and initializes a new `minirisc_t` object.
 */
struct minirisc_t *minirisc_new(uint32_t initial_PC, struct platform_t *platform);

/**
 * Cleanup and free a `minirisc_t` object.
 */
void minirisc_free(struct minirisc_t *minirisc);

/**
 * Read the instruction pointed to by PC and place it in IR
 */
void minirisc_fetch(struct minirisc_t *minirisc);

/**
 * Decode the instruction in IR and execute it
 */
void minirisc_decode_and_execute(struct minirisc_t *minirisc);

/**
 * Run the processor:
 * minirisc_fetch() and minirisc_decode_and_execute()
 * in a loop while halt is false.
 */
void minirisc_run(struct minirisc_t *minirisc);

void extend_sign(uint32_t *imm, int n);

void set_reg(struct minirisc_t *minirisc, int reg, uint32_t value);

#endif