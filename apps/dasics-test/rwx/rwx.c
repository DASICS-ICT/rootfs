#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "udasics.h"

const char *test_info = "[MAIN]-  Test 3: bound register allocation and authority \n";

static char ATTR_ULIB_DATA secret[100] 		 = "[ULIB1]: It's the secret!";
static char ATTR_ULIB_DATA pub_readonly[100] = "[ULIB1]: It's readonly buffer!";
static char ATTR_ULIB_DATA pub_rwbuffer[100] = "[ULIB1]: It's public rw buffer!";

int ATTR_ULIB_TEXT test_rwx() {
    dasics_umaincall(Umaincall_PRINT, "************* ULIB START ***************** \n");  // lib call main

    dasics_umaincall(Umaincall_PRINT, "try to print the read only buffer: %s\n", pub_readonly);  // That's ok
    dasics_umaincall(Umaincall_PRINT, "try to print the rw buffer: %s\n", pub_rwbuffer);         // That's ok

    dasics_umaincall(Umaincall_PRINT, "try to modify the rw buffer: %s\n", pub_rwbuffer);        // That's ok
    pub_rwbuffer[19] = pub_readonly[12];  // That's ok
    pub_rwbuffer[21] = 'B';               // That's ok
    dasics_umaincall(Umaincall_PRINT, "new rw buffer: %s\n", pub_rwbuffer);  // That's ok

    dasics_umaincall(Umaincall_PRINT, "try to modify read only buffer\n");
    pub_readonly[15] = 'B';               // raise DasicsUStoreAccessFault

    dasics_umaincall(Umaincall_PRINT, "try to load from the secret\n");
    char temp = secret[3];                // raise DasicsULoadAccessFault
    dasics_umaincall(Umaincall_PRINT, "try to store to the secret\n");
    secret[3] = temp;                     // raise DasicsUStoreAccessFault

    dasics_umaincall(Umaincall_PRINT, "************* ULIB   END ***************** \n");  // lib call main

    return 0;
}


void exit_function() {
    printf("[MAIN] test dasics finished\n");
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

    // Allocate permissions for public buffers
    int idx_ro = dasics_libcfg_alloc(DASICS_LIBCFG_R                  , (uint64_t)pub_readonly, (uint64_t)(pub_readonly + 100));
    int idx_rw = dasics_libcfg_alloc(DASICS_LIBCFG_R | DASICS_LIBCFG_W, (uint64_t)pub_rwbuffer, (uint64_t)(pub_rwbuffer + 100));

    lib_call(&test_rwx);

    // Free those used permissions via handlers
    dasics_libcfg_free(idx_rw);
    dasics_libcfg_free(idx_ro);
    dasics_libcfg_free(idx_stack);
    dasics_jumpcfg_free(idx_ulibtext);

    unregister_udasics();

    return 0;
}