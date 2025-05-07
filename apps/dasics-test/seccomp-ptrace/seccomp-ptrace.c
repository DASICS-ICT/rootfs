#define SYS_open   56
#define SYS_close  57
#define SYS_read   63
#define SYS_write  64

#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <errno.h>

// RISC-V 寄存器定义（参考 <sys/user.h>）
struct user_regs_struct {
  unsigned long pc;
  unsigned long ra;
  unsigned long sp;
  unsigned long gp;
  unsigned long tp;
  unsigned long t0;
  unsigned long t1;
  unsigned long t2;
  unsigned long s0;
  unsigned long s1;
  unsigned long a0;
  unsigned long a1;
  unsigned long a2;
  unsigned long a3;
  unsigned long a4;
  unsigned long a5;
  unsigned long a6;
  unsigned long a7; // 系统调用号存放在 a7
  // ... 其他寄存器省略
};

// 允许的系统调用白名单
static const int ALLOWED_SYSCALLS[] = {
    SYS_open, SYS_close, SYS_read, SYS_write
};

// 检查系统调用是否在白名单中
static int is_syscall_allowed(long syscall_no) {
    for (size_t i = 0; i < sizeof(ALLOWED_SYSCALLS)/sizeof(int); i++) {
        if (syscall_no == ALLOWED_SYSCALLS[i]) {
            return 1;
        }
    }
    return 0;
}

void monitor_child(pid_t pid) {
    int status;
    struct user_regs_struct regs;

    while (1) {
        // 等待子进程触发系统调用
        ptrace(PTRACE_SYSCALL, pid, 0, 0);
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) break;

        // 获取寄存器状态
        ptrace(PTRACE_GETREGS, pid, 0, &regs);
        long syscall_no = regs.a7;

        // 拦截非白名单系统调用
        if (!is_syscall_allowed(syscall_no)) {
            // 强制返回错误 ENOSYS（Function not implemented）
            regs.a0 = -ENOSYS;  // RISC-V 返回值存储在 a0
            ptrace(PTRACE_SETREGS, pid, 0, &regs);
            
            // 跳过系统调用执行（直接进入退出阶段）
            ptrace(PTRACE_SYSCALL, pid, 0, 0);
            continue;
        }

        // 放行允许的系统调用
        ptrace(PTRACE_SYSCALL, pid, 0, 0);
    }
}

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        // 子进程：请求被跟踪
        ptrace(PTRACE_TRACEME, 0, 0, 0);
        execl("./origin-syscall", "origin-syscall", NULL);  // 替换为你的目标程序
    } else {
        // 父进程：监控子进程
        monitor_child(pid);
    }
    return 0;
}
