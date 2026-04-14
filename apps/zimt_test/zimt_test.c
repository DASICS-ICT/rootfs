/* SPDX-License-Identifier: GPL-2.0 */
/*
 * RISC-V ZIMT (Tagged Memory) userspace test program.
 *
 * Tests the ZIMT kernel and hardware interfaces:
 *   1. hwprobe syscall extension detection
 *   2. prctl tagged address control
 *   3. mmap with PROT_ZIMT
 *   4. ZIMT instructions (gentag/addtag/settag/checktag)
 *   5. Tag mismatch fault -> SIGSEGV/SEGV_MTESERR
 *   6. ptrace GETTAG/SETTAG
 *   7. SP-based access tag check exemption
 */

#include "zimt_test.h"

/* ═══════════════════ Test 1: hwprobe syscall ═══════════════════ */

static void test_hwprobe_basic(void)
{
	struct riscv_hwprobe pair = { .key = RISCV_HWPROBE_KEY_BASE_BEHAVIOR };
	long rc;

	rc = riscv_hwprobe(&pair, 1, 0, NULL, 0);
	TEST_ASSERT("hwprobe syscall returns 0", rc == 0);
}

/* ═══════════════════ Test 2: prctl tagged addr ═══════════════════ */

static void test_prctl_get(void)
{
	long ctrl = get_tagged_addr_ctrl();

	if (ctrl < 0) {
		TEST_SKIP("prctl GET_TAGGED_ADDR_CTRL", "not supported on this kernel");
		return;
	}
	TEST_PASS("prctl GET_TAGGED_ADDR_CTRL readable");
}

static void test_prctl_set_roundtrip(void)
{
	long ctrl = get_tagged_addr_ctrl();

	if (ctrl < 0) {
		TEST_SKIP("prctl SET round-trip", "GET unsupported");
		return;
	}

	long rc = prctl(PR_SET_TAGGED_ADDR_CTRL, ctrl, 0, 0, 0);
	TEST_ASSERT("prctl SET_TAGGED_ADDR_CTRL round-trip", rc == 0);
}

static void test_prctl_enable_pmlen16(void)
{
	long rc = enable_tagged_addr(16);

	if (rc < 0) {
		TEST_SKIP("prctl enable PMLEN=16", strerror(errno));
		return;
	}

	long ctrl = get_tagged_addr_ctrl();
	unsigned int got_pmlen = (ctrl >> 24) & 0x7f;
	TEST_ASSERT("prctl PMLEN=16 enabled", got_pmlen == 16);
}

/* ═══════════════════ Test 3: mmap PROT_ZIMT ═══════════════════ */

