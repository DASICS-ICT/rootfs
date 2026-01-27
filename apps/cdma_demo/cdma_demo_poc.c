#define _FILE_OFFSET_BITS 64
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "udasics.h"
#include "dbchecker.h"
#include "uio_utils.h"

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

#define RESET_WAIT_US               1000
#define RESET_TRIES                 50
#define IDLE_POLL_US                1000

/* Fixed PoC parameters: adjust to your platform. */

static char *secret = "SECRET_DATA_1234\0";

#define POC_SRC_PHYS                0xF1000000ULL
#define POC_DST_PHYS                0xF1001000ULL
#define POC_SRC_VIRT                (g_ctx.src_map.virt + g_ctx.src_map.page_offset)
#define POC_DST_VIRT                (g_ctx.dst_map.virt + g_ctx.dst_map.page_offset)
#define POC_LEN_BYTES               (strlen(secret) + 1)
#define POC_TIMEOUT_MS              1000

struct phys_map {
    uint8_t *virt;
    size_t map_len;
    size_t page_offset;
};

struct cdma_context {
    volatile uint8_t *regs;
    size_t regs_size;
    int memfd;
    struct phys_map src_map;
    struct phys_map dst_map;
};

static struct cdma_context ATTR_ULIB_DATA g_ctx = {
    .memfd = -1,
};

static int cdma_reset(volatile uint8_t *regs)
{
    mmio_write32((void *)regs, XAXICDMA_CR_OFFSET, XAXICDMA_CR_RESET_MASK);

    for (int i = 0; i < RESET_TRIES; ++i) {
        if ((mmio_read32(regs, XAXICDMA_CR_OFFSET) & XAXICDMA_CR_RESET_MASK) == 0) {
            return 0;
        }
        usleep(RESET_WAIT_US);
    }

    fprintf(stderr, "CDMA reset timed out\n");
    return -ETIMEDOUT;
}

