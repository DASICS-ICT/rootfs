/*
 * Zicfilp prctl Interface Test
 *
 * This program tests the prctl interface for enabling/disabling Zicfilp
 * (Landing Pad) protection on a per-process basis.
 *
 * Compile: riscv64-linux-gnu-gcc -o test_prctl test_prctl.c
 * Usage: ./test_prctl
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <errno.h>
#include <string.h>

/* Zicfilp prctl constants (from include/uapi/linux/prctl.h) */
#ifndef PR_RISCV_ZICFILP
#define PR_RISCV_ZICFILP 1000
#endif

#ifndef RISCV_ZICFILP_GET
#define RISCV_ZICFILP_GET    1
#define RISCV_ZICFILP_SET    2
#define RISCV_ZICFILP_DISABLE 0
#define RISCV_ZICFILP_ENABLE  1
#endif

void print_separator(const char *title) {
    printf("\n========== %s ==========\n", title);
}

int get_zicfilp_status(void) {
    int ret = prctl(PR_RISCV_ZICFILP, RISCV_ZICFILP_GET, 0, 0, 0);
    if (ret < 0) {
        fprintf(stderr, "Error: prctl GET failed: %s\n", strerror(errno));
        return -1;
    }
    return ret;
}

int set_zicfilp_status(int enable) {
    int ret = prctl(PR_RISCV_ZICFILP, RISCV_ZICFILP_SET,
                    enable ? RISCV_ZICFILP_ENABLE : RISCV_ZICFILP_DISABLE, 0, 0);
    if (ret < 0) {
        fprintf(stderr, "Error: prctl SET failed: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

void test_get_initial_status(void) {
    print_separator("Test 1: Get Initial Zicfilp Status");

    int status = get_zicfilp_status();
    if (status >= 0) {
        printf("Initial Zicfilp status: %s\n",
               status ? "ENABLED" : "DISABLED");
    }
}

void test_enable_zicfilp(void) {
    print_separator("Test 2: Enable Zicfilp");

    printf("Enabling Zicfilp...\n");
    if (set_zicfilp_status(1) == 0) {
        printf("SUCCESS: Zicfilp enabled\n");

        int status = get_zicfilp_status();
        if (status == 1) {
            printf("VERIFIED: Status is now ENABLED\n");
        } else {
            printf("WARNING: Status mismatch (expected 1, got %d)\n", status);
        }
    }
}

void test_disable_zicfilp(void) {
    print_separator("Test 3: Disable Zicfilp");

    printf("Disabling Zicfilp...\n");
    if (set_zicfilp_status(0) == 0) {
        printf("SUCCESS: Zicfilp disabled\n");

        int status = get_zicfilp_status();
        if (status == 0) {
            printf("VERIFIED: Status is now DISABLED\n");
        } else {
            printf("WARNING: Status mismatch (expected 0, got %d)\n", status);
        }
    }
}

void test_toggle_multiple_times(void) {
    print_separator("Test 4: Toggle Zicfilp Multiple Times");

    for (int i = 0; i < 5; i++) {
        printf("Iteration %d:\n", i + 1);

        printf("  Enabling...\n");
        if (set_zicfilp_status(1) == 0) {
            int status = get_zicfilp_status();
            printf("  Status: %s\n", status ? "ENABLED" : "DISABLED");
        }

        printf("  Disabling...\n");
        if (set_zicfilp_status(0) == 0) {
            int status = get_zicfilp_status();
            printf("  Status: %s\n", status ? "ENABLED" : "DISABLED");
        }
    }

    printf("SUCCESS: Completed %d toggle cycles\n", 5);
}

void test_invalid_operations(void) {
    print_separator("Test 5: Invalid Operations");

    /* Test invalid operation code */
    printf("Testing invalid operation (999)...\n");
    int ret = prctl(PR_RISCV_ZICFILP, 999, 0, 0, 0);
    if (ret < 0 && errno == EINVAL) {
        printf("SUCCESS: Invalid operation rejected (EINVAL)\n");
    } else {
        printf("WARNING: Expected EINVAL, got ret=%d errno=%d\n", ret, errno);
    }

    /* Test invalid value for SET */
    printf("Testing invalid SET value (999)...\n");
    ret = prctl(PR_RISCV_ZICFILP, RISCV_ZICFILP_SET, 999, 0, 0);
    if (ret < 0 && errno == EINVAL) {
        printf("SUCCESS: Invalid SET value rejected (EINVAL)\n");
    } else {
        printf("WARNING: Expected EINVAL, got ret=%d errno=%d\n", ret, errno);
    }
}

int main(int argc, char *argv[]) {
    printf("===============================================\n");
    printf("  RISC-V Zicfilp prctl Interface Test\n");
    printf("===============================================\n");
    printf("Testing prctl() system call for Zicfilp control\n");

    /* Run all tests */
    test_get_initial_status();
    test_enable_zicfilp();
    test_disable_zicfilp();
    test_toggle_multiple_times();
    test_invalid_operations();

    print_separator("All Tests Completed");
    printf("If you see this message, basic prctl interface is working!\n");
    printf("===============================================\n");

    return 0;
}