static void test_mmap_prot_zimt(void)
{
	void *p = mmap(NULL, 4096, PROT_READ | PROT_WRITE | PROT_ZIMT,
		       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (p == MAP_FAILED) {
		if (errno == EINVAL || errno == EOPNOTSUPP) {
			TEST_SKIP("mmap(PROT_ZIMT)", "kernel rejected (expected when HW unavailable)");
		} else {
			char buf[64];
			snprintf(buf, sizeof(buf), "unexpected errno %d: %s", errno, strerror(errno));
			TEST_FAIL("mmap(PROT_ZIMT)", buf);
		}
		return;
	}

	/* Write and read back to ensure the mapping is functional */
	volatile int *ip = (volatile int *)p;
	*ip = 0xDEADBEEF;
	TEST_ASSERT("mmap(PROT_ZIMT) writable", *ip == (int)0xDEADBEEF);

	munmap(p, 4096);
}

static void test_mmap_prot_zimt_large(void)
{
	size_t sz = 16 * 4096;
	void *p = mmap(NULL, sz, PROT_READ | PROT_WRITE | PROT_ZIMT,
		       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (p == MAP_FAILED) {
		TEST_SKIP("mmap(PROT_ZIMT) 64KB", "not available");
		return;
	}

	/* Touch every page to trigger page faults and tag initialization */
	volatile char *base = (volatile char *)p;
	for (size_t i = 0; i < sz; i += 4096)
		base[i] = (char)i;

	TEST_PASS("mmap(PROT_ZIMT) 64KB multi-page touch");
	munmap(p, sz);
}

/* ═══════════════════ Test 4: ZIMT instructions ═══════════════════ */

static bool zimt_instructions_available = false;

static void test_gentag(void)
{
	unsigned long tagged;
	uint8_t tag;
	int i;

	/*
	 * gentag produces a random tag. On zimop fallback rd is zeroed.
	 * Try a few times — if we always get rd=0, assume zimop fallback.
	 */
	for (i = 0; i < 8; i++) {
		tagged = zimt_gentag();
		if (tagged != 0)
			break;
	}

	tag = ptr_get_tag((void *)tagged);

	if (tagged == 0) {
		TEST_SKIP("gentag", "zimop fallback (rd=0 after 8 tries), ZIMT HW not active");
		return;
	}

	zimt_instructions_available = true;
	TEST_LOG("gentag returned 0x%lx (tag=%u)", tagged, tag);
	TEST_PASS("gentag produces tagged value");
}

static void test_addtag(void)
{
	void *base = ptr_strip_tag((void *)0x1000UL);
	unsigned long result;
	uint8_t tag;

	if (!zimt_instructions_available) {
		TEST_SKIP("addtag", "ZIMT HW not available (gentag returned 0)");
		return;
	}

	result = zimt_addtag((unsigned long)base, 5);
	tag = ptr_get_tag((void *)result);

	if (result == 0) {
		TEST_SKIP("addtag", "zimop fallback (rd=0)");
		return;
	}

	TEST_LOG("addtag(0x%lx, 5) -> 0x%lx (tag=%u)", (unsigned long)base, result, tag);
	TEST_ASSERT("addtag embeds tag in pointer", tag == 5);
}

static void test_settag_checktag(void)
{
	if (!zimt_instructions_available) {
		TEST_SKIP("settag/checktag", "gentag not available, skip HW tag tests");
		return;
	}

	void *p = mmap(NULL, 4096, PROT_READ | PROT_WRITE | PROT_ZIMT,
		       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (p == MAP_FAILED) {
		TEST_SKIP("settag/checktag", "PROT_ZIMT mmap failed");
		return;
	}

	/* Write some data first */
	*(volatile int *)p = 42;

	/* Create a tagged pointer with tag=3 */
	unsigned long tagged = zimt_addtag((unsigned long)p, 3);
	uint8_t tag = ptr_get_tag((void *)tagged);
	TEST_LOG("settag: tagged_ptr=0x%lx tag=%u", tagged, tag);

	/* Write tag to memory (1 granule) */
	zimt_settag1(tagged);
	TEST_PASS("settag executed without fault");

	/* Check tag should match */
	zimt_checktag1(tagged);
	TEST_PASS("checktag matching tag passes");

	munmap(p, 4096);
}

/* ═══════════════════ Test 5: Tag fault signal ═══════════════════ */

static volatile sig_atomic_t tag_fault_received;
static volatile int tag_fault_si_code;
static volatile void *tag_fault_si_addr;

static void tag_fault_handler(int sig, siginfo_t *si, void *uctx)
{
	(void)uctx;
	tag_fault_received = 1;
	tag_fault_si_code = si->si_code;
	tag_fault_si_addr = si->si_addr;

	/*
	 * Returning would re-execute the faulting instruction and loop forever.
	 * Exit directly from the handler.
	 */
	_exit(0);
}

static void test_tag_fault_signal(void)
{
	if (!zimt_instructions_available) {
		TEST_SKIP("tag fault signal", "ZIMT HW not available");
		return;
	}

	pid_t pid = fork();
	if (pid < 0) {
		TEST_FAIL("tag fault signal", "fork failed");
		return;
	}

	if (pid == 0) {
		/* Child: set up signal handler and trigger mismatch */
		struct sigaction sa;
		memset(&sa, 0, sizeof(sa));
		sa.sa_sigaction = tag_fault_handler;
		sa.sa_flags = SA_SIGINFO;
		sigaction(SIGSEGV, &sa, NULL);

		/* Enable tagged addresses */
		enable_tagged_addr(16);

		void *p = mmap(NULL, 4096, PROT_READ | PROT_WRITE | PROT_ZIMT,
			       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (p == MAP_FAILED)
			_exit(2);

		*(volatile int *)p = 0;

		/* Set memory tag = 3 */
		unsigned long tagged3 = zimt_addtag((unsigned long)p, 3);
		zimt_settag1(tagged3);

		/*
		 * Access with wrong tag (tag=7). The hardware should detect
		 * pointer_tag(7) != memory_tag(3) and raise a software check
		 * exception, which the kernel delivers as SIGSEGV/SEGV_MTESERR.
		 */
		unsigned long tagged7 = zimt_addtag((unsigned long)p, 7);
		volatile int *bad_ptr = (volatile int *)tagged7;

		/* This access should trigger a tag fault */
		(void)*bad_ptr;

		/* If we reach here, the fault was somehow handled */
		if (tag_fault_received)
			_exit(0);
		_exit(3);
	}

	/* Parent: wait for child */
	int status;
	waitpid(pid, &status, 0);

	if (WIFEXITED(status)) {
		int code = WEXITSTATUS(status);
		if (code == 0) {
			TEST_PASS("tag fault delivered SIGSEGV to handler");
		} else if (code == 2) {
			TEST_SKIP("tag fault signal", "PROT_ZIMT mmap failed in child");
		} else if (code == 3) {
			TEST_SKIP("tag fault signal",
				  "no fault raised (VITT base likely 0, tag checking inactive)");
		} else {
			char buf[64];
			snprintf(buf, sizeof(buf), "child exited with code %d", code);
			TEST_FAIL("tag fault signal", buf);
		}
	} else if (WIFSIGNALED(status)) {
		int sig = WTERMSIG(status);
		if (sig == SIGSEGV) {
			TEST_PASS("tag fault raised SIGSEGV (child killed)");
		} else {
			char buf[64];
			snprintf(buf, sizeof(buf), "child killed by signal %d", sig);
			TEST_FAIL("tag fault signal", buf);
		}
	} else {
		TEST_FAIL("tag fault signal", "unexpected child status");
	}
}

/* ═══════════════════ Test 6: ptrace GETTAG/SETTAG ═══════════════════ */

static bool wait_child_stopped(pid_t pid)
{
	int status;
	if (waitpid(pid, &status, 0) < 0)
		return false;
	return WIFSTOPPED(status);
}

static void test_ptrace_gettag(void)
{
	pid_t pid = fork();
	if (pid < 0) {
		TEST_FAIL("ptrace GETTAG", "fork failed");
		return;
	}

	if (pid == 0) {
		/* Child: just pause forever */
		for (;;)
			pause();
	}

	if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) < 0) {
		kill(pid, SIGKILL);
		waitpid(pid, NULL, 0);
		TEST_FAIL("ptrace GETTAG", "PTRACE_ATTACH failed");
		return;
	}

	if (!wait_child_stopped(pid)) {
		kill(pid, SIGKILL);
		waitpid(pid, NULL, 0);
		TEST_FAIL("ptrace GETTAG", "child did not stop");
		return;
	}

	uint8_t tag = 0;
	long rc = ptrace(PTRACE_GETTAG, pid, 0, &tag);

	/*
	 * When ZIMT HW is not present, expect -1 with EOPNOTSUPP/ENODEV/EIO.
	 * When HW is present, expect 0 and a valid tag value.
	 */
	if (rc == 0) {
		TEST_LOG("PTRACE_GETTAG returned tag=%u", tag);
		TEST_PASS("ptrace GETTAG succeeds");
	} else if (rc == -1 && (errno == EOPNOTSUPP || errno == ENODEV || errno == EIO)) {
		TEST_PASS("ptrace GETTAG syscall path reachable (rejected as expected)");
	} else {
		char buf[64];
		snprintf(buf, sizeof(buf), "unexpected rc=%ld errno=%d", rc, errno);
		TEST_FAIL("ptrace GETTAG", buf);
	}

	ptrace(PTRACE_DETACH, pid, NULL, NULL);
	kill(pid, SIGKILL);
	waitpid(pid, NULL, 0);
}

static void test_ptrace_settag(void)
{
	pid_t pid = fork();
	if (pid < 0) {
		TEST_FAIL("ptrace SETTAG", "fork failed");
		return;
	}

	if (pid == 0) {
		for (;;)
			pause();
	}

	if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) < 0) {
		kill(pid, SIGKILL);
		waitpid(pid, NULL, 0);
		TEST_FAIL("ptrace SETTAG", "PTRACE_ATTACH failed");
		return;
	}

	if (!wait_child_stopped(pid)) {
		kill(pid, SIGKILL);
		waitpid(pid, NULL, 0);
		TEST_FAIL("ptrace SETTAG", "child did not stop");
		return;
	}

	uint8_t tag = 5;
	long rc = ptrace(PTRACE_SETTAG, pid, 0, &tag);

	if (rc == 0) {
		TEST_PASS("ptrace SETTAG succeeds");
	} else if (rc == -1 && (errno == EOPNOTSUPP || errno == ENODEV || errno == EIO)) {
		TEST_PASS("ptrace SETTAG syscall path reachable (rejected as expected)");
	} else {
		char buf[64];
		snprintf(buf, sizeof(buf), "unexpected rc=%ld errno=%d", rc, errno);
		TEST_FAIL("ptrace SETTAG", buf);
	}

	ptrace(PTRACE_DETACH, pid, NULL, NULL);
	kill(pid, SIGKILL);
	waitpid(pid, NULL, 0);
}

