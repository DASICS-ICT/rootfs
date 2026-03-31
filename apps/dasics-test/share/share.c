#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "udasics.h"
#include "fit.h"

#define ACCESS_DATA(readonly, rwbuffer, rwbss, c) \
    { \
        for (int i = 0; i < 100; i++) { \
            volatile char __attribute__((unused)) temp = (readonly)[i]; \
        } \
        for (int i = 0; i < 100; i++) { \
            volatile char temp = (rwbuffer)[i]; \
            (rwbuffer)[i] = temp; \
        } \
        for (int i = 0; i < 10; i++) { \
            (rwbss)[i] = c; \
            volatile char __attribute__((unused)) temp = (rwbss)[i]; \
        } \
        (rwbss)[9] = '\0'; \
    }

const char *test_info = "[MAIN]-  Test 4: shared function closure \n";

static char __attribute__((section(".ulibrodata.test_rwx1"))) ulib1_readonly[100]  = "[ULIB1]: It's readonly buffer!";
static char __attribute__((section(".ulibdata.test_rwx1")))   ulib1_rwbuffer[100]  = "[ULIB1]: It's public rw buffer!";
static char __attribute__((section(".ulibbss.test_rwx1")))    ulib1_rwbss[10];
static char __attribute__((section(".ulibrodata.test_rwx2"))) ulib2_readonly[100]  = "[ULIB2]: It's readonly buffer!";
static char __attribute__((section(".ulibdata.test_rwx2")))   ulib2_rwbuffer[100]  = "[ULIB2]: It's public rw buffer!";
static char __attribute__((section(".ulibbss.test_rwx2")))    ulib2_rwbss[10];
static char __attribute__((section(".ulibrodata.share")))     ushare_readonly[100] = "[USHARE]: It's readonly buffer!";
static char __attribute__((section(".ulibdata.share")))       ushare_rwbuffer[100] = "[USHARE]: It's public rw buffer!";
static char __attribute__((section(".ulibbss.share")))        ushare_rwbss[10];

void __attribute__((section(".ulibtext.share"))) share_func(int parent) {
    // Share starts
    dasics_umaincall(Umaincall_PRINT, "************* USHARE START **************** \n");  // lib call main

    // Access the private data of share legally
    ACCESS_DATA(ushare_readonly, ushare_rwbuffer, ushare_rwbss, 'C');
    dasics_umaincall(Umaincall_PRINT, "[USHARE %d] ushare rwbss: %s\n", parent, ushare_rwbss);  // That's ok

    // Try to access the private data of test_rwx1 and test_rwx2
    char temp;
    dasics_umaincall(Umaincall_PRINT, "[USHARE %d] try to access ulib1_readonly\n", parent);
    temp = ulib1_readonly[0];
    dasics_umaincall(Umaincall_PRINT, "[USHARE %d] try to access ulib1_rwbuffer\n", parent);
    temp = ulib1_rwbuffer[99];
    ulib1_rwbuffer[0] = 'C';
    ulib1_rwbuffer[9] = 'C';
    ulib1_rwbuffer[99] = 'C';
    dasics_umaincall(Umaincall_PRINT, "[USHARE %d] try to access ulib1_rwbss\n", parent);
    ulib1_rwbss[0] = 'C';
    ulib1_rwbss[8] = 'C';
    dasics_umaincall(Umaincall_PRINT, "[USHARE %d] ulib1 rwbss: %s\n", parent, ulib1_rwbss);  // That's ok

    dasics_umaincall(Umaincall_PRINT, "[USHARE %d] try to access ulib2_readonly\n", parent);
    temp = ulib2_readonly[0];
    dasics_umaincall(Umaincall_PRINT, "[USHARE %d] try to access ulib2_rwbuffer\n", parent);
    temp = ulib2_rwbuffer[99];
    ulib2_rwbuffer[0] = 'C';
    ulib2_rwbuffer[9] = 'C';
    ulib2_rwbuffer[99] = 'C';
    dasics_umaincall(Umaincall_PRINT, "[USHARE %d] try to access ulib2_rwbss\n", parent);
    ulib2_rwbss[0] = 'C';
    ulib2_rwbss[8] = 'C';
    dasics_umaincall(Umaincall_PRINT, "[USHARE %d] ulib2 rwbss: %s\n", parent, ulib2_rwbss);  // That's ok

    // Share ends
    dasics_umaincall(Umaincall_PRINT, "************* USHARE   END **************** \n");  // lib call main
}

