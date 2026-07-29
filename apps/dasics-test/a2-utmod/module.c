#include <linux/module.h>

extern int a2_outer(void);
extern int a2_callback(void);
extern char a2_untrusted_start[];
extern char a2_untrusted_end[];

EXPORT_SYMBOL(a2_outer);
EXPORT_SYMBOL(a2_callback);
EXPORT_SYMBOL(a2_untrusted_start);
EXPORT_SYMBOL(a2_untrusted_end);

static int __init a2_utmod_init(void)
{
	return 0;
}

static void __exit a2_utmod_exit(void)
{
}

module_init(a2_utmod_init);
module_exit(a2_utmod_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DASICS A2 nested-call untrusted test module");