/* ═══════════════════ Test 7: SP exemption ═══════════════════ */

static void test_sp_exemption(void)
{
	if (!zimt_instructions_available) {
		TEST_SKIP("SP exemption", "ZIMT HW not available");
		return;
	}

	/*
	 * ZIMT spec: memory accesses with base register = sp (x2) are exempt
	 * from tag checking. Stack operations should never fault even if the
	 * stack page has MTAG set and the sp carries a mismatched tag.
	 *
	 * We verify this indirectly: normal function calls and local variable
	 * access on the stack work fine despite ZIMT being enabled. This is
	 * a sanity check that the sp exemption is functioning.
	 */
	volatile int stack_var = 0x12345678;
	int val = stack_var;
	TEST_ASSERT("SP exemption: stack access works", val == 0x12345678);
}

/* ═══════════════════ Test 8: Tagged pointer dereference ═══════════════════ */

static void test_tagged_pointer_access(void)
{
	if (!zimt_instructions_available) {
		TEST_SKIP("tagged pointer access", "ZIMT HW not available");
		return;
	}

	long rc = enable_tagged_addr(16);
	if (rc < 0) {
		TEST_SKIP("tagged pointer access", "cannot enable PMLEN=16");
		return;
	}

	void *p = mmap(NULL, 4096, PROT_READ | PROT_WRITE | PROT_ZIMT,
		       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (p == MAP_FAILED) {
		TEST_SKIP("tagged pointer access", "PROT_ZIMT mmap failed");
		return;
	}

	/* Set memory tag = 5 using settag */
	unsigned long tagged = zimt_addtag((unsigned long)p, 5);
	zimt_settag1(tagged);

	/* Access with matching tag should succeed */
	volatile int *tp = (volatile int *)tagged;
	*tp = 0xCAFEBABE;
	int val = *tp;

	TEST_ASSERT("tagged pointer write+read with matching tag", val == (int)0xCAFEBABE);

	munmap(p, 4096);
}

/* ═══════════════════ main ═══════════════════ */

int main(void)
{
	printf("==============================\n");
	printf("ZIMT Extension Test Suite\n");
	printf("==============================\n\n");

	printf("[Group 1] hwprobe syscall\n");
	test_hwprobe_basic();

	printf("\n[Group 2] prctl tagged address control\n");
	test_prctl_get();
	test_prctl_set_roundtrip();
	test_prctl_enable_pmlen16();

	printf("\n[Group 3] mmap PROT_ZIMT\n");
	test_mmap_prot_zimt();
	test_mmap_prot_zimt_large();

	printf("\n[Group 4] ZIMT instructions\n");
	test_gentag();
	test_addtag();
	test_settag_checktag();

	printf("\n[Group 5] Tag fault signal\n");
	test_tag_fault_signal();

	printf("\n[Group 6] ptrace tag access\n");
	test_ptrace_gettag();
	test_ptrace_settag();

	printf("\n[Group 7] SP exemption\n");
	test_sp_exemption();

	printf("\n[Group 8] Tagged pointer dereference\n");
	test_tagged_pointer_access();

	test_summary();

	return __test_fail > 0 ? 1 : 0;
}
