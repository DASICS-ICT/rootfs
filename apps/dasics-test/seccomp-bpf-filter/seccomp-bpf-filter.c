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

#define __NR_openat    56
#define __NR_close     57
#define __NR_read      63
#define __NR_write     64
#define __NR_exit_group 94
#define __NR_brk 214

#define read_csr(reg) ({ unsigned long __tmp; \
    asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
    __tmp; })
  
  #define write_csr(reg, val) ({ \
    asm volatile ("csrw " #reg ", %0" :: "rK"(val)); })
  
  #define rdcycle() read_csr(cycle)
  #define rdtime() read_csr(time)


struct sock_filter filter[] = {
    // 检查架构
    BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, arch)),
    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_RISCV64, 1, 0),
    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL_PROCESS),
    // 加载系统调用号
    BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, nr)),
    // 允许 openat
    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_openat, 0, 1),
    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
    // 允许 close
    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_close, 0, 1),
    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
    // 允许 read
    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_read, 0, 1),
    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
    // 允许 write
    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_write, 0, 1),
    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
    // 允许 brk
    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_brk, 1, 0),
    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
    // 其它一律拒绝
    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL_PROCESS),
};

struct sock_fprog prog = {
    .len = (unsigned short)(sizeof(filter)/sizeof(filter[0])),
    .filter = filter,
};

int main() {
    prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);
    prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog);

    uint64_t open_cycles, close_cycles, read_cycles, write_cycles = 0;
    uint64_t start, end;
    
    for(int i =0; i < TEST_LOOP; i++) {
        start = rdcycle();
        int fd = open("test-2.txt", O_RDWR | O_CREAT, 0644);
        end = rdcycle();
        open_cycles += (end - start);
        // if (fd < 0) {
        //     perror("open");
        //     return 1;
        // }
        const char *msg = "Hello, world!";
        start = rdcycle();
        ssize_t bytes_written = write(fd, msg, 13);
        end = rdcycle();
        write_cycles += (end - start);
        // if (bytes_written < 0) {
        //     perror("write");
        //     close(fd);
        //     return 1;
        // }
        char buffer[128];
        start = rdcycle();
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        end = rdcycle();
        read_cycles += (end - start);
        // if (bytes_read < 0) {
        //     perror("read");
        //     close(fd);
        //     return 1;
        // }
        buffer[bytes_read] = '\0';
        // printf("Read: %s\n", buffer);

        start = rdcycle();
        int res = close(fd);
        end = rdcycle();
        close_cycles += (end - start);
        // if (res < 0) {
        //     perror("close");
        //     return 1;
        // }
    }

    //把四个cycles结果除以TEST_LOOP，并通过write系统调用输出到标准输出
    open_cycles /= TEST_LOOP;
    close_cycles /= TEST_LOOP;
    read_cycles /= TEST_LOOP;
    write_cycles /= TEST_LOOP;
    printf("open_cycles: %lu\nclose_cycles: %lu\nread_cycles: %lu\nwrite_cycles: %lu\n", open_cycles, close_cycles, read_cycles, write_cycles);
    while(1);


    return 0;
}