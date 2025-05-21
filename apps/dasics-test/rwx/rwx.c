#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "udasics.h"
#include "fit.h"

const char *test_info = "[MAIN]-  Test 1: bound register allocation and authority \n";

static char secret[100] = "[ULIB1]: It's the secret!";
static char __attribute__((section(".ulibrodata.test_rwx"))) pub_readonly[100] = "[ULIB1]: It's readonly buffer!";
static char __attribute__((section(".ulibdata.test_rwx"))) pub_rwbuffer[100] = "[ULIB1]: It's public rw buffer!";
static char __attribute__((section(".ulibbss.test_rwx"))) pub_rwbss[10];

void __attribute__((section(".ulibtext.test_rwx"))) test_rwx() {
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

    dasics_umaincall(Umaincall_PRINT, "try to modify the bss buffer: %s\n", pub_rwbss);  // That's ok
    for (int i = 0; i < 10; i++) {
        pub_rwbss[i] = 'A';               // That's ok
    }
    pub_rwbss[10] = '\0';               // That's ok
    dasics_umaincall(Umaincall_PRINT, "new bss buffer: %s\n", pub_rwbss);  // That's ok
    pub_rwbss[7] = pub_readonly[12];  // That's ok
    pub_rwbss[4] = 'B';               // That's ok
    pub_rwbss[100] = 'B';             // raise DasicsUStoreAccessFault
    dasics_umaincall(Umaincall_PRINT, "new bss buffer: %s\n", pub_rwbss);  // That's ok

    dasics_umaincall(Umaincall_PRINT, "************* ULIB   END ***************** \n");  // lib call main
}


void exit_function() {
    printf("[MAIN] test dasics finished\n");
}

int main() {
    atexit(exit_function);

    printf(test_info);

    register_udasics(0);

    fit_init();
    fit_print();

    fit_switchto(test_rwx);

    fit_destroy();

    unregister_udasics();

    return 0;
}