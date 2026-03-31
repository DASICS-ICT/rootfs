#include <stdio.h>
#include <asm/unistd.h>
#include <signal.h>
#include <errno.h>

#include "udasics.h"
#include "usyscall.h"
#include "fit.h"

const char *test_info = "[MAIN]-  Test 2: syscall and maincall test \n";

void __attribute__((section(".ulibtext.test_syscall"))) test_syscall() {
    dasics_umaincall(Umaincall_PRINT, "************* ULIB START ***************** \n");  // lib call main

    uint64_t pid = UINT64_MAX;
    pid = ULIB_SYSCALL0(__NR_getpid);  // That's ok (sysno = 172)
    dasics_umaincall(Umaincall_PRINT, "pid = %ld\n", pid);  // That's ok

    ULIB_SYSCALL0(__NR_exit);  // raise DasicsUSyscallAccessFault (sysno = 93)
    dasics_umaincall(Umaincall_UNKNOWN, 0xdeadbeef);  // cancelled by fit_check_maincall

    dasics_umaincall(Umaincall_PRINT, "************* ULIB   END ***************** \n");  // lib call main
}

void exit_function() {
    printf("[MAIN] test dasics finished\n");
}

int main() {
    atexit(exit_function);

    printf("%s", test_info);

    fit_print();

    fit_switchto(test_syscall);

    return 0;
}