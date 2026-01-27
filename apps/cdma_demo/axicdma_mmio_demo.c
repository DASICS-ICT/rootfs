#define _FILE_OFFSET_BITS 64
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* Minimal AXI CDMA register layout (copied from xaxicdma_hw.h). */
#define XAXICDMA_CR_OFFSET          0x00
#define XAXICDMA_SR_OFFSET          0x04
#define XAXICDMA_SRCADDR_OFFSET     0x18
#define XAXICDMA_SRCADDR_MSB_OFFSET 0x1C
#define XAXICDMA_DSTADDR_OFFSET     0x20
#define XAXICDMA_DSTADDR_MSB_OFFSET 0x24
#define XAXICDMA_BTT_OFFSET         0x28

#define XAXICDMA_CR_RESET_MASK      0x00000004
#define XAXICDMA_CR_SGMODE_MASK     0x00000008

#define XAXICDMA_SR_IDLE_MASK       0x00000002
#define XAXICDMA_SR_ERR_ALL_MASK    0x00000770

#define XAXICDMA_XR_IRQ_ALL_MASK    0x00007000

#define DEFAULT_LEN_BYTES           64u
#define DEFAULT_TIMEOUT_MS          1000
#define RESET_WAIT_US               1000
#define RESET_TRIES                 50
#define IDLE_POLL_US                1000

struct phys_map {
    uint8_t *virt;
    size_t map_len;
    size_t page_offset;
};

static uint32_t cdma_read(volatile uint8_t *regs, off_t offset)
{
    return *(volatile uint32_t *)(regs + offset);
}

static void cdma_write(volatile uint8_t *regs, off_t offset, uint32_t value)
{
    *(volatile uint32_t *)(regs + offset) = value;
}

static int cdma_reset(volatile uint8_t *regs)
{
    cdma_write(regs, XAXICDMA_CR_OFFSET, XAXICDMA_CR_RESET_MASK);

    for (int i = 0; i < RESET_TRIES; ++i) {
        if ((cdma_read(regs, XAXICDMA_CR_OFFSET) & XAXICDMA_CR_RESET_MASK) == 0) {
            return 0;
        }
        usleep(RESET_WAIT_US);
    }

    fprintf(stderr, "CDMA reset timed out\n");
    return -ETIMEDOUT;
}

static void cdma_clear_errors(volatile uint8_t *regs)
{
    /* Write 1s to clear sticky status bits. */
    cdma_write(regs, XAXICDMA_SR_OFFSET, XAXICDMA_SR_ERR_ALL_MASK | XAXICDMA_XR_IRQ_ALL_MASK);
}

static int cdma_wait_idle(volatile uint8_t *regs, int timeout_ms)
{
    int waited_ms = 0;

    while (waited_ms <= timeout_ms) {
        uint32_t sr = cdma_read(regs, XAXICDMA_SR_OFFSET);

        if (sr & XAXICDMA_SR_ERR_ALL_MASK) {
            fprintf(stderr, "CDMA error: status=0x%08x\n", sr);
            return -EIO;
        }

        if (sr & XAXICDMA_SR_IDLE_MASK) {
            return 0;
        }

        usleep(IDLE_POLL_US);
        waited_ms += IDLE_POLL_US / 1000;
    }

    return -ETIMEDOUT;
}

static int cdma_simple_transfer(volatile uint8_t *regs, uint64_t src, uint64_t dst,
                                uint32_t length, int timeout_ms)
{
    if (length == 0) {
        fprintf(stderr, "Length must be non-zero\n");
        return -EINVAL;
    }

    uint32_t cr = cdma_read(regs, XAXICDMA_CR_OFFSET);
    if (cr & XAXICDMA_CR_SGMODE_MASK) {
        fprintf(stderr, "CDMA is in scatter-gather mode; simple mode required\n");
        return -EINVAL;
    }

    cdma_clear_errors(regs);

    int rc = cdma_wait_idle(regs, timeout_ms);
    if (rc) {
        fprintf(stderr, "CDMA not idle before start (%d)\n", rc);
        return rc;
    }

    cdma_write(regs, XAXICDMA_SRCADDR_OFFSET, (uint32_t)(src & 0xFFFFFFFFu));
    cdma_write(regs, XAXICDMA_SRCADDR_MSB_OFFSET, (uint32_t)(src >> 32));
    cdma_write(regs, XAXICDMA_DSTADDR_OFFSET, (uint32_t)(dst & 0xFFFFFFFFu));
    cdma_write(regs, XAXICDMA_DSTADDR_MSB_OFFSET, (uint32_t)(dst >> 32));

    /* Writing BTT kicks off the transfer. */
    cdma_write(regs, XAXICDMA_BTT_OFFSET, length);

    rc = cdma_wait_idle(regs, timeout_ms);
    if (rc) {
        fprintf(stderr, "CDMA timed out waiting for completion (%d)\n", rc);
    }

    return rc;
}

