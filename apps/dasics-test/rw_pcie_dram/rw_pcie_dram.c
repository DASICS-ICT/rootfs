#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
    int xdma_fd;
    uint64_t offset_in_fpga_dram;
    size_t trans_size;
    char *file_name;
    FILE *file;
    void *buffer;
    char *end_ptr;

    if (argc != 5) {
        fprintf(stderr, "Usage: %s r/w <offset> <size> <filename>\n"
                        "  r 从FPGA读取数据到文件\n"
                        "  w 将文件数据写入FPGA\n"
                        "  offset: FPGA DRAM地址（十六进制或十进制）\n"
                        "  size:  传输数据大小（字节）\n"
                        "  filename: 输入/输出文件名\n", argv[0]);
        return -1;
    }

    // 验证模式参数
    if (strlen(argv[1]) != 1) {
        fprintf(stderr, "错误：模式参数必须为单个字符\n");
        return -1;
    }
    const char mode = argv[1][0];

    // 解析地址参数
    offset_in_fpga_dram = strtoull(argv[2], &end_ptr, 0);
    if (*end_ptr != '\0') {
        fprintf(stderr, "错误：无效的地址参数 '%s'\n", argv[2]);
        return -1;
    }

    // 解析传输大小
    trans_size = strtoul(argv[3], &end_ptr, 0);
    if (*end_ptr != '\0' || trans_size == 0) {
        fprintf(stderr, "错误：无效的大小参数 '%s'\n", argv[3]);
        return -1;
    }

    file_name = argv[4];

    // 打开XDMA设备
    if (mode == 'w') {
        if ((xdma_fd = open("/dev/xdma0_h2c_0", O_WRONLY)) < 0) {
            perror("无法打开h2c设备");
            return -1;
        }
        if ((file = fopen(file_name, "rb")) == NULL) {
            perror("无法打开输入文件");
            close(xdma_fd);
            return -1;
        }
    } else if (mode == 'r') {
        if ((xdma_fd = open("/dev/xdma0_c2h_0", O_RDONLY)) < 0) {
            perror("无法打开c2h设备");
            return -1;
        }
        if ((file = fopen(file_name, "wb")) == NULL) {
            perror("无法创建输出文件");
            close(xdma_fd);
            return -1;
        }
    } else {
        fprintf(stderr, "错误：无效模式 '%c' (必须为 r 或 w)\n", mode);
        return -1;
    }

    // 分配内存缓冲区
    if ((buffer = malloc(trans_size)) == NULL) {
        perror("内存分配失败");
        fclose(file);
        close(xdma_fd);
        return -1;
    }

    ssize_t result = -1;
    if (mode == 'w') {
        // 文件 → 缓冲区
        const size_t read_bytes = fread(buffer, 1, trans_size, file);
        if (read_bytes != trans_size) {
            fprintf(stderr, "警告: 文件读取不完整 (%zu/%zu 字节)\n", 
                    read_bytes, trans_size);
        }

        // 缓冲区 → FPGA
        result = pwrite(xdma_fd, buffer, trans_size, (off_t)offset_in_fpga_dram);
        if (result < 0) {
            perror("FPGA写入失败");
        } else {
            printf("成功写入 FPGA [0x%lX] %zd 字节\n", 
                  (unsigned long)offset_in_fpga_dram, result);
        }
    } else {
        // FPGA → 缓冲区
        result = pread(xdma_fd, buffer, trans_size, (off_t)offset_in_fpga_dram);
        if (result < 0) {
            perror("FPGA读取失败");
        } else {
            printf("成功读取 FPGA [0x%lX] %zd 字节\n", 
                  (unsigned long)offset_in_fpga_dram, result);
        }

        // 缓冲区 → 文件
        if (result > 0) {
            const size_t write_bytes = fwrite(buffer, 1, result, file);
            if (write_bytes != (size_t)result) {
                fprintf(stderr, "警告: 文件写入不完整 (%zu/%zd 字节)\n", 
                        write_bytes, result);
            }
        }
    }

    // 释放资源
    free(buffer);
    fclose(file);
    close(xdma_fd);

    return (result < 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}