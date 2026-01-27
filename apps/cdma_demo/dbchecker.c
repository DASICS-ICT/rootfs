/*
 * User-space DBChecker driver for DPDK-like environment.
 * This file replaces the previous kernel module implementation.
 * It accesses device registers via /dev/uio0 using pread/pwrite.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <inttypes.h>
#include <dirent.h>
#include <limits.h>
#include <sys/mman.h>

/* public declarations and definitions */
#include "dbchecker.h"
#include "uio_utils.h"

//#define TEST_DBTE_CACHE_HIT

static uint16_t dbte_alloc_id = 0;
static uint8_t dbchecker_enable = 0;
static char uio_device[256] = "/dev/uio0";
static int uio_fd = -1;
/* mmap'ed region base and size */
static void *uio_map = NULL;
static size_t uio_map_size = 0;
static dbchecker_mtdt_u *dbte_table;
static void *dbte_table_map_base = NULL;
static size_t dbte_table_map_len = 0;


/*
    --- physical memory layout ---  
    0xF0000000 - 0xF000FFFF : DBTE table (64K entries, 16 bytes each)
    0xF1000000 - 0xF1FFFFFF : DMA zone for CDMA PoC testing
*/ 

static int map_dbte_table_from_phys(void)
{
    const size_t table_size = sizeof(dbchecker_mtdt_u) * MAX_DBTE_TABLE_SIZE;
    const size_t page_size = (size_t)sysconf(_SC_PAGESIZE);
    const uint64_t page_mask = ~(uint64_t)(page_size - 1U);

    const uint64_t page_base = DBTE_TABLE_PHYS_ADDR & page_mask;
    const size_t page_offset = (size_t)(DBTE_TABLE_PHYS_ADDR - page_base);
    dbte_table_map_len = page_offset + table_size;

    int memfd = open("/dev/mem", O_RDWR | O_SYNC);
    if (memfd < 0) {
        perror("open /dev/mem");
        dbte_table_map_len = 0;
        return -errno;
    }

    void *map_base = mmap(NULL, dbte_table_map_len, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, (off_t)page_base);
    close(memfd);

    if (map_base == MAP_FAILED) {
        perror("mmap dbte table");
        dbte_table_map_len = 0;
        return -errno;
    }

    dbte_table_map_base = map_base;
    dbte_table = (dbchecker_mtdt_u *)((uint8_t *)map_base + page_offset);
    return 0;
}

/*
 * dbte_alloc_id layout: [ group (12 bits) | offset (4 bits) ]
 * Increment order: increment group first (0..4095), then offset (0..15).
 */
static inline uint16_t dbte_next_id(uint16_t id)
{
    uint16_t offset = id & 0xFULL;
    uint16_t group = id >> 4;
    group++;
    if (group > 0xFFF) {
        group = 0;
        offset = (offset + 1) & 0xFULL; /* wrap offset mod 16 */
    }
    return (uint16_t)((group << 4) | (offset & 0xF));
}

int dbchecker_command(uint32_t cmd){
    //printf("dbchecker_command: cmd=0x%08x\n", hw_cmd);
    mmio_write32(uio_map, DBCHECKER_CMD_OFFSET, cmd);
    return 0;
}

void dbchecker_en_set(uint32_t dev_mask){
    mmio_write32(uio_map, DBCHECKER_EN_OFFSET, dev_mask);
}

uint32_t dbchecker_en_get(void){
    return mmio_read32(uio_map, DBCHECKER_EN_OFFSET);
}


dma_addr_t dbchecker_alloc_mtdt(dma_addr_t addr, size_t size, enum dma_data_direction dir){
    //if (!(dbchecker_en_get() & 0xFFFFFFFF))
    //    return addr; // not enabled

    // use global flag to avoid mmio
    if (!dbchecker_enable)
        return addr; // not enabled

    dbchecker_mtdt_u mtdt;
    if (dir <= DMA_TO_DEVICE) {
        mtdt.wr = dma_to_db_map[dir];
    } else {
        mtdt.wr = DBCHECKER_RWMODE_INVALID;
    }

    dma_addr_t alloc_addr = (dma_addr_t)-1;
    mtdt.lo_bnd = addr & 0xFFFFFFFFFFFFULL;
    mtdt.up_bnd_lo = (uint16_t)((addr + size) & 0xFFFFULL);
    mtdt.up_bnd_hi = (uint32_t)(((addr + size) >> 16) & 0xFFFFFFFFUL);
    mtdt.dev_id = UNTRUST_DEV_ID;

    /* Find a free slot starting at current counter. The counter encodes
     * [group:12 | offset:4] and increments group first then offset.
     * If the current slot is occupied (v != 0), walk forward until a
     * slot with v == 0 is found or we wrap back to start -> failure.
     */
    uint16_t start = dbte_alloc_id;
    uint16_t idx = start;
    bool found = false;
    const uint64_t v_mask = (1ULL << 39); 
    do {
        if ((dbte_table[idx].raw1 & v_mask) == 0) {
            found = true;
            break;
        }
        idx = dbte_next_id(idx);
    } while (idx != start);

    if (!found) {
        printf("DBCHECKER: alloc failed, table full (start idx %u)\n", start);
        return alloc_addr; /* -1 */
    }

    /* fill index_off from low 4 bits (offset) as before */
    mtdt.index_off = (idx & 0xFUL);
    #ifdef TEST_DBTE_CACHE_HIT
        mtdt.v = 1;
    #else
        mtdt.v = 0;
    #endif

    /* construct returned iova with table index in high bits as previous design */
    alloc_addr = (addr & 0xFFFFFFFFFFFFULL) | ((uint64_t)idx << 48);

    /* store copy of mtdt at found index and advance allocation cursor to next position */
    dbte_table[idx].raw0 = mtdt.raw0;
    wmb();
    dbte_table[idx].raw1 = mtdt.raw1;
    dbte_alloc_id = dbte_next_id(idx);
    // DBCHECKER_DEBUG_LOG("DBCHECKER: alloc addr: 0x%llx, save metadata idx %zu\n",
    //     (unsigned long long)alloc_addr, idx);
    return alloc_addr;
}