static const char *basename_const(const char *path)
{
    const char *slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}

static int read_sysfs_hex(const char *path, uint64_t *value)
{
    FILE *f = fopen(path, "r");
    if (!f) {
        return -errno;
    }

    unsigned long long temp = 0;
    int n = fscanf(f, "%llx", &temp);
    fclose(f);

    if (n != 1) {
        return -EINVAL;
    }

    *value = (uint64_t)temp;
    return 0;
}

static int map_uio_regs(const char *uio_path, volatile uint8_t **regs_out, size_t *size_out)
{
    char size_path[256];
    const char *uio_name = basename_const(uio_path);
    snprintf(size_path, sizeof(size_path), "/sys/class/uio/%s/maps/map0/size", uio_name);

    uint64_t map_size = 0;
    int rc = read_sysfs_hex(size_path, &map_size);
    if (rc) {
        fprintf(stderr, "Warning: could not read %s (%s), falling back to one page\n",
                size_path, strerror(-rc));
        map_size = (uint64_t)getpagesize();
    }

    int fd = open(uio_path, O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("open uio");
        return -errno;
    }

    void *regs = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    if (regs == MAP_FAILED) {
        perror("mmap uio");
        return -errno;
    }

    *regs_out = (volatile uint8_t *)regs;
    *size_out = (size_t)map_size;
    return 0;
}

static int map_phys_range(int memfd, uint64_t phys, size_t len, struct phys_map *out)
{
    const size_t page_size = (size_t)sysconf(_SC_PAGESIZE);
    const uint64_t page_mask = ~(uint64_t)(page_size - 1u);

    uint64_t page_base = phys & page_mask;
    size_t page_offset = (size_t)(phys - page_base);
    size_t map_len = page_offset + len;

    void *v = mmap(NULL, map_len, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, (off_t)page_base);
    if (v == MAP_FAILED) {
        perror("mmap /dev/mem");
        return -errno;
    }

    out->virt = (uint8_t *)v;
    out->map_len = map_len;
    out->page_offset = page_offset;
    return 0;
}

static void unmap_phys(struct phys_map *m)
{
    if (m->virt) {
        munmap(m->virt, m->map_len);
        m->virt = NULL;
        m->map_len = 0;
        m->page_offset = 0;
    }
}

static void usage(const char *prog)
{
    fprintf(stderr,
            "Usage: %s -s <src_phys> -d <dst_phys> [options]\n"
            "  -u, --uio <path>       UIO device (default /dev/uio0)\n"
            "  -s, --src <phys>       Source physical address for CDMA\n"
            "  -d, --dst <phys>       Destination physical address for CDMA\n"
            "  -l, --len <bytes>      Transfer length in bytes (default %u)\n"
            "  -t, --timeout <ms>     Timeout for idle wait (default %u)\n"
            "  -m, --mem <path>       Device used for mapping buffers (default /dev/mem)\n"
            "  -V, --verify           Map buffers via /dev/mem to fill pattern and verify\n"
            "      --map-bytes <n>    Bytes to map for verify (default = len)\n"
            "\nNotes:\n"
            "  * The CDMA must be configured for simple mode; SG mode is not supported here.\n"
            "  * Buffer addresses must be DMA-capable (uncached or cache-coherent).\n"
            , prog, DEFAULT_LEN_BYTES, DEFAULT_TIMEOUT_MS);
}

int main(int argc, char **argv)
{
    const char *uio_path = "/dev/uio1";
    const char *mem_path = "/dev/mem";
    uint64_t src_phys = 0;
    uint64_t dst_phys = 0;
    uint32_t length = DEFAULT_LEN_BYTES;
    int timeout_ms = DEFAULT_TIMEOUT_MS;
    int verify = 0;
    size_t verify_len = 0;

    static struct option long_opts[] = {
        {"uio", required_argument, NULL, 'u'},
        {"src", required_argument, NULL, 's'},
        {"dst", required_argument, NULL, 'd'},
        {"len", required_argument, NULL, 'l'},
        {"timeout", required_argument, NULL, 't'},
        {"mem", required_argument, NULL, 'm'},
        {"verify", no_argument, NULL, 'V'},
        {"map-bytes", required_argument, NULL, 1000},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "u:s:d:l:t:m:V", long_opts, NULL)) != -1) {
        switch (opt) {
        case 'u':
            uio_path = optarg;
            break;
        case 's':
            src_phys = strtoull(optarg, NULL, 0);
            break;
        case 'd':
            dst_phys = strtoull(optarg, NULL, 0);
            break;
        case 'l':
            length = (uint32_t)strtoul(optarg, NULL, 0);
            break;
        case 't':
            timeout_ms = (int)strtol(optarg, NULL, 0);
            break;
        case 'm':
            mem_path = optarg;
            break;
        case 'V':
            verify = 1;
            break;
        case 1000:
            verify_len = (size_t)strtoull(optarg, NULL, 0);
            break;
        default:
            usage(argv[0]);
            return 1;
        }
    }

    if (src_phys == 0 || dst_phys == 0) {
        usage(argv[0]);
        return 1;
    }

    if (verify_len == 0) {
        verify_len = (size_t)length;
    }

    volatile uint8_t *regs = NULL;
    size_t regs_size = 0;
    int rc = map_uio_regs(uio_path, &regs, &regs_size);
    if (rc) {
        return 1;
    }

    printf("CDMA regs mapped: %p (size 0x%zx)\n", (void *)regs, regs_size);
    printf("Transfer: src 0x%016" PRIx64 " -> dst 0x%016" PRIx64 " len %u bytes\n",
           src_phys, dst_phys, length);

    struct phys_map src_map = {0};
    struct phys_map dst_map = {0};
    int memfd = -1;

    if (verify) {
        memfd = open(mem_path, O_RDWR | O_SYNC);
        if (memfd < 0) {
            perror("open mem");
            munmap((void *)regs, regs_size);
            return 1;
        }

        if ((rc = map_phys_range(memfd, src_phys, verify_len, &src_map)) != 0) {
            munmap((void *)regs, regs_size);
            close(memfd);
            return 1;
        }

        if ((rc = map_phys_range(memfd, dst_phys, verify_len, &dst_map)) != 0) {
            unmap_phys(&src_map);
            munmap((void *)regs, regs_size);
            close(memfd);
            return 1;
        }

        uint8_t *src_ptr = src_map.virt + src_map.page_offset;
        uint8_t *dst_ptr = dst_map.virt + dst_map.page_offset;

        for (size_t i = 0; i < length; ++i) {
            src_ptr[i] = (uint8_t)(i & 0xFF);
            dst_ptr[i] = 0;
        }

        /* Make sure cacheable mappings are flushed if the memory is cacheable. */
        msync(src_map.virt, src_map.map_len, MS_SYNC);
        msync(dst_map.virt, dst_map.map_len, MS_SYNC);
    }

    rc = cdma_reset(regs);
    if (rc) {
        fprintf(stderr, "CDMA reset failed: %d\n", rc);
        goto done;
    }

    rc = cdma_simple_transfer(regs, src_phys, dst_phys, length, timeout_ms);
    if (rc) {
        goto done;
    }

    printf("Transfer completed.\n");

    if (verify) {
        uint8_t *src_ptr = src_map.virt + src_map.page_offset;
        uint8_t *dst_ptr = dst_map.virt + dst_map.page_offset;
        int mismatch = 0;

        msync(dst_map.virt, dst_map.map_len, MS_SYNC);

        for (size_t i = 0; i < length; ++i) {
            if (dst_ptr[i] != src_ptr[i]) {
                fprintf(stderr, "Mismatch at byte %zu: dst=0x%02x src=0x%02x\n",
                        i, dst_ptr[i], src_ptr[i]);
                mismatch = 1;
                break;
            }
        }

        if (mismatch) {
            rc = -EIO;
        } else {
            printf("Data verified successfully.\n");
        }
    }

    if (!rc) {
        printf("Done.\n");
    }

 done:
    if (verify) {
        unmap_phys(&src_map);
        unmap_phys(&dst_map);
        if (memfd >= 0) {
            close(memfd);
        }
    }

    if (regs) {
        munmap((void *)regs, regs_size);
    }

    return rc ? 1 : 0;
}
