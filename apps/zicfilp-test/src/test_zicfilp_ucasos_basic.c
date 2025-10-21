/*
 * UCAS-OS Zicfilp Basic Test
 *
 * Tests enable/disable functionality of Zicfilp using UCAS-OS syscalls.
 *
 * CRITICAL: Must use inline syscall macros (not function calls) when
 * Zicfilp is enabled, as function calls require landing pads.
 */

#include <stdio.h>

/* UCAS-OS system call numbers */
#define SYSCALL_ZICFILP_SET 350
#define SYSCALL_ZICFILP_GET 351
#define SYS_write 64

/*
 * Inline syscall macros - expand directly to ecall instruction
 * NO function call overhead - safe to use with Zicfilp enabled
 */
#define syscall_inline0(n) ({ \
    register long _a7 asm("a7") = (n); \
    register long _a0 asm("a0"); \
    asm volatile("ecall" : "=r"(_a0) : "r"(_a7) : "memory"); \
    _a0; \
})

#define syscall_inline1(n, a1) ({ \
    register long _a7 asm("a7") = (n); \
    register long _a0 asm("a0") = (long)(a1); \
    asm volatile("ecall" : "+r"(_a0) : "r"(_a7) : "memory"); \
    _a0; \
})

#define syscall_inline3(n, a1, a2, a3) ({ \
    register long _a7 asm("a7") = (n); \
    register long _a0 asm("a0") = (long)(a1); \
    register long _a1 asm("a1") = (long)(a2); \
    register long _a2 asm("a2") = (long)(a3); \
    asm volatile("ecall" : "+r"(_a0) : "r"(_a1), "r"(_a2), "r"(_a7) : "memory"); \
    _a0; \
})

int main(void) {
    int status;

    /* ========== PHASE 1: Before enabling (safe zone) ========== */
    printf("\n============================================\n");
    printf("UCAS-OS Zicfilp Basic Test\n");
    printf("============================================\n\n");

    /* Get initial status - should be 0 (disabled) */
    status = syscall_inline0(SYSCALL_ZICFILP_GET);
    printf("Initial Zicfilp status: %d (should be 0)\n\n", status);

    if (status != 0) {
        printf("ERROR: Zicfilp should be disabled initially!\n");
        return 1;
    }

    printf("WARNING: About to enable Zicfilp!\n");
    printf("After enabling, NO function calls allowed.\n");
    printf("Only inline syscalls (direct ecall) work.\n");
    printf("Enabling now...\n\n");

    /* ========== CRITICAL POINT: Enable Zicfilp ========== */
    long ret = syscall_inline1(SYSCALL_ZICFILP_SET, 1);

    /*
     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     * DANGER ZONE STARTS HERE - Zicfilp is NOW ENABLED
     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     *
     * DO NOT call any functions (printf, etc.)
     * ONLY use inline syscall macros
     */

    if (ret != 0) {
        /* Enable failed, but we can still use functions */
        printf("ERROR: Failed to enable Zicfilp (ret=%ld)\n", ret);
        return 1;
    }

    /* Use inline syscall to write - no function calls */
    const char msg1[] = ">>> Zicfilp is NOW ENABLED <<<\n";
    syscall_inline3(SYS_write, 1, (long)msg1, sizeof(msg1) - 1);

    const char msg2[] = "Verifying status via inline syscall...\n";
    syscall_inline3(SYS_write, 1, (long)msg2, sizeof(msg2) - 1);

    /* Get status using inline syscall */
    status = syscall_inline0(SYSCALL_ZICFILP_GET);

    /* Write status using inline syscall */
    if (status == 1) {
        const char msg3[] = "Status: ENABLED (1) - Correct!\n\n";
        syscall_inline3(SYS_write, 1, (long)msg3, sizeof(msg3) - 1);
    } else {
        const char msg3[] = "ERROR: Status mismatch! Expected 1\n\n";
        syscall_inline3(SYS_write, 1, (long)msg3, sizeof(msg3) - 1);
    }

    const char msg4[] = "Disabling Zicfilp...\n";
    syscall_inline3(SYS_write, 1, (long)msg4, sizeof(msg4) - 1);

    /* ========== CRITICAL POINT: Disable Zicfilp ========== */
    syscall_inline1(SYSCALL_ZICFILP_SET, 0);

    /*
     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     * SAFE ZONE RESTORED - Zicfilp is NOW DISABLED
     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     *
     * Function calls are safe again
     */

    /* ========== PHASE 2: After disabling (safe zone) ========== */
    printf("\n>>> Zicfilp is NOW DISABLED <<<\n\n");

    status = syscall_inline0(SYSCALL_ZICFILP_GET);
    printf("Final Zicfilp status: %d (should be 0)\n\n", status);

    if (status == 0) {
        printf("SUCCESS: Test completed!\n");
        printf("- Enabled Zicfilp successfully\n");
        printf("- Operated safely with inline syscalls only\n");
        printf("- Disabled Zicfilp successfully\n");
        printf("============================================\n\n");
        return 0;
    } else {
        printf("ERROR: Final status should be 0, got %d\n", status);
        return 1;
    }
}
