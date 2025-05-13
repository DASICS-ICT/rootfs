#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>  // 新增头文件用于strcasecmp

#define PAGE_SIZE sysconf(_SC_PAGESIZE)

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s <r|read|w|write> <filename> <physical_address> <length>\n", prog_name);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        print_usage(argv[0]);
    }

    // 解析操作模式（新增模式标准化逻辑）
    char *mode_str = NULL;
    char *mode_arg = argv[1];
    if (strcasecmp(mode_arg, "r") == 0 || strcasecmp(mode_arg, "read") == 0) {
        mode_str = "read";
    } else if (strcasecmp(mode_arg, "w") == 0 || strcasecmp(mode_arg, "write") == 0) {
        mode_str = "write";
    } else {
        fprintf(stderr, "Invalid mode: must be 'read'/'r' or 'write'/'w'\n");
        print_usage(argv[0]);
    }

    char *filename = argv[2];
    unsigned long phys_addr = strtoul(argv[3], NULL, 0);
    size_t length = strtoul(argv[4], NULL, 0);

    // 打开/dev/mem设备（使用标准化后的模式判断）
    int mem_fd = open("/dev/mem", (strcmp(mode_str, "write") == 0) ? O_RDWR : O_RDONLY);
    if (mem_fd == -1) {
        perror("Failed to open /dev/mem");
        exit(EXIT_FAILURE);
    }

    // 计算页对齐参数
    size_t page_size = PAGE_SIZE;
    off_t map_base = phys_addr & ~(page_size - 1);
    off_t map_offset = phys_addr - map_base;
    size_t map_size = length + map_offset;

    // 映射物理内存（使用标准化后的模式判断）
    void *mapped = mmap(NULL, map_size,
                       (strcmp(mode_str, "write") == 0) ? (PROT_READ | PROT_WRITE) : PROT_READ,
                       MAP_SHARED, mem_fd, map_base);
    if (mapped == MAP_FAILED) {
        perror("mmap failed");
        close(mem_fd);
        exit(EXIT_FAILURE);
    }

    // 计算实际访问指针
    void *virt_addr = mapped + map_offset;

    if (strcmp(mode_str, "write") == 0) {
        // 写入模式：从文件读取数据写入物理内存
        FILE *file = fopen(filename, "rb");
        if (!file) {
            perror("Failed to open file for writing");
            munmap(mapped, map_size);
            close(mem_fd);
            exit(EXIT_FAILURE);
        }

        size_t bytes_read = fread(virt_addr, 1, length, file);
        if (bytes_read != length) {
            fprintf(stderr, "Warning: File truncated (read %zu/%zu bytes)\n", bytes_read, length);
        }

        fclose(file);
        printf("Written %zu bytes to 0x%lx\n", bytes_read, phys_addr);

    } else { // 已通过前面检查，这里一定是read模式
        // 读取模式：从物理内存读取数据写入文件
        FILE *file = fopen(filename, "wb");
        if (!file) {
            perror("Failed to open file for reading");
            munmap(mapped, map_size);
            close(mem_fd);
            exit(EXIT_FAILURE);
        }

        size_t bytes_written = fwrite(virt_addr, 1, length, file);
        if (bytes_written != length) {
            fprintf(stderr, "Error: Failed to write all data (wrote %zu/%zu bytes)\n",
                    bytes_written, length);
        }

        fclose(file);
        printf("Read %zu bytes from 0x%lx\n", bytes_written, phys_addr);
    }

    // 清理资源
    munmap(mapped, map_size);
    close(mem_fd);

    return EXIT_SUCCESS;
}