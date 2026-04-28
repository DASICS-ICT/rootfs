#ifndef DASICS_TAGGING_TEST_H
#define DASICS_TAGGING_TEST_H

#include <stdint.h>
#include <sys/types.h>

#include "udasics.h"

#ifndef PROT_ZIMT
#define PROT_ZIMT 0x20
#endif

#ifndef PR_PMLEN_MASK
#define PR_PMLEN_MASK (0x7fUL << 24)
#endif
#ifndef PR_TAGGED_ADDR_ENABLE
#define PR_TAGGED_ADDR_ENABLE (1UL << 31)
#endif

#define ZIMT_PMLEN 16
#define ZIMT_GRANULE 16
#define ZIMT_TAG_SHIFT 60

#define ZIMT_INSN(funct7, imm4, rs1, rd) \
	(((funct7) << 25) | (((imm4) & 0xf) << 20) | ((rs1) << 15) | \
	 (4 << 12) | ((rd) << 7) | 0x73)

#define REG_ZERO 0
#define REG_A0   10
#define REG_A1   11

#define ZIMT_ADDTAG_A1_A0(imm4) ZIMT_INSN(0x43, (imm4), REG_A0, REG_A1)
#define ZIMT_SETTAG1_A0         ZIMT_INSN(0x41, 0, REG_A0, REG_ZERO)
#define ZIMT_CHECKTAG1_A0       ZIMT_INSN(0x43, 0, REG_A0, REG_ZERO)

static inline unsigned long zimt_addtag(unsigned long ptr, int imm4)
{
	register unsigned long rs1 __asm__("a0") = ptr;
	register unsigned long rd __asm__("a1");

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

static inline void *ptr_strip_tag(void *p)
{
	unsigned long addr = (unsigned long)p;
	return (void *)((long)(addr << ZIMT_PMLEN) >> ZIMT_PMLEN);
}

int ATTR_ULIB_TEXT untrusted_tagged_rw(void *tagged_ptr);
int ATTR_ULIB_TEXT untrusted_fake_tag_read(void *tagged_ptr);
int ATTR_ULIB_TEXT untrusted_fake_tag_write(void *tagged_ptr);
int ATTR_ULIB_TEXT untrusted_settag_no_bound(void *addr);
int ATTR_ULIB_TEXT untrusted_settag_vitt(void *vitt_base);
int ATTR_ULIB_TEXT untrusted_touch_first(void *tagged_ptr);

#endif
