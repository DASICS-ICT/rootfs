#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>

#include "dasics_tagging_test.h"

#define TEST_EXIT_TAG   42
#define TEST_EXIT_STORE 43
#define TEST_EXIT_SKIP  77
#define VITT_MIN_SIZE   (1UL << 30)

static int pass_count;
static int fail_count;

static int tag_fault_handler(struct ucontext_trap *regs)
{
	printf("[DASICS_EXCEPTION]: Tag fault: 0x%lx pc: 0x%lx\n",
	       regs->utval, regs->uepc);
	_exit(TEST_EXIT_TAG);
}

static int store_fault_handler(struct ucontext_trap *regs)
{
	printf("[DASICS_EXCEPTION]: Store fault: 0x%lx pc: 0x%lx\n",
	       regs->utval, regs->uepc);
	_exit(TEST_EXIT_STORE);
}

static void report_result(const char *name, int ok)
{
	if (ok) {
		pass_count++;
		printf("[PASS] %s\n", name);
	} else {
		fail_count++;
		printf("[FAIL] %s\n", name);
	}
}

static void setup_udasics(void)
{
	extern char __ULIBTEXT_BEGIN__, __ULIBTEXT_END__;
	uint64_t sp, stack_top;

	register_udasics(0);
	register_utag_fault_handler(tag_fault_handler);
	register_ustore_fault_handler(store_fault_handler);

	dasics_jumpcfg_alloc((uint64_t)&__ULIBTEXT_BEGIN__,
			     (uint64_t)&__ULIBTEXT_END__);

	asm volatile("mv %0, sp" : "=r"(sp));
	stack_top = sp - 128;
	dasics_libcfg_alloc(DASICS_LIBCFG_R | DASICS_LIBCFG_W,
			    stack_top - 4096, stack_top + 4096);
}

static void *alloc_tagged_page(void)
{
	void *p;
	unsigned long tagged;

	p = mmap(NULL, 4096, PROT_READ | PROT_WRITE | PROT_ZIMT,
		 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (p == MAP_FAILED)
		return NULL;

	dasics_libcfg_alloc(DASICS_LIBCFG_R | DASICS_LIBCFG_W,
			    (uint64_t)p, (uint64_t)p + 4096);
	tagged = zimt_addtag((unsigned long)p, 5);
	zimt_settag1(tagged);
	return (void *)tagged;
}

static void *find_vitt_base(void)
{
	FILE *maps;
	char line[256];
	unsigned long best_start = 0;
	unsigned long best_size = 0;

	maps = fopen("/proc/self/maps", "r");
	if (!maps)
		return NULL;

	while (fgets(line, sizeof(line), maps)) {
		unsigned long start, end;
		char perms[5];

		if (sscanf(line, "%lx-%lx %4s", &start, &end, perms) != 3)
			continue;
		if (perms[0] != 'r' || perms[1] != 'w')
			continue;
		if (end > start && end - start > best_size) {
			best_start = start;
			best_size = end - start;
		}
	}

	fclose(maps);
	if (best_size < VITT_MIN_SIZE)
		return NULL;
	return (void *)best_start;
}

static int run_child(const char *name, int (*body)(void), int expect_exit)
{
	pid_t pid;
	int status;

	pid = fork();
	if (pid < 0) {
		printf("[FAIL] %s: fork failed: %s\n", name, strerror(errno));
		fail_count++;
		return -1;
	}

	if (pid == 0)
		_exit(body());

	if (waitpid(pid, &status, 0) < 0) {
		printf("[FAIL] %s: waitpid failed: %s\n", name, strerror(errno));
		fail_count++;
		return -1;
	}

	if (WIFEXITED(status) && WEXITSTATUS(status) == TEST_EXIT_SKIP) {
		printf("[SKIP] %s\n", name);
		return 0;
	}

	report_result(name,
		      (WIFEXITED(status) && WEXITSTATUS(status) == expect_exit) ||
		      (WIFSIGNALED(status) && 128 + WTERMSIG(status) == expect_exit));
	return 0;
}

static int child_normal(void)
{
	void *tagged;

	setup_udasics();
	tagged = alloc_tagged_page();
	if (!tagged)
		return TEST_EXIT_SKIP;

	return lib_call(&untrusted_tagged_rw, tagged) == 0 ? 0 : 1;
}

static int child_fake_read(void)
{
	void *tagged;

	setup_udasics();
	tagged = alloc_tagged_page();
	if (!tagged)
		return TEST_EXIT_SKIP;

	lib_call(&untrusted_fake_tag_read, tagged);
	return 1;
}

static int child_fake_write(void)
{
	void *tagged;

	setup_udasics();
	tagged = alloc_tagged_page();
	if (!tagged)
		return TEST_EXIT_SKIP;

	lib_call(&untrusted_fake_tag_write, tagged);
	return 1;
}

static int child_settag_no_bound(void)
{
	setup_udasics();
	lib_call(&untrusted_settag_no_bound, (void *)&untrusted_tagged_rw);
	return 1;
}

static int child_settag_vitt(void)
{
	void *vitt_base;

	setup_udasics();
	vitt_base = find_vitt_base();
	if (!vitt_base)
		return TEST_EXIT_SKIP;

	lib_call(&untrusted_settag_vitt, vitt_base);
	return 1;
}

static int child_lru_reload(void)
{
	void *first = NULL;

	setup_udasics();

	for (int i = 0; i < 17; ++i) {
		void *tagged = alloc_tagged_page();

		if (!tagged)
			return TEST_EXIT_SKIP;
		if (i == 0)
			first = tagged;
	}

	return lib_call(&untrusted_touch_first, first) == 0 ? 0 : 1;
}

int main(void)
{
	unsigned long ctrl = PR_TAGGED_ADDR_ENABLE | ((unsigned long)ZIMT_PMLEN << 24);

	if (prctl(PR_SET_TAGGED_ADDR_CTRL, ctrl, 0, 0, 0) != 0) {
		printf("[SKIP] PR_SET_TAGGED_ADDR_CTRL failed: %s\n", strerror(errno));
		return 0;
	}

	run_child("normal tagged load/store", child_normal, 0);
	run_child("fake tag read faults via DFR_TF", child_fake_read, TEST_EXIT_TAG);
	run_child("fake tag write faults via DFR_TF", child_fake_write, TEST_EXIT_TAG);
	run_child("settag outside lib bound faults via DFR_SF", child_settag_no_bound,
		  TEST_EXIT_STORE);
	run_child("settag VITT area faults via DFR_SF", child_settag_vitt,
		  TEST_EXIT_STORE);
	run_child("17 buffers reload evicted lib bound", child_lru_reload, 0);

	printf("dasics_tagging_test: %d passed, %d failed\n", pass_count, fail_count);
	return fail_count ? 1 : 0;
}