dma_addr_t dbchecker_free_mtdt(dma_addr_t addr){
    //if (!(dbchecker_en_get() & 0xFFFFFFFF)) 
    //    return addr; // dbchecker not enabled

    // use global flag to avoid mmio
    if (!dbchecker_enable)
        return addr; // dbchecker not enabled

    uint16_t index = (uint16_t)((addr >> 48) & 0xFFFFUL);
    //printf("dbchecker_free_mtdt\n");

    if (index >= MAX_DBTE_TABLE_SIZE) {
        printf("DBCHECKER Error: free mtdt failed, index %u out of bounds (Max %u)\n", 
           index, MAX_DBTE_TABLE_SIZE);
        return (dma_addr_t)-1; 
    }

    dbchecker_mtdt_u mtdt;
    mtdt.raw1 = dbte_table[index].raw1;
    mtdt.v = 0;

    dbte_table[index].raw1 = mtdt.raw1;
    wmb();
    if (!mtdt.non_cached){
        dbchecker_cmd_u free_cmd = {
            .imm = index,
            .op  = DBCHECKER_OP_FREE,
            .v   = 1
        };
        dbchecker_command(free_cmd.raw);
    }

    // DBCHECKER_DEBUG_LOG("DBCHECKER: free addr: 0x%llx\n", (unsigned long long)addr);
    
    return addr & 0xFFFFFFFFFFFFULL; // orig addr
}

void dbchecker_free_all_mtdt(void){
    //printf("dbchecker_free_all_mtdt\n");    
    //printf("free dbte table\n");
    dbchecker_cmd_u free_cmd = {
        .imm = 1 << 16,
        .op  = DBCHECKER_OP_FREE,
        .v   = 1
    };
    dbchecker_command(free_cmd.raw);
    //printf("submit clear all cmd\n");
}


int dbchecker_err_handler(void){
    uint32_t cnt = mmio_read32(uio_map, DBCHECKER_ERR_CNT_OFFSET);
    uint32_t info = mmio_read32(uio_map, DBCHECKER_ERR_INFO_OFFSET);
    uint32_t addr_lo = mmio_read32(uio_map, DBCHECKER_ERR_ADDR_LO_OFFSET);
    uint32_t addr_hi = mmio_read32(uio_map, DBCHECKER_ERR_ADDR_HI_OFFSET);
    uint64_t addr = ((uint64_t)addr_hi << 32) | addr_lo;
    uint16_t index =  (uint16_t)((addr >> 48) & 0xFFFFUL);
    if (cnt & ~0xF){
        fprintf(stderr, "DBCHECKER: error detected!\n");
        fprintf(stderr, "DBCHECKER: error count: 0x%llx, info: 0x%llx, addr: 0x%llx\n",
            (unsigned long long)cnt, (unsigned long long)info, (unsigned long long)addr);
        fprintf(stderr, "DBCHECKER: error mtdt raw0 : 0x%llx, raw1: 0x%llx\n",
         (unsigned long long)dbte_table[index].raw0, (unsigned long long)dbte_table[index].raw1);
        // err cnt format:| cnt3(7) | cnt2(7) | cnt1(7) | cnt0(7) | latest err(4) |
        // cnt0: cross boundary violation
        // cnt1: write-read violation
        // cnt2: invalid metadata
        // cnt3: device mismatch
        fprintf(stderr, "DBCHECKER: error type: %s\n",
            ((cnt >> 4)  & 0x7F) ? "cross boundary violation" :
            ((cnt >> 11) & 0x7F) ? "write-read violation" :
            ((cnt >> 18) & 0x7F) ? "invalid metadata" :
            ((cnt >> 25) & 0x7F) ? "device mismatch" : "unknown");
        dbchecker_cmd_u err_cmd = {
            .op  = DBCHECKER_OP_CLEAR,
            .v   = 1
        };
        dbchecker_command(err_cmd.raw);
        return -1;
    }
    return 0;
}

