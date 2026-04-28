#include "dasics_tagging_test.h"

int ATTR_ULIB_TEXT untrusted_tagged_rw(void *tagged_ptr)
{
	volatile unsigned char *p = (volatile unsigned char *)tagged_ptr;

	p[0] = 0x5a;
	return p[0] == 0x5a ? 0 : -1;
}

int ATTR_ULIB_TEXT untrusted_fake_tag_read(void *tagged_ptr)
{
	unsigned long bad = zimt_addtag((unsigned long)ptr_strip_tag(tagged_ptr), 1);
	volatile unsigned char value = *(volatile unsigned char *)bad;

	return value;
}

int ATTR_ULIB_TEXT untrusted_fake_tag_write(void *tagged_ptr)
{
	unsigned long bad = zimt_addtag((unsigned long)ptr_strip_tag(tagged_ptr), 1);
	volatile unsigned char *p = (volatile unsigned char *)bad;

	*p = 0xa5;
	return 0;
}

int ATTR_ULIB_TEXT untrusted_settag_no_bound(void *addr)
{
	unsigned long tagged = zimt_addtag((unsigned long)addr, 3);

	zimt_settag1(tagged);
	return 0;
}

int ATTR_ULIB_TEXT untrusted_settag_vitt(void *vitt_base)
{
	unsigned long tagged = zimt_addtag((unsigned long)vitt_base, 4);

	zimt_settag1(tagged);
	return 0;
}

int ATTR_ULIB_TEXT untrusted_touch_first(void *tagged_ptr)
{
	volatile unsigned char *p = (volatile unsigned char *)tagged_ptr;

	p[0] ^= 0x3c;
	return 0;
}
