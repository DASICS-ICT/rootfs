#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/kdasics.h>

MODULE_LICENSE("GPL");

int test_bound(uint64_t *array)
{
    uint64_t smaincall_entry = csr_read(0x8b0);
    uint64_t (*p)(SmaincallTypes type, ...);
    p = smaincall_entry;
    p(Smaincall_PRINT, "try to modify array[0] to 3\n");
    array[0] = 3;
    p(Smaincall_PRINT, "modify array[0] success\n");
    p(Smaincall_PRINT, "try to modify array[1] to 4\n");
    array[1] = 4;
    p(Smaincall_PRINT, "modify array[1] success\n");
    p(Smaincall_PRINT, "try to modify array[2] to 5\n");
    array[2] = 5;
    p(Smaincall_PRINT, "modify array[2] success\n");
    return 0;
}
EXPORT_SYMBOL(test_bound);

static int init_utmod(void)
{
    uint64_t smaincall_entry = csr_read(0x8b0);
    uint64_t (*p)(SmaincallTypes type, ...);
    p = smaincall_entry;
    p(Smaincall_PRINT, "utmod init success!\n");
    //pr_info("hello utmod\n");
    return 0;
}

static void exit_utmod(void)
{
    pr_notice("exit tmod\n");
}

module_init(init_utmod);
module_exit(exit_utmod);