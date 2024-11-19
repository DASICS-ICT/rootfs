#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "udasics.h"

const char *test_info = "[MAIN]-  Test 1: out of bound test \n";

static char ATTR_ULIB_DATA unboundedData[100] = "[ULIB]: It's the unbounded data!";

int ATTR_ULIB_TEXT test_ofb() {
    // Test user main boundarys.
    // Note: gcc -O2 option and RVC will cause
    // some unexpected compilation results.

    dasics_umaincall(Umaincall_PRINT, "************* ULIB START ***************** \n"); // lib call main
    dasics_umaincall(Umaincall_PRINT, "try to load from the unbounded address: 0x%lx\n", unboundedData); // lib call main
    char data = unboundedData[0]; //should arise uload fault and skip the load instruction
    dasics_umaincall(Umaincall_PRINT, "try to store to the unbounded address:  0x%lx\n", unboundedData); // lib call main
    unboundedData[1] = data;      //should arise ustore fault and skip the store instruction
    dasics_umaincall(Umaincall_PRINT, "************* ULIB   END ***************** \n"); // lib call main

    return 0;
}

void exit_function() {
    printf("[MAIN]test dasics finished\n");
}

int main() {
    atexit(exit_function);

    printf(test_info);

    register_udasics(0);

    // Allocate jump bound for .ulibtext section
    extern char __ULIBTEXT_BEGIN__, __ULIBTEXT_END__;
    int idx_ulibtext = dasics_jumpcfg_alloc((uint64_t)&__ULIBTEXT_BEGIN__, (uint64_t)&__ULIBTEXT_END__);

    // Allocate permissions for stack
    uint64_t frame_addr, badfunc_stack_top;
    asm volatile("mv %0, sp" : "=r"(frame_addr));
    badfunc_stack_top = frame_addr - 0x8;  // 0x8 is the stack size of lib_call
    int idx_stack = dasics_libcfg_alloc(DASICS_LIBCFG_R | DASICS_LIBCFG_W, badfunc_stack_top - 32, badfunc_stack_top);

    // Call the test function
    lib_call(&test_ofb);

    // Free those used permissions via handlers
    dasics_libcfg_free(idx_stack);
    dasics_jumpcfg_free(idx_ulibtext);

    unregister_udasics();

    return 0;
}