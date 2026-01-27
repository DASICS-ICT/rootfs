/* dbchecker.h - Userspace DBChecker public definitions */
#ifndef LIB_DBCHECKER_H
#define LIB_DBCHECKER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Minimal local replacements for kernel types used by original API */
typedef uint64_t dma_addr_t;
enum dma_data_direction {
    DMA_BIDIRECTIONAL = 0,
    DMA_FROM_DEVICE = 1,
    DMA_TO_DEVICE = 2
};

#define DBCHECKER_BASE_ADDR 0x40000000ULL
#define DBCHECKER_REG_SIZE 4 /* 32bit */
#define DBCHECKER_REG_NUM 10 /* 10 registers */

#define DBCHECKER_EN_OFFSET          0x00U
#define DBCHECKER_CMD_OFFSET         0x04U
#define DBCHECKER_DBTE_MB_LO_OFFSET  0x08U
#define DBCHECKER_DBTE_MB_HI_OFFSET  0x0CU
#define DBCHECKER_ERR_ADDR_LO_OFFSET 0x10U
#define DBCHECKER_ERR_ADDR_HI_OFFSET 0x14U
#define DBCHECKER_ERR_INFO_OFFSET    0x18U
#define DBCHECKER_ERR_CNT_OFFSET     0x1CU

#define MAX_DBTE_TABLE_SIZE 65536
#define DBTE_TABLE_PHYS_ADDR 0xF0000000ULL

enum dbchecker_cmd_op {
  DBCHECKER_OP_FREE,
  DBCHECKER_OP_CLEAR,
};

enum dbchecker_rw_mode {
    DBCHECKER_RWMODE_INVALID,
    DBCHECKER_RWMODE_RO,
    DBCHECKER_RWMODE_WO,
    DBCHECKER_RWMODE_RW
};

static const enum dbchecker_rw_mode dma_to_db_map[] = {
    [DMA_BIDIRECTIONAL] = DBCHECKER_RWMODE_RW, // Index 0 -> Value 3
    [DMA_FROM_DEVICE]   = DBCHECKER_RWMODE_WO, // Index 1 -> Value 2
    [DMA_TO_DEVICE]     = DBCHECKER_RWMODE_RO  // Index 2 -> Value 1
};

// we assume that we use little-endian system
// struct dbchecker_mtdt {
//     uint64_t lo_bnd    : 48;
//     uint64_t up_bnd_lo : 16;
//     uint64_t up_bnd_hi : 32;
//     uint64_t dev_id    : 5;
//     uint64_t wr        : 2;
//     uint64_t v         : 1;
//     uint64_t non_cached: 1;
//     uint64_t reserved  : 19;
//     uint64_t index_off : 4;
// }__attribute__((packed));

typedef union {
    struct {
        // --- Word 0 (low 64) ---
        uint64_t lo_bnd    : 48;
        uint64_t up_bnd_lo : 16;
        
        // --- Word 1 (hi 64) ---
        uint64_t up_bnd_hi : 32;
        uint64_t dev_id    : 5;
        uint64_t wr        : 2;
        uint64_t v         : 1;
        uint64_t non_cached: 1;
        uint64_t reserved  : 19;
        uint64_t index_off : 4;
    } __attribute__((packed));

    struct {
        uint64_t raw0; // word 0
        uint64_t raw1; // word 1
    };
} dbchecker_mtdt_u;

// struct dbchecker_cmd {
//   uint32_t imm    : 30; /* index of the mtdt to be cleaned */
//   uint32_t op     : 1;
//   uint32_t v      : 1;
// }__attribute__((packed));

typedef union {
    struct {
        uint32_t imm : 30; // 低 30 位
        uint32_t op  : 1;  // 第 31 位
        uint32_t v   : 1;  // 第 32 位 (最高位)
    };
    uint32_t raw; // 整个 4 字节视图
} __attribute__((packed, aligned(4))) dbchecker_cmd_u;

#define DBCHECKER_ENABLE_MASK 0x3UL /* bypass device 31 by default */
#define DBCHECKER_DISABLE_MASK 0x0UL
#define UNTRUST_DEV_ID 0x0U

#define DBCHECKER_DEBUG 0

#define DBCHECKER_DEBUG_LOG(fmt, args...) \
        do { \
                if (DBCHECKER_DEBUG) \
                        printf(fmt, ##args); \
        } while (0)

#define wmb()		__asm__ __volatile__ ("fence w, w" : : : "memory")

/* Public API */
int dbchecker_init(void);
void dbchecker_exit(void);
int dbchecker_command(uint32_t cmd);
void dbchecker_en_set(uint32_t dev_mask);
uint32_t dbchecker_en_get(void);
dma_addr_t dbchecker_alloc_mtdt(dma_addr_t addr, size_t size, enum dma_data_direction dir);
dma_addr_t dbchecker_free_mtdt(dma_addr_t addr);
int dbchecker_activate_mtdt(dma_addr_t addr, enum dma_data_direction dir, uint16_t dev_id, bool is_non_cached);
int dbchecker_deactivate_mtdt(dma_addr_t addr);
void dbchecker_free_all_mtdt(void);
int dbchecker_err_handler(void);

#endif /* LIB_DBCHECKER_DBCHECKER_H */
