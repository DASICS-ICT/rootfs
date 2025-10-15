/*
 * Zicfilp Correct Test - Proper handling of function call restrictions
 *
 * Critical understanding:
 * - The MOMENT Zicfilp is enabled, all function calls become unsafe
 * - Even the next line after syscall(enable) cannot call printf
 * - Must use only syscalls/inline asm until Zicfilp is disabled
 *
 * Timeline:
 * 1. printf("Going to enable...")  <- OK, Zicfilp disabled
 * 2. syscall(enable)                <- OK, it's a syscall
 * 3. [Zicfilp NOW ENABLED]
 * 4. printf("Enabled")              <- CRASH! printf needs LPAD
 * 5. Only syscalls work here        <- OK
 * 6. syscall(disable)               <- OK
 * 7. [Zicfilp NOW DISABLED]
 * 8. printf("Disabled")             <- OK again
 */

#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>

#define PR_RISCV_ZICFILP 1000
#define RISCV_ZICFILP_GET    1
#define RISCV_ZICFILP_SET    2
#define RISCV_ZICFILP_DISABLE 0
#define RISCV_ZICFILP_ENABLE  1

int main(void) {
    int status;

    /* ========== PHASE 1: Before enabling (safe zone) ========== */
    printf("\n============================================\n");
    printf("RISC-V Zicfilp Correct Test\n");
    printf("============================================\n\n");

    status = syscall(SYS_prctl, PR_RISCV_ZICFILP, RISCV_ZICFILP_GET, 0, 0, 0);
    printf("Initial status: %d\n\n", status);

    printf("WARNING: About to enable Zicfilp!\n");
    printf("After enabling, NO function calls until disabled.\n");
    printf("Enabling now...\n\n");

    /* ========== CRITICAL POINT: Enable Zicfilp ========== */
    long ret = syscall(SYS_prctl, PR_RISCV_ZICFILP, RISCV_ZICFILP_SET,
                       RISCV_ZICFILP_ENABLE, 0, 0);

    /*
     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     * DANGER ZONE STARTS HERE - Zicfilp is NOW ENABLED
     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     *
     * DO NOT call any functions (printf, etc.)
     * DO NOT return from main (return needs function epilogue)
     * ONLY use syscalls and inline assembly
     */

    if (ret != 0) {
        /* Enable failed, but we can still use functions */
        printf("ERROR: Failed to enable Zicfilp (ret=%ld)\n", ret);
        return 1;
    }

    /* Use syscall write directly - no function calls */
    const char msg1[] = ">>> Zicfilp is NOW ENABLED <<<\n";
    syscall(SYS_write, 1, msg1, sizeof(msg1) - 1);

    const char msg2[] = "Verifying status via syscall...\n";
    syscall(SYS_write, 1, msg2, sizeof(msg2) - 1);

    /* Get status using syscall */
    status = syscall(SYS_prctl, PR_RISCV_ZICFILP, RISCV_ZICFILP_GET, 0, 0, 0);

    /* Write status using syscall */
    if (status == 1) {
        const char msg3[] = "Status: ENABLED (1)\n\n";
        syscall(SYS_write, 1, msg3, sizeof(msg3) - 1);
    } else {
        const char msg3[] = "ERROR: Status mismatch!\n\n";
        syscall(SYS_write, 1, msg3, sizeof(msg3) - 1);
    }

    const char msg4[] = "Disabling Zicfilp...\n";
    syscall(SYS_write, 1, msg4, sizeof(msg4) - 1);

    /* ========== CRITICAL POINT: Disable Zicfilp ========== */
    syscall(SYS_prctl, PR_RISCV_ZICFILP, RISCV_ZICFILP_SET,
            RISCV_ZICFILP_DISABLE, 0, 0);

    /*
     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     * SAFE ZONE RESTORED - Zicfilp is NOW DISABLED
     * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     *
     * Function calls are safe again
     */

    /* ========== PHASE 2: After disabling (safe zone) ========== */
    printf("\n>>> Zicfilp is NOW DISABLED <<<\n\n");

    status = syscall(SYS_prctl, PR_RISCV_ZICFILP, RISCV_ZICFILP_GET, 0, 0, 0);
    printf("Final status: %d\n\n", status);

    if (status == 0) {
        printf("SUCCESS: Test completed!\n");
        printf("- Enabled Zicfilp successfully\n");
        printf("- Operated safely with syscalls only\n");
        printf("- Disabled Zicfilp successfully\n");
        printf("============================================\n\n");
        return 0;
    } else {
        printf("ERROR: Final status should be 0\n");
        return 1;
    }
}