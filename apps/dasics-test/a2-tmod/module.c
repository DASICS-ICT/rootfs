#include <linux/kernel.h>
#include <linux/module.h>

#include <asm/csr.h>
#include <asm/kdasics.h>

enum a2_maincall_request {
	A2_REQUEST_CALLBACK = 1,
	A2_REPORT_CALLBACK,
	A2_REPORT_OUTER,
};

extern int a2_outer(void);
extern int a2_callback(void);
extern char a2_untrusted_start[];
extern char a2_untrusted_end[];
extern int a2_call_outer(void *target);
extern int a2_call_callback(void *target);

static unsigned long a2_parent_dretpc;
static unsigned long a2_child_dretpc;
static bool a2_callback_seen;
static bool a2_outer_continued;

static uint64_t a2_maincall(uint64_t request, uint64_t callback,
			    uint64_t unused)
{
	unsigned long dretpc = csr_read(CSR_DRETURNPC);
	int ret;

	(void)unused;

	switch (request) {
	case A2_REQUEST_CALLBACK:
		if (callback != (uint64_t)a2_callback) {
			pr_err("A2: rejected unexpected callback %px\n",
			       (void *)callback);
			return -EINVAL;
		}

		a2_parent_dretpc = dretpc;
		pr_info("A2: trusted gate saved parent dretpc=%px\n",
			(void *)a2_parent_dretpc);
		ret = a2_call_callback(a2_callback);
		a2_child_dretpc = csr_read(CSR_DRETURNPC);
		pr_info("A2: callback returned, current child dretpc=%px\n",
			(void *)a2_child_dretpc);
		csr_write(CSR_DRETURNPC, a2_parent_dretpc);
		pr_info("A2: trusted gate restored parent dretpc=%px\n",
			(void *)csr_read(CSR_DRETURNPC));
		return ret;
	case A2_REPORT_CALLBACK:
		a2_callback_seen = true;
		pr_info("A2: callback observed child dretpc=%px\n",
			(void *)dretpc);
		return 0;
	case A2_REPORT_OUTER:
		a2_outer_continued = true;
		pr_info("A2: outer continued with dretpc=%px\n", (void *)dretpc);
		return 0;
	default:
		pr_err("A2: rejected maincall request %llu\n", request);
		return -EINVAL;
	}
}

static int __init a2_tmod_init(void)
{
	unsigned long stack_top;
	int jump_idx = -1;
	int stack_idx = -1;
	int ret = -EINVAL;

	asm volatile("mv %0, sp" : "=r"(stack_top));

	pr_info("A2: untrusted code range=[%px,%px)\n",
		(void *)a2_untrusted_start, (void *)a2_untrusted_end);
	register_kdasics((uint64_t)a2_maincall);

	jump_idx = dasics_jumpcfg_kalloc((uint64_t)a2_untrusted_start,
					 (uint64_t)a2_untrusted_end);
	if (jump_idx < 0)
		goto out;

	/* Test-only budget for the two small untrusted assembly frames. */
	stack_idx = dasics_libcfg_kalloc(DASICS_LIBCFG_R | DASICS_LIBCFG_W,
					 stack_top, stack_top - 512);
	if (stack_idx < 0)
		goto out;

	pr_info("A2: entering outer, initial dretpc=%px\n",
		(void *)csr_read(CSR_DRETURNPC));
	ret = a2_call_outer(a2_outer);
	pr_info("A2: outer returned to trusted caller, dretpc=%px ret=%d\n",
		(void *)csr_read(CSR_DRETURNPC), ret);

	if (ret || !a2_callback_seen || !a2_outer_continued ||
	    !a2_parent_dretpc || !a2_child_dretpc ||
	    a2_parent_dretpc == a2_child_dretpc) {
		pr_err("A2: FAIL callback=%d outer=%d parent=%px child=%px\n",
		       a2_callback_seen, a2_outer_continued,
		       (void *)a2_parent_dretpc, (void *)a2_child_dretpc);
		ret = -EINVAL;
		goto out;
	}

	pr_info("A2: PASS nested dasicscall overwrote and restored dretpc\n");
out:
	if (stack_idx >= 0)
		dasics_libcfg_kfree(stack_idx);
	if (jump_idx >= 0)
		dasics_jumpcfg_kfree(jump_idx);
	unregister_kdasics();
	return ret;
}

static void __exit a2_tmod_exit(void)
{
}

module_init(a2_tmod_init);
module_exit(a2_tmod_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DASICS A2 software dretpc restore experiment");
