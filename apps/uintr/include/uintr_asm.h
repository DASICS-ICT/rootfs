#ifndef _UINTR_ASM_H_
#define _UINTR_ASM_H_
// save / restore regs

#define OFFSET_REG_ZERO         0

/* return address */
#define OFFSET_REG_RA           8

/* pointers */
#define OFFSET_REG_SP           16 // stack
#define OFFSET_REG_GP           24 // global
#define OFFSET_REG_TP           32 // thread

/* temporary */
#define OFFSET_REG_T0           40
#define OFFSET_REG_T1           48
#define OFFSET_REG_T2           56

/* saved register */
#define OFFSET_REG_S0           64
#define OFFSET_REG_S1           72

/* args */
#define OFFSET_REG_A0           80
#define OFFSET_REG_A1           88
#define OFFSET_REG_A2           96
#define OFFSET_REG_A3           104
#define OFFSET_REG_A4           112
#define OFFSET_REG_A5           120
#define OFFSET_REG_A6           128
#define OFFSET_REG_A7           136

/* saved register */
#define OFFSET_REG_S2           144
#define OFFSET_REG_S3           152
#define OFFSET_REG_S4           160
#define OFFSET_REG_S5           168
#define OFFSET_REG_S6           176
#define OFFSET_REG_S7           184
#define OFFSET_REG_S8           192
#define OFFSET_REG_S9           200
#define OFFSET_REG_S10          208
#define OFFSET_REG_S11          216

/* temporary register */
#define OFFSET_REG_T3           224
#define OFFSET_REG_T4           232
#define OFFSET_REG_T5           240
#define OFFSET_REG_T6           248

/* privileged register */
#define OFFSET_REG_USTATUS      256
#define OFFSET_REG_UEPC         264
/* Size of stack frame, word/double word alignment */
#define OFFSET_SIZE             272

#define CSR_USTATUS         0x000
#define CSR_UEPC            0x041

#endif
