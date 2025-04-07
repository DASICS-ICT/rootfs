#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/kdasics.h>
#include <utmod/utmod.h>
#include <linux/pgtable.h>

MODULE_LICENSE("GPL");

extern int test_bound(uint64_t *array);

static int init_tmod(void)
{
    int ret = 0xdead;
    uint64_t array[] = {1, 2, 3};
    uint64_t frame_addr, badfunc_stack_top, lib_lo, lib_hi;
    asm volatile("mv %0, sp" : "=r"(frame_addr));
    badfunc_stack_top = frame_addr - 104;
	lib_lo = badfunc_stack_top - 16;
	lib_hi = badfunc_stack_top + 32;
    pr_info("before klib call: array[0] = %x, array[1] = %x\n", array[0], array[1]);
    pr_info("array[2] = %x and should not be modified\n", array[2]);
    register_kdasics(0);
    //uint32_t jmpcfg_idx = dasics_jumpcfg_kalloc(UT_MODULES_START, UT_MODULES_END);
    uint32_t jmpcfg_idx = dasics_jumpcfg_kalloc(test_bound, test_bound+0x100);
	uint32_t libcfg_idx_0 = dasics_libcfg_kalloc(DASICS_LIBCFG_R | DASICS_LIBCFG_W, array + 2, array);
    uint32_t libcfg_idx_1 = dasics_libcfg_kalloc(DASICS_LIBCFG_R | DASICS_LIBCFG_W, frame_addr, lib_lo);
    pr_info("jmp bound: %lx %lx\n", test_bound, test_bound+0x20);
    pr_info("klib bound 0: %lx %lx\n", array, array + 2);
    pr_info("klib bound 1: %lx %lx\n", lib_lo, frame_addr);
    ret = ret_klib_call(test_bound, array);
    dasics_libcfg_kfree(libcfg_idx_0);
    dasics_libcfg_kfree(libcfg_idx_1);
	dasics_jumpcfg_kfree(jmpcfg_idx);
	unregister_kdasics();

    if (ret)
        pr_info("test falied\n");
    pr_info("after klib call: array[0] = %x, array[1] = %x\n", array[0], array[1]);
    return 0;
}

static void exit_tmod(void)
{
    pr_info("exit tmod\n");
}

module_init(init_tmod);
module_exit(exit_tmod);