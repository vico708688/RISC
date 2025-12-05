#ifndef H_TYPES
#define H_TYPES

#define CHAROUT_BASE 0x10000000
#define RAM_BASE 0x80000000

#define LUI_CODE 1
#define AUIPC_CODE 2
#define JAL_CODE 3
#define JALR_CODE 4
#define BEQ_CODE 5
#define BNE_CODE 6
#define BLT_CODE 7
#define BGE_CODE 8
#define BLTU_CODE 9
#define BGEU_CODE 10
#define LB_CODE 11
#define LH_CODE 12
#define LW_CODE 13
#define LBU_CODE 14
#define LHU_CODE 15
#define SB_CODE 16
#define SH_CODE 17
#define SW_CODE 18
#define ADDI_CODE 19
#define SLTI_CODE 20
#define SLTIU_CODE 21
#define XORI_CODE 22
#define ORI_CODE 23
#define ANDI_CODE 24
#define SLLI_CODE 25
#define SRLI_CODE 26
#define SRAI_CODE 27
#define ADD_CODE 28
#define SUB_CODE 29
#define SLL_CODE 30
#define SRL_CODE 31
#define SRA_CODE 32
#define SLT_CODE 33
#define SLTU_CODE 34
#define XOR_CODE 35
#define OR_CODE 36
#define AND_CODE 37
#define ECALL_CODE 38
#define EBREAK_CODE 39
#define MUL_CODE 56
#define MULH_CODE 57
#define MULHSU_CODE 58
#define MULHU_CODE 59
#define DIV_CODE 60
#define DIVU_CODE 61
#define REM_CODE 62
#define REMU_CODE 63

#define INT_MAX 0x80000000

#endif