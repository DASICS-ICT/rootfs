#include <linux/filter.h>
#include <linux/seccomp.h>
#include <linux/audit.h>  // 确保包含此头文件
#include <sys/prctl.h>
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <string.h>

#define TEST_LOOP 100

#define read_csr(reg) ({ unsigned long __tmp; \
    asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
    __tmp; })
  
  #define write_csr(reg, val) ({ \
    asm volatile ("csrw " #reg ", %0" :: "rK"(val)); })
  
  #define rdcycle() read_csr(cycle)
  #define rdtime() read_csr(time)


int main() {
    uint64_t open_cycles, close_cycles, read_cycles, write_cycles = 0;
    uint64_t start, end;
    
    for(int i =0; i < TEST_LOOP; i++) {
        start = rdcycle();
        int fd = open("test-1.txt", O_RDWR | O_CREAT, 0644);
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
        // printf("Read: %s\n", buffer);

        start = rdcycle();
        int res = close(fd);
        end = rdcycle();
        close_cycles += (end - start);
        if (res < 0) {
            perror("close");
            return 1;
        }
    }

    printf("open cycles: %lu, close cycles: %lu, read cycles: %lu, write cycles: %lu\n",
        open_cycles/TEST_LOOP,
        close_cycles/TEST_LOOP,
        read_cycles/TEST_LOOP,
        write_cycles/TEST_LOOP);

    return 0;
}