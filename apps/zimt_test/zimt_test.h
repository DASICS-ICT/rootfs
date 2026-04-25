/* SPDX-License-Identifier: GPL-2.0 */
#ifndef ZIMT_TEST_H
#define ZIMT_TEST_H

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* ───────────────────── lightweight test framework ───────────────────── */

static int __test_pass;
static int __test_fail;
static int __test_skip;

#define TEST_LOG(fmt, ...) \
	printf("  " fmt "\n", ##__VA_ARGS__)

#define TEST_PASS(name) do { \
	__test_pass++; \
	printf("  [PASS] %s\n", (name)); \
} while (0)

#define TEST_FAIL(name, reason) do { \
	__test_fail++; \
	printf("  [FAIL] %s: %s\n", (name), (reason)); \
} while (0)

#define TEST_SKIP(name, reason) do { \
	__test_skip++; \
	printf("  [SKIP] %s: %s\n", (name), (reason)); \
} while (0)

#define TEST_ASSERT(name, cond) do { \
	if (cond) TEST_PASS(name); \
	else      TEST_FAIL(name, #cond " is false"); \
} while (0)

static inline void test_summary(void)
{
	printf("\n==============================\n");
	printf("Results: %d passed, %d failed, %d skipped\n",
	       __test_pass, __test_fail, __test_skip);
	printf("==============================\n");
}

/* ───────────────────── syscall / ABI constants ───────────────────── */

#ifndef __NR_riscv_hwprobe
#define __NR_riscv_hwprobe 258
#endif

struct riscv_hwprobe {
	int64_t  key;
	uint64_t value;
};

#define RISCV_HWPROBE_KEY_BASE_BEHAVIOR 3

#ifndef PR_PMLEN_MASK
#define PR_PMLEN_MASK (0x7fUL << 24)
#endif
#ifndef PR_TAGGED_ADDR_ENABLE
#define PR_TAGGED_ADDR_ENABLE (1UL << 31)
#endif

#ifndef PROT_ZIMT
#define PROT_ZIMT 0x20
#endif

#ifndef PTRACE_GETTAG
#define PTRACE_GETTAG 34
#endif
#ifndef PTRACE_SETTAG
#define PTRACE_SETTAG 35
#endif

#ifndef SEGV_MTESERR
#define SEGV_MTESERR 9
#endif

/* ───────────────────── ZIMT instruction encodings ─────────────────── */
/*
 * All ZIMT instructions use the SYSTEM opcode (0x73), funct3=4, and are
 * carved out of the zimop (mop.rr) encoding space.
 *
 * Encoding:  funct7[31:25] 0[24] tag_imm4[23:20] rs1[19:15] 100[14:12] rd[11:7] 1110011[6:0]
 *
 * gentag:   funct7=1000011, rs1=x0, imm4=0, rd=dest  -> generate random tag
 * addtag:   funct7=1000011, rs1=src, imm4=N, rd=dest  -> add imm4 to tag
 * settag:   funct7=1000001, rs1=ptr, imm4=count, rd=0 -> write tag to count+1 granules
 * checktag: funct7=1000011, rs1=ptr, imm4=count, rd=0 -> check tag on count+1 granules
 *
 * We encode them as raw .insn words to avoid toolchain dependency.
 */

/* Build a ZIMT instruction word */
#define ZIMT_INSN(funct7, imm4, rs1, rd) \
	(((funct7) << 25) | (0 << 24) | (((imm4) & 0xf) << 20) | \
	 ((rs1) << 15) | (4 << 12) | ((rd) << 7) | 0x73)

#define REG_ZERO 0
#define REG_A0   10
#define REG_A1   11

/* gentag a0 */
#define ZIMT_GENTAG_A0     ZIMT_INSN(0x43, 0, REG_ZERO, REG_A0)
/* addtag a1, a0, imm4 */
#define ZIMT_ADDTAG_A1_A0(imm4) ZIMT_INSN(0x43, (imm4), REG_A0, REG_A1)
/* settag a0, 0 (one granule) */
#define ZIMT_SETTAG1_A0    ZIMT_INSN(0x41, 0, REG_A0, REG_ZERO)
/* checktag a0, 0 (one granule) */
#define ZIMT_CHECKTAG1_A0  ZIMT_INSN(0x43, 0, REG_A0, REG_ZERO)

/* ──── inline helpers using raw .4byte encoding ──── */

static inline unsigned long zimt_gentag(void)
{
	register unsigned long rd __asm__("a0");
	__asm__ volatile(".4byte %1" : "=r"(rd) : "i"(ZIMT_GENTAG_A0));
	return rd;
}

static inline unsigned long zimt_addtag(unsigned long ptr, int imm4)
{
	register unsigned long rs1 __asm__("a0") = ptr;
	register unsigned long rd  __asm__("a1");
	/*
	 * imm4 must be a compile-time constant for the encoding; provide
	 * common cases via switch.
	 */
	switch (imm4 & 0xf) {
	case 0:  __asm__ volatile(".4byte %1" : "=r"(rd) : "i"(ZIMT_ADDTAG_A1_A0(0)),  "r"(rs1)); break;
	case 1:  __asm__ volatile(".4byte %1" : "=r"(rd) : "i"(ZIMT_ADDTAG_A1_A0(1)),  "r"(rs1)); break;
	case 2:  __asm__ volatile(".4byte %1" : "=r"(rd) : "i"(ZIMT_ADDTAG_A1_A0(2)),  "r"(rs1)); break;
	case 3:  __asm__ volatile(".4byte %1" : "=r"(rd) : "i"(ZIMT_ADDTAG_A1_A0(3)),  "r"(rs1)); break;
	case 4:  __asm__ volatile(".4byte %1" : "=r"(rd) : "i"(ZIMT_ADDTAG_A1_A0(4)),  "r"(rs1)); break;
	case 5:  __asm__ volatile(".4byte %1" : "=r"(rd) : "i"(ZIMT_ADDTAG_A1_A0(5)),  "r"(rs1)); break;
	case 6:  __asm__ volatile(".4byte %1" : "=r"(rd) : "i"(ZIMT_ADDTAG_A1_A0(6)),  "r"(rs1)); break;
	case 7:  __asm__ volatile(".4byte %1" : "=r"(rd) : "i"(ZIMT_ADDTAG_A1_A0(7)),  "r"(rs1)); break;
	case 8:  __asm__ volatile(".4byte %1" : "=r"(rd) : "i"(ZIMT_ADDTAG_A1_A0(8)),  "r"(rs1)); break;
	case 9:  __asm__ volatile(".4byte %1" : "=r"(rd) : "i"(ZIMT_ADDTAG_A1_A0(9)),  "r"(rs1)); break;
	case 10: __asm__ volatile(".4byte %1" : "=r"(rd) : "i"(ZIMT_ADDTAG_A1_A0(10)), "r"(rs1)); break;
	case 11: __asm__ volatile(".4byte %1" : "=r"(rd) : "i"(ZIMT_ADDTAG_A1_A0(11)), "r"(rs1)); break;
	case 12: __asm__ volatile(".4byte %1" : "=r"(rd) : "i"(ZIMT_ADDTAG_A1_A0(12)), "r"(rs1)); break;
	case 13: __asm__ volatile(".4byte %1" : "=r"(rd) : "i"(ZIMT_ADDTAG_A1_A0(13)), "r"(rs1)); break;
	case 14: __asm__ volatile(".4byte %1" : "=r"(rd) : "i"(ZIMT_ADDTAG_A1_A0(14)), "r"(rs1)); break;
	default: __asm__ volatile(".4byte %1" : "=r"(rd) : "i"(ZIMT_ADDTAG_A1_A0(15)), "r"(rs1)); break;
	}
	return rd;
}

static inline void zimt_settag1(unsigned long tagged_ptr)
{
	register unsigned long rs1 __asm__("a0") = tagged_ptr;
	__asm__ volatile(".4byte %0" : : "i"(ZIMT_SETTAG1_A0), "r"(rs1) : "memory");
}

static inline void zimt_checktag1(unsigned long tagged_ptr)
{
	register unsigned long rs1 __asm__("a0") = tagged_ptr;
	__asm__ volatile(".4byte %0" : : "i"(ZIMT_CHECKTAG1_A0), "r"(rs1) : "memory");
}

/* ──── pointer tag manipulation ──── */
/*
 * PMLEN=16: top 16 bits [63:48] are masked for address translation.
 * MT_MODE_4BIT: hardware tag occupies only the top 4 bits [63:60].
 * The gentag/addtag instructions place the tag at bits [63:60].
 */
#define ZIMT_PMLEN       16
#define ZIMT_TAG_WIDTH   4                             /* MT_MODE_4BIT */
#define ZIMT_TAG_SHIFT   (64 - ZIMT_TAG_WIDTH)         /* 60 */
#define ZIMT_TAG_MASK    ((1UL << ZIMT_TAG_WIDTH) - 1)  /* 0xF */
#define ZIMT_GRANULE     16

static inline uint8_t ptr_get_tag(const void *p)
{
	return (uint8_t)(((unsigned long)p >> ZIMT_TAG_SHIFT) & ZIMT_TAG_MASK);
}

static inline void *ptr_set_tag(void *p, uint8_t tag)
{
	unsigned long addr = (unsigned long)p;
	addr = (long)(addr << ZIMT_PMLEN) >> ZIMT_PMLEN;
	addr |= ((unsigned long)(tag & ZIMT_TAG_MASK)) << ZIMT_TAG_SHIFT;
	return (void *)addr;
}

static inline void *ptr_strip_tag(void *p)
{
	unsigned long addr = (unsigned long)p;
	return (void *)((long)(addr << ZIMT_PMLEN) >> ZIMT_PMLEN);
}

/* ───────────────────── syscall wrappers ───────────────────── */

static inline long riscv_hwprobe(struct riscv_hwprobe *pairs, size_t count,
				 size_t cpusetsize, unsigned long *cpus,
				 unsigned int flags)
{
	return syscall(__NR_riscv_hwprobe, pairs, count, cpusetsize, cpus, flags);
}

static inline long enable_tagged_addr(unsigned int pmlen)
{
	unsigned long arg = PR_TAGGED_ADDR_ENABLE |
			    ((unsigned long)pmlen << 24);
	return prctl(PR_SET_TAGGED_ADDR_CTRL, arg, 0, 0, 0);
}

static inline long get_tagged_addr_ctrl(void)
{
	return prctl(PR_GET_TAGGED_ADDR_CTRL, 0, 0, 0, 0);
}

#endif /* ZIMT_TEST_H */
