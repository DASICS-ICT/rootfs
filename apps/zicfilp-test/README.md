# Zicfilp User-Space Test Programs

RISC-V Zicfilp (Landing Pad) 扩展的用户态测试程序。

## 测试程序

### test_zicfilp_correct.c
演示正确的 Zicfilp 测试方法：
- 启用前：可以调用 printf
- 启用后：只能使用 syscall
- 禁用后：恢复正常

## 关键理解

**Zicfilp 启用后不能调用库函数的原因：**
- 函数调用使用 `jalr x1` 跳转到目标
- 目标函数需要有 LPAD 指令
- libc (printf等) 没有用 Zicfilp 编译器编译
- 缺少 LPAD → Landing Pad Fault

**解决方案：**
- 启用后只用 syscall（ecall 不是函数调用）
- 测试完立即禁用

## 编译

```bash
cd /path/to/rootfs
./build-zicfilp.sh
```

## 运行

```bash
# 在 NEMU 启动 Linux 后
/root/test_zicfilp_correct
```
