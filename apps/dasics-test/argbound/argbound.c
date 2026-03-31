#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "udasics.h"
#include "fit.h"

const char *test_info = "[MAIN]-  Test 3: parameter bounds \n";

typedef struct {
    long sensitive_long;
    char string[10];
    int sensitive_int;
} __attribute__((aligned(8))) test_argbound_args_t;

int __attribute__((section(".ulibtext.test_argbound"))) test_argbound(char *src, char *dest) {
    dasics_umaincall(Umaincall_PRINT, "************* ULIB START ***************** \n");  // lib call main

    dasics_umaincall(Umaincall_PRINT, "Performing legal access...\n");  // lib call main

    // Get the length of the source string via while-loop
    int len = 0;
    while (src[len] != '\0') {
        len++;
    }

    // Copy the string from src to dest
    for (int i = 0; i < len; i++) {
        dest[i] = src[i];
    }
    dest[len] = '\0';  // Null-terminate the destination string

    // Perform malicious access to src and dest
    dasics_umaincall(Umaincall_PRINT, "Performing malicious access...\n");  // lib call main
    char __attribute__((unused)) temp;
    src[0] = 'A';  // raise DasicsUStoreAccessFault
    temp = src[-4];  // raise DasicsULoadAccessFault
    dest[10] = 'B';  // raise DasicsUStoreAccessFault
    temp = dest[10];  // raise DasicsULoadAccessFault

    dasics_umaincall(Umaincall_PRINT, "************* ULIB   END ***************** \n");  // lib call main

    return 123;
}

int __attribute__((section(".ulibtext.test_argbound"))) test_argbound_wrapper(va_list args) {
    // Get the arguments
    char *src = va_arg(args, char *);
    char *dest = va_arg(args, char *);

    // Call the actual test function
    return test_argbound(src, dest);
}

void exit_function() {
    printf("[MAIN] test dasics finished\n");
}

int main() {
    atexit(exit_function);

    printf("%s", test_info);

    fit_init(0);
    fit_print();

    test_argbound_args_t src = {
        .sensitive_long = 0x1234567890abcdef,
        .string = "Hello!",
        .sensitive_int = 42
    };

    test_argbound_args_t dst = {
        .sensitive_long = 0xdeadbeef,
        .string = {'\0'},
        .sensitive_int = 123123
    };

    /* Grant parameter bounds to test_argbound_wrapper (src.string R, dst.string W), times=1 */
    fit_bounds_t arg_perms[2];
    arg_perms[0].perm = DASICS_LIBCFG_R;
    arg_perms[0].lo = (uint64_t)src.string;
    arg_perms[0].hi = (uint64_t)src.string + 10;
    arg_perms[0].handle = -1;
    arg_perms[1].perm = DASICS_LIBCFG_W;
    arg_perms[1].lo = (uint64_t)dst.string;
    arg_perms[1].hi = (uint64_t)dst.string + 10;
    arg_perms[1].handle = -1;
    size_t valist_size = sizeof(char *) * 2;
    if (fit_permission_grant(test_argbound_wrapper, arg_perms, 2, valist_size, 1) != 0) {
        printf("[MAIN] fit_permission_grant failed\n");
        fit_destroy();
        return -1;
    }

    uint64_t ret = 0;
    ret = fit_switchto(test_argbound_wrapper, src.string, dst.string);
    printf("[MAIN] return value: %lx\n", ret);

    fit_destroy();

    return 0;
}