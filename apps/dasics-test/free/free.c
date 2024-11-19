#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "udasics.h"

const char *test_info = "[MAIN]-  Test 4: test dasics bound register free \n";

static char ATTR_ULIB_DATA pub_rwbuffer[100] = "[ULIB1]: It's public rw buffer!";

void exit_function() {
    printf("[MAIN]test dasics finished\n");
}

int main() {
    int32_t idx0;

    atexit(exit_function);

    printf(test_info);

    register_udasics(0);

    idx0 = dasics_libcfg_alloc(DASICS_LIBCFG_R | DASICS_LIBCFG_W, (uint64_t)(pub_rwbuffer + 100), (uint64_t)pub_rwbuffer);

    dasics_print_cfg_register(idx0);

    // Free those used libcfg via handlers
    dasics_libcfg_free(idx0);

    uint32_t result = dasics_libcfg_get(idx0) & DASICS_LIBCFG_V;

    if(result)
        printf("\x1b[31m%s\x1b[0m","free function error!\n");

    dasics_print_cfg_register(idx0);

    unregister_udasics();

    return 0;
}