void __attribute__((section(".ulibtext.test_rwx1"))) test_rwx1() {
    // Test_rwx1 starts
    dasics_umaincall(Umaincall_PRINT, "************* ULIB1 START ***************** \n");  // lib call main

    // Access the private data of test_rwx1 legally
    ACCESS_DATA(ulib1_readonly, ulib1_rwbuffer, ulib1_rwbss, 'A');
    dasics_umaincall(Umaincall_PRINT, "[ULIB1] ulib1 rwbss: %s\n", ulib1_rwbss);  // That's ok

    // Access the private data of share legally
    ACCESS_DATA(ushare_readonly, ushare_rwbuffer, ushare_rwbss, 'A');
    dasics_umaincall(Umaincall_PRINT, "[ULIB1] ushare rwbss: %s\n", ushare_rwbss);  // That's ok

    // Test illegal access to test_rwx2
    char temp;
    dasics_umaincall(Umaincall_PRINT, "[ULIB1] try to access ulib2_readonly\n");
    temp = ulib2_readonly[0];  // raise DasicsULoadAccessFault
    dasics_umaincall(Umaincall_PRINT, "[ULIB1] try to access ulib2_rwbuffer\n");
    temp = ulib2_rwbuffer[99];  // raise DasicsULoadAccessFault
    ulib2_rwbuffer[0] = 'A';  // raise DasicsUStoreAccessFault
    ulib2_rwbuffer[9] = 'A';  // raise DasicsUStoreAccessFault
    ulib2_rwbuffer[99] = 'A';  // raise DasicsUStoreAccessFault
    dasics_umaincall(Umaincall_PRINT, "[ULIB1] try to access ulib2_rwbss\n");
    temp = ulib2_rwbss[0];  // raise DasicsULoadAccessFault
    ulib2_rwbss[0] = 'A';  // raise DasicsUStoreAccessFault
    ulib2_rwbss[8] = 'A';  // raise DasicsUStoreAccessFault

    // Jump to the share function
    share_func(1);

    // Test_rwx1 ends
    dasics_umaincall(Umaincall_PRINT, "************* ULIB1   END ***************** \n");  // lib call main
}

void __attribute__((section(".ulibtext.test_rwx2"))) test_rwx2() {
    // Test_rwx2 starts
    dasics_umaincall(Umaincall_PRINT, "************* ULIB2 START ***************** \n");  // lib call main

    // Access the private data of test_rwx2 legally
    ACCESS_DATA(ulib2_readonly, ulib2_rwbuffer, ulib2_rwbss, 'B');
    dasics_umaincall(Umaincall_PRINT, "[ULIB2] ulib2 rwbss: %s\n", ulib2_rwbss);  // That's ok

    // Access the private data of share legally
    ACCESS_DATA(ushare_readonly, ushare_rwbuffer, ushare_rwbss, 'B');
    dasics_umaincall(Umaincall_PRINT, "[ULIB2] ushare rwbss: %s\n", ushare_rwbss);  // That's ok

    // Test illegal access to test_rwx1
    char temp;
    dasics_umaincall(Umaincall_PRINT, "[ULIB2] try to access ulib1_readonly\n");
    temp = ulib1_readonly[0];  // raise DasicsULoadAccessFault
    dasics_umaincall(Umaincall_PRINT, "[ULIB2] try to access ulib1_rwbuffer\n");
    temp = ulib1_rwbuffer[99];  // raise DasicsULoadAccessFault
    ulib1_rwbuffer[0] = 'B';  // raise DasicsUStoreAccessFault
    ulib1_rwbuffer[9] = 'B';  // raise DasicsUStoreAccessFault
    ulib1_rwbuffer[99] = 'B';  // raise DasicsUStoreAccessFault
    dasics_umaincall(Umaincall_PRINT, "[ULIB2] try to access ulib1_rwbss\n");
    temp = ulib1_rwbss[0];  // raise DasicsULoadAccessFault
    ulib1_rwbss[0] = 'B';  // raise DasicsUStoreAccessFault
    ulib1_rwbss[8] = 'B';  // raise DasicsUStoreAccessFault

    // Jump to the share function
    share_func(2);

    // Test_rwx2 ends
    dasics_umaincall(Umaincall_PRINT, "************* ULIB2   END ***************** \n");  // lib call main
}

void exit_function() {
    printf("[MAIN] test dasics finished\n");
}

int main() {
    atexit(exit_function);

    printf("%s", test_info);

    fit_print();

    fit_switchto(test_rwx1);
    fit_switchto(test_rwx2);

    return 0;
}