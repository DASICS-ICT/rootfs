#include "uintr_asm.h"

.macro SAVE_UINTR_CONTEXT

  addi sp, sp, -(OFFSET_SIZE)

  /* save all general purpose registers here! */
  // return address
  sd ra, OFFSET_REG_RA(sp)

  // pointers, sp and tp will be handled later
  sd gp, OFFSET_REG_GP(sp)
  sd tp, OFFSET_REG_TP(sp)
  
  // temporary
  sd t0, OFFSET_REG_T0(sp)
  sd t1, OFFSET_REG_T1(sp)
  sd t2, OFFSET_REG_T2(sp)

  // saved register
  sd s0, OFFSET_REG_S0(sp)
  sd s1, OFFSET_REG_S1(sp)

  // args
  sd a0, OFFSET_REG_A0(sp)
  sd a1, OFFSET_REG_A1(sp)
  sd a2, OFFSET_REG_A2(sp)
  sd a3, OFFSET_REG_A3(sp)
  sd a4, OFFSET_REG_A4(sp)
  sd a5, OFFSET_REG_A5(sp)
  sd a6, OFFSET_REG_A6(sp)
  sd a7, OFFSET_REG_A7(sp)

  // saved register
  sd s2, OFFSET_REG_S2(sp)
  sd s3, OFFSET_REG_S3(sp)
  sd s4, OFFSET_REG_S4(sp)
  sd s5, OFFSET_REG_S5(sp)
  sd s6, OFFSET_REG_S6(sp)
  sd s7, OFFSET_REG_S7(sp)
  sd s8, OFFSET_REG_S8(sp)
  sd s9, OFFSET_REG_S9(sp)
  sd s10, OFFSET_REG_S10(sp)
  sd s11, OFFSET_REG_S11(sp)

  // temporary register
  sd t3, OFFSET_REG_T3(sp)
  sd t4, OFFSET_REG_T4(sp)
  sd t5, OFFSET_REG_T5(sp)
  sd t6, OFFSET_REG_T6(sp)

  addi s0, sp, OFFSET_SIZE
  csrrc s1, CSR_USTATUS, t0
  csrr s2,  CSR_UEPC
  sd s0,   OFFSET_REG_SP(sp)
  sd s1,   OFFSET_REG_USTATUS(sp)
  sd s2,   OFFSET_REG_UEPC(sp)
.endm

.macro RESTORE_UINTR_CONTEXT
  /* Restore all registers and uepc,ustatus */
  // priviledge register
  ld t0, OFFSET_REG_USTATUS(sp)
  ld t1, OFFSET_REG_UEPC(sp)
  csrw CSR_USTATUS, t0
  csrw CSR_UEPC, t1

  // return address
  ld ra, OFFSET_REG_RA(sp)

  // pointers, sp will be handled later
  ld gp, OFFSET_REG_GP(sp)
  ld tp, OFFSET_REG_TP(sp)

  // temporary
  ld t0, OFFSET_REG_T0(sp)
  ld t1, OFFSET_REG_T1(sp)
  ld t2, OFFSET_REG_T2(sp)

  // saved register
  ld s0, OFFSET_REG_S0(sp)
  ld s1, OFFSET_REG_S1(sp)

  // args
  ld a0, OFFSET_REG_A0(sp)
  ld a1, OFFSET_REG_A1(sp)
  ld a2, OFFSET_REG_A2(sp)
  ld a3, OFFSET_REG_A3(sp)
  ld a4, OFFSET_REG_A4(sp)
  ld a5, OFFSET_REG_A5(sp)
  ld a6, OFFSET_REG_A6(sp)
  ld a7, OFFSET_REG_A7(sp)

  // saved register
  ld s2, OFFSET_REG_S2(sp)
  ld s3, OFFSET_REG_S3(sp)
  ld s4, OFFSET_REG_S4(sp)
  ld s5, OFFSET_REG_S5(sp)
  ld s6, OFFSET_REG_S6(sp)
  ld s7, OFFSET_REG_S7(sp)
  ld s8, OFFSET_REG_S8(sp)
  ld s9, OFFSET_REG_S9(sp)
  ld s10, OFFSET_REG_S10(sp)
  ld s11, OFFSET_REG_S11(sp)

  // temporary register
  ld t3, OFFSET_REG_T3(sp)
  ld t4, OFFSET_REG_T4(sp)
  ld t5, OFFSET_REG_T5(sp)
  ld t6, OFFSET_REG_T6(sp)

  // sp register
  ld sp,  OFFSET_REG_SP(sp)
.endm

// user interrupt entry (for utimer test)
.section .text
.global u_intr_entry
.type u_intr_entry, @function
.align 2, 0x13  # nop = addi x0, x0, 0


u_intr_entry: 
    SAVE_UINTR_CONTEXT
    call u_intr_handler
    RESTORE_UINTR_CONTEXT
    uret 