int dbchecker_activate_mtdt(dma_addr_t addr, enum dma_data_direction dir, uint16_t dev_id, bool is_non_cached){
    #ifndef TEST_DBTE_CACHE_HIT
        if (!dbchecker_enable)
            return 0; // dbchecker not enabled

        uint16_t index = (uint16_t)((addr >> 48) & 0xFFFFUL);
        dbchecker_mtdt_u mtdt;
        mtdt.raw1 = dbte_table[index].raw1;
        if (dir <= DMA_TO_DEVICE) {
            mtdt.wr = dma_to_db_map[dir];
        } else {
            mtdt.wr = DBCHECKER_RWMODE_INVALID;
        }
        mtdt.dev_id = dev_id;
        mtdt.non_cached = is_non_cached ? 1 : 0;
        mtdt.v = 1;
        dbte_table[index].raw1 = mtdt.raw1;
        // printf(" DBCHECKER: activate addr: 0x%llx, index %x, wr %x, dev_id %x raw1 %llx raw0 %llx\n",
        //     (unsigned long long)addr, index, mtdt.wr, mtdt.dev_id, mtdt.raw1, mtdt.raw0);
    #endif
    return 0;
}

int dbchecker_deactivate_mtdt(dma_addr_t addr){
    #ifndef TEST_DBTE_CACHE_HIT
        if (!dbchecker_enable) 
            return 0; // dbchecker not enabled

        uint16_t index = (uint16_t)((addr >> 48) & 0xFFFFUL);
        dbchecker_mtdt_u mtdt;
        mtdt.raw1 = dbte_table[index].raw1;
        mtdt.v = 0;
        dbte_table[index].raw1 = mtdt.raw1;
        // printf("deactivate mtdt index %x valid %llx\n", index, (unsigned long long)dbte_table[index].v);
        wmb();
        if (!mtdt.non_cached){
            dbchecker_cmd_u free_cmd = {
                .imm = index,
                .op  = DBCHECKER_OP_FREE,
                .v   = 1
            };
            //printf("deactivate index %x\n", index);
            return dbchecker_command(free_cmd.raw);
        }
        else return 0;
    #else
        return 0;
    #endif
}


// static void *err_thread_fn(void *arg)
// {
//     (void)arg;
//     while (err_thread_running) {
//         dbchecker_err_handler();
//         usleep(1000 * 1000); /* 1000 ms */
//     }
//     return NULL;
// }

/* Initialize user-space DBChecker (open UIO, start poll thread) */
int dbchecker_init(int enable_mask)
{

    /* try to locate device by name "dbchecker_uio" */
    char found[256];
    if (find_uio_device_by_name("dbchecker_uio", found, sizeof(found)) == 0) {
        strncpy(uio_device, found, sizeof(uio_device) - 1);
        uio_device[sizeof(uio_device) - 1] = '\0';
    }
    else {
        printf("DBCHECKER: could not find uio device by name 'dbchecker_uio'\n");
        return -1;
    }
    if (uio_map_regs(uio_device, (volatile uint8_t **)&uio_map, &uio_map_size) != 0) {
        fprintf(stderr, "Failed to map %s\n", uio_device);
        uio_map = NULL;
        uio_map_size = 0;
        return -1;
    }

    if (map_dbte_table_from_phys() != 0) {
        if (uio_map) {
            munmap(uio_map, uio_map_size);
            uio_map = NULL;
            uio_map_size = 0;
        }
        return -1;
    }

    mmio_write32(uio_map, DBCHECKER_DBTE_MB_LO_OFFSET, (uint32_t)(DBTE_TABLE_PHYS_ADDR & 0xFFFFFFFFUL));
    mmio_write32(uio_map, DBCHECKER_DBTE_MB_HI_OFFSET, (uint32_t)(DBTE_TABLE_PHYS_ADDR >> 32));
    dbchecker_en_set(enable_mask);
    dbchecker_enable = 1;
    printf("DBCHECKER (userspace): init, using %s\n", uio_device);
    return 0;
}

/* Cleanup user-space DBChecker */
void dbchecker_exit(void)
{
    // if (err_thread_running) {
    //     err_thread_running = false;
    //     pthread_join(err_thread, NULL);
    // }
    dbchecker_free_all_mtdt();
    dbchecker_en_set(DBCHECKER_DISABLE_MASK);
    if (dbte_table_map_base) {
        munmap(dbte_table_map_base, dbte_table_map_len);
        dbte_table_map_base = NULL;
        dbte_table_map_len = 0;
        dbte_table = NULL;
    }
    if (uio_map) {
        munmap(uio_map, uio_map_size);
        uio_map = NULL;
        uio_map_size = 0;
    }
    if (uio_fd >= 0) close(uio_fd);
    dbchecker_enable = 0;
    printf("DBCHECKER (userspace): exit\n");
}