static int cdma_wait_idle(volatile uint8_t *regs, int timeout_ms)
{
    int waited_ms = 0;

    while (waited_ms <= timeout_ms) {
        uint32_t sr = mmio_read32(regs, XAXICDMA_SR_OFFSET);

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

    uint32_t cr = mmio_read32(regs, XAXICDMA_CR_OFFSET);
    if (cr & XAXICDMA_CR_SGMODE_MASK) {
        fprintf(stderr, "CDMA is in scatter-gather mode; simple mode required\n");
        return -EINVAL;
    }

    /* clear err: Write 1s to clear sticky status bits. */
    mmio_write32((void *)regs, XAXICDMA_SR_OFFSET, XAXICDMA_SR_ERR_ALL_MASK | XAXICDMA_XR_IRQ_ALL_MASK);

    int rc = cdma_wait_idle(regs, timeout_ms);
    if (rc) {
        fprintf(stderr, "CDMA not idle before start (%d)\n", rc);
        return rc;
    }

    mmio_write32((void *)regs, XAXICDMA_SRCADDR_OFFSET, (uint32_t)(src & 0xFFFFFFFFu));
    mmio_write32((void *)regs, XAXICDMA_SRCADDR_MSB_OFFSET, (uint32_t)(src >> 32));
    mmio_write32((void *)regs, XAXICDMA_DSTADDR_OFFSET, (uint32_t)(dst & 0xFFFFFFFFu));
    mmio_write32((void *)regs, XAXICDMA_DSTADDR_MSB_OFFSET, (uint32_t)(dst >> 32));

    /* Writing BTT kicks off the transfer. */
    mmio_write32((void *)regs, XAXICDMA_BTT_OFFSET, length);

    rc = cdma_wait_idle(regs, timeout_ms);
    if (rc) {
        fprintf(stderr, "CDMA timed out waiting for completion (%d)\n", rc);
    }

    return rc;
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

static int cdma_init(void)
{
    struct cdma_context *ctx = &g_ctx;
    memset(ctx, 0, sizeof(*ctx));
    ctx->memfd = -1;

    const char *uio_path = NULL;
    char auto_uio[256];
    if (find_uio_device_by_name("axi_cdma_uio", auto_uio, sizeof(auto_uio)) == 0) {
        uio_path = auto_uio;
    } else {
        fprintf(stderr, "error: no CDMA UIO device\n");
        return -ENODEV;
    }

    int rc = uio_map_regs(uio_path, &ctx->regs, &ctx->regs_size);
    if (rc) return rc;

    printf("CDMA regs mapped: %p (size 0x%zx)\n", (void *)ctx->regs, ctx->regs_size);
    printf("Transfer: src 0x%016" PRIx64 " -> dst 0x%016" PRIx64 " len %u bytes\n",
           (uint64_t)POC_SRC_PHYS, (uint64_t)POC_DST_PHYS, (unsigned)POC_LEN_BYTES);

    ctx->memfd = open("/dev/mem", O_RDWR | O_SYNC);
    if (ctx->memfd < 0) {
        perror("open mem");
        return -errno;
    }

    const size_t page_size = (size_t)sysconf(_SC_PAGESIZE);
    const size_t map_len = ((POC_LEN_BYTES + page_size - 1) / page_size + 1) * page_size; /* at least two pages */

    rc = map_phys_range(ctx->memfd, POC_SRC_PHYS, map_len, &ctx->src_map);
    if (rc) return rc;

    rc = map_phys_range(ctx->memfd, POC_DST_PHYS, map_len, &ctx->dst_map);
    if (rc) return rc;

    uint8_t *src_ptr = ctx->src_map.virt + ctx->src_map.page_offset;
    uint8_t *dst_ptr = ctx->dst_map.virt + ctx->dst_map.page_offset;
    for (size_t i = 0; i < POC_LEN_BYTES; ++i) {
        src_ptr[i] = (uint8_t)(i & 0xFF);
        dst_ptr[i] = 0;
    }
    msync(ctx->src_map.virt, ctx->src_map.map_len, MS_SYNC);
    msync(ctx->dst_map.virt, ctx->dst_map.map_len, MS_SYNC);
    return 0;
}

static void cdma_exit(void)
{
    struct cdma_context *ctx = &g_ctx;
    unmap_phys(&ctx->src_map);
    unmap_phys(&ctx->dst_map);
    if (ctx->memfd >= 0) close(ctx->memfd);
    if (ctx->regs) munmap((void *)ctx->regs, ctx->regs_size);
}

static int cdma_verify(void)
{
    struct cdma_context *ctx = &g_ctx;
    if (!ctx->src_map.virt || !ctx->dst_map.virt) return -EINVAL;

    msync(ctx->dst_map.virt, ctx->dst_map.map_len, MS_SYNC);

    uint8_t *src_ptr = ctx->src_map.virt + ctx->src_map.page_offset;
    uint8_t *dst_ptr = ctx->dst_map.virt + ctx->dst_map.page_offset;
    for (size_t i = 0; i < POC_LEN_BYTES; ++i) {
        if (dst_ptr[i] != src_ptr[i]) {
            fprintf(stderr, "Mismatch at byte %zu: dst=0x%02x src=0x%02x\n",
                    i, dst_ptr[i], src_ptr[i]);
            return -EIO;
        }
    }
    printf("Data verified successfully.\n");
    return 0;
}


int origin_attack(void){
    // simple mem/dev attack without any protection; can be better
    printf("[ORIGIN_ULIB] try mem attack\n");
    printf("secret in origin buffer: %s\n", (char *)POC_SRC_VIRT);
    printf("[ORIGIN_ULIB] try dev attack\n");
    if (cdma_simple_transfer(g_ctx.regs, POC_SRC_PHYS, POC_DST_PHYS, POC_LEN_BYTES, POC_TIMEOUT_MS) == 0) {
        printf("[CDMA] Transfer completed.\n");
        printf("secret in copied buffer: %s\n", (char *)POC_DST_VIRT);
    }
    else return -1;
}

int ATTR_ULIB_TEXT dasics_attack(void){
    //......
}

int main(void)
{
    int rc;
    // initialize cdma
    if ((rc = cdma_init())){
        fprintf(stderr, "CDMA init failed: %d\n", rc);
        goto done;
    }
    if ((rc = cdma_reset(g_ctx.regs))) {
        fprintf(stderr, "CDMA reset failed: %d\n", rc);
        goto done;
    }

    printf("DASICS + DBChecker CDMA PoC starting...\n");

    //prepare secret data
    strncpy((char *)POC_SRC_VIRT, secret, POC_LEN_BYTES);
    printf("Prepared secret data: %s.\n", (char *)POC_SRC_VIRT);

/*
    Todo PoC:
    1. no dasics, no dbchecker: mem/dev attack complete
    2. with dasics, no dbchecker: mem attack blocked, dev attack complete
    3. with dasics + dbchecker: mem/dev attack blocked
*/ 

    printf("Case 1: no DASICS, no DBChecker\n");
    origin_attack();

    printf("Case 2: with DASICS, no DBChecker\n");
    register_udasics(0);
    // ... (alloc dasics metadata)
    dasics_attack();

    printf("Case 3: with DASICS + DBChecker\n");
    if (dbchecker_init()) return 1;
    // ... (alloc dbchecker metadata)
    dasics_attack();

    // ===== ref code =====
    // rc = cdma_simple_transfer(g_ctx.regs, POC_SRC_PHYS, POC_DST_PHYS, POC_LEN_BYTES, POC_TIMEOUT_MS);
    // if (rc == 0) {
    //     printf("Transfer completed.\n");
    // }

    // if (rc == 0) {
    //     rc = cdma_verify();
    // }
    // ====================
    
    if (!rc) printf("DASICS + DBChecker CDMA PoC: Test done.\n");

 done:
    unregister_udasics();
    dbchecker_exit();
    cdma_exit();
    return rc ? 1 : 0;
}

