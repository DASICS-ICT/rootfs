#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/prctl.h>
#include <stddef.h>
#include <stdint.h>
#include <fcntl.h> 
#include <sys/stat.h>

#include "udasics.h"

#define TEST_LOOP 100
#define read_csr(reg) ({ unsigned long __tmp; \
    asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
    __tmp; })
  
#define write_csr(reg, val) ({ \
asm volatile ("csrw " #reg ", %0" :: "rK"(val)); })

#define rdcycle() read_csr(cycle)
#define rdtime() read_csr(time)

int ATTR_ULIB_TEXT test_syscall() {
    uint64_t open_cycles, close_cycles, read_cycles, write_cycles = 0;
    uint64_t start, end;

    for(int i =0; i < TEST_LOOP; i++) {
        start = rdcycle();
        int fd = open("test-3.txt", O_RDWR | O_CREAT, 0644);
        end = rdcycle();
        open_cycles += (end - start);
        if (fd < 0) {
            perror("open");
            return 1;
        }
        const char *msg = "Hello, world!";
        start = rdcycle();
        ssize_t bytes_written = write(fd, msg, 13);
        end = rdcycle();
        write_cycles += (end - start);
        if (bytes_written < 0) {
            perror("write");
            close(fd);
            return 1;
        }
        char buffer[128];
        start = rdcycle();
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        end = rdcycle();
        read_cycles += (end - start);
        if (bytes_read < 0) {
            perror("read");
            close(fd);
            return 1;
        }
        buffer[bytes_read] = '\0';

        start = rdcycle();
        int res = close(fd);
        end = rdcycle();
        close_cycles += (end - start);
        if (res < 0) {
            perror("close");
            return 1;
        }
    }
    open_cycles /= TEST_LOOP;
    close_cycles /= TEST_LOOP;
    read_cycles /= TEST_LOOP;
    write_cycles /= TEST_LOOP;
    printf("open_cycles: %lu\nclose_cycles: %lu\nread_cycles: %lu\nwrite_cycles: %lu\n", open_cycles, close_cycles, read_cycles, write_cycles);

    return 0;
}


void exit_function() {
    printf("[MAIN] test dasics finished\n");
}

int main() {
    atexit(exit_function);

    register_udasics(0);

    // Allocate jump bound for .ulibtext section
    extern char __ULIBTEXT_BEGIN__, __ULIBTEXT_END__;
    int idx_ulibtext = dasics_jumpcfg_alloc(0, 0x4000000000);
    int idx_ulibdata = dasics_libcfg_alloc(DASICS_LIBCFG_R | DASICS_LIBCFG_W, 0, 0x4000000000);

    init_syscall_check();

    lib_call(&test_syscall);

    // Free those used permissions via handlers
    dasics_libcfg_free(idx_ulibdata);
    dasics_jumpcfg_free(idx_ulibtext);

    unregister_udasics();

    return 0;
}