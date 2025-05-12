#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/kdasics.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of_address.h>
#include <linux/gfp.h>
#include <asm/page.h>

MODULE_LICENSE("GPL");

#define SHARE_MEM_SIZE (1 << 30) // 1GB
#define PAGE_SIZE (1 << 12) // 4KB
#define SQ_CQ_SIZE (1 << 7) // 128
#define SQ_BASE_OFFSET 0x0
#define CQ_BASE_OFFSET 0x8
#define SQ_TAIL_OFFSET PAGE_SIZE
#define SQ_HEAD_OFFSET (PAGE_SIZE + 0x8)
#define CQ_TAIL_OFFSET (PAGE_SIZE + 0x10)
#define CQ_HEAD_OFFSET (PAGE_SIZE + 0x18)
#define FILE_BUF_OFFSET (1 << 20) // 1MB

/* 
    share mem layout
    +---------------------------+ base
    | sq base reg | cq base reg |
    |    64-bit   |    64-bit   |
    +---------------------------+ base + 4KB
    | sq tail reg | sq head reg | cq tail reg | cq head reg |
    |    64-bit   |    64-bit   |    64-bit   |    64-bit   |
    +---------------------------+ base + 1MB
    |   buffer for big files received from host   |
    +---------------------------+ base + 1GB
*/

static void __iomem *share_mem_virt;
static uint64_t *sq_base_reg;
static uint64_t *cq_base_reg;
static uint64_t *sq_tail_reg;
static uint64_t *sq_head_reg;
static uint64_t *cq_tail_reg;
static uint64_t *cq_head_reg;
static uint64_t *file_buf;

enum sqe_type{
    SQE_TX = 1,
    SQE_RX = 2,
    SQE_FILE = 3
};

enum cqe_type{
    CQE_TX = 1,
    CQE_RX = 2,
    CQE_FILE = 3
};

static struct sqe{
    uint32_t type;
    uint32_t id;
    uint64_t len;
    uint64_t phys_buf;
    uint64_t virt_buf;
    /* 
    63: 0 processed, 1 processing 
    31-0: message
    */
    uint64_t status;
};

static struct cqe {
    uint32_t type;
    uint32_t id;
    uint64_t reserved0;
    uint64_t reserved1;
    /* 
    63: 0 processed, 1 processing 
    31-0: message
    */
    uint64_t status;
};

static struct sqe *sqes;
static struct cqe *cqes;

uint64_t local_sq_head = 0;
uint64_t local_sq_tail = 0;
uint64_t local_cq_head = 0;
uint64_t local_cq_tail = 0;

//uint64_t *smaincall_entry(uint64_t arg1, uint64_t arg2);

static uint64_t sq_full(void)
{
    return ((local_sq_tail + 1) % SQ_CQ_SIZE == local_sq_head);
}

static uint64_t cq_full(void)
{
    return ((local_cq_tail + 1) % SQ_CQ_SIZE == local_cq_head);
}

static uint64_t cq_empty(void)
{
    return (local_cq_head == local_cq_tail);
}

static int e1000_init_share_mem(void)
{
    uint64_t phys_addr;
    uint32_t size;
    struct device_node *np;
    struct resource res;
    np = of_find_node_by_name(NULL, "my_reserved");
    if (!np) {
        pr_err("Device tree node 'my_reserved' not found\n");
        return -ENODEV;
    }
    if (of_address_to_resource(np, 0, &res)) {
        pr_err("Failed to get resource\n");
        return -EINVAL;
    }
    phys_addr = res.start;
    size = resource_size(&res);

    // 直接映射到指定虚拟地址（需确保地址未被占用）
    share_mem_virt = ioremap(phys_addr, size);
    if (!share_mem_virt) {
        pr_err("ioremap failed\n");
        return -ENOMEM;
    }
    memset(share_mem_virt, 0x0, size);
    pr_info("Reserved memory mapped at virtual address: 0x%llx -> 0x%llx\n",
            (u64)phys_addr, share_mem_virt);
    return 0;
}

static void e1000_init_mmio_regs(void)
{
    sq_base_reg = (uint64_t *)share_mem_virt;
    cq_base_reg = (uint64_t *)(share_mem_virt + CQ_BASE_OFFSET);
    sq_tail_reg = (uint64_t *)(share_mem_virt + SQ_TAIL_OFFSET);
    sq_head_reg = (uint64_t *)(share_mem_virt + SQ_HEAD_OFFSET);
    cq_tail_reg = (uint64_t *)(share_mem_virt + CQ_TAIL_OFFSET);
    cq_head_reg = (uint64_t *)(share_mem_virt + CQ_HEAD_OFFSET);
    file_buf = (uint64_t *)(share_mem_virt + FILE_BUF_OFFSET);
    *sq_base_reg = __virt_to_phys(sqes);
    *cq_base_reg = __virt_to_phys(cqes);
    *sq_tail_reg = 0;
    *sq_head_reg = 0;
    *cq_tail_reg = 0;
    *cq_head_reg = 0;
    pr_info("sq base reg addr: 0x%llx\n", sq_base_reg);
    pr_info("cq base reg addr: 0x%llx\n", cq_base_reg);
    pr_info("sq base phys addr: 0x%llx\n", *sq_base_reg);
    pr_info("cq base phys addr: 0x%llx\n", *cq_base_reg);
    pr_info("sq tail reg: 0x%llx\n", sq_tail_reg);
    pr_info("sq head reg: 0x%llx\n", sq_head_reg);
    pr_info("cq tail reg: 0x%llx\n", cq_tail_reg);
    pr_info("cq head reg: 0x%llx\n", cq_head_reg);
    pr_info("file buf: 0x%llx\n", file_buf);
    pr_info("share mem virt: 0x%llx\n", share_mem_virt);
}

static uint64_t e1000_init_qps(void)
{
    sqes = (struct sqe *)kmalloc(SQ_CQ_SIZE * sizeof(struct sqe), GFP_KERNEL | GFP_DMA);
    if (!sqes) {
        pr_err("alloc sqes failed\n");
        return -1;
    }
    memset(sqes, 0x0, SQ_CQ_SIZE * sizeof(struct sqe));
    cqes = (struct cqe *)kmalloc(SQ_CQ_SIZE * sizeof(struct cqe), GFP_KERNEL | GFP_DMA);
    if (!cqes) {
        pr_err("alloc cqes failed\n");
        kfree(sqes);
        return -1;
    }
    memset(cqes, 0x0, SQ_CQ_SIZE * sizeof(struct cqe));
    return 0;
}

static struct sqe* e1000_alloc_sqe(uint64_t *buf, uint64_t len, uint32_t type)
{
    struct sqe *sqe = (struct sqe*)kmalloc(sizeof(struct sqe), GFP_KERNEL | GFP_DMA);
    if (!sqe) {
        pr_err("alloc sqe failed\n");
        return NULL;
    }
    memset(sqe, 0x0, sizeof(struct sqe));
    sqe->type = type;
    sqe->id = local_sq_tail;
    sqe->len = len;
    sqe->virt_buf = (uint64_t)buf;
    sqe->phys_buf = __virt_to_phys(buf);
    pr_info("sqe id: %d, vbuf: 0x%llx, pbuf: 0x%llx, len: %lld type: %lx\n", 
        sqe->id, sqe->virt_buf, sqe->phys_buf,sqe->len, sqe->type);
    return sqe;
}

static void e1000_ring_sq_db(uint64_t sq_tail)
{
    *sq_tail_reg = sq_tail;
}

static uint64_t* e1000_alloc_dma_buf(uint64_t len)
{
    uint64_t *buf = kmalloc(len, GFP_KERNEL | GFP_DMA);
    if (!buf) {
        pr_err("alloc dma buf failed\n");
        return NULL;
    }
    memset(buf, 0xAB, len);
    return buf;
}

static uint64_t e1000_free_dma_buf(uint64_t *buf)
{
    kfree(buf);
    return 0;
}

static uint64_t* e1000_alloc_big_dma_buf(void)
{
    return file_buf;
}

static uint64_t e1000_process_cqe(void)
{
    *cq_head_reg = *cq_tail_reg;
    return 0;
}

static uint64_t e1000_poll_cq(void)
{
    //while(cq_empty());
    //struct cqe cqe = cqes[local_cq_head];
    //if (cqe is not good) return -1;
    return 0;
}

static uint64_t e1000_transfer_packet(uint32_t type)
{
    uint64_t len = 0x1000;
    uint64_t *buf = e1000_alloc_dma_buf(len);
    if (!buf) {
        pr_err("alloc tx buf failed\n");
        return -1;
    }
    struct sqe *sqe = e1000_alloc_sqe(buf, len, type);
    if (!sqe) {
        pr_err("alloc sqe failed\n");
        e1000_free_dma_buf(buf);
        return -1;
    }
    if (sq_full() || cq_full()) {
        pr_err("sq or cq is full\n");
        e1000_free_dma_buf(buf);
        kfree(sqe);
        return -1;
    }
    memcpy(sqes + local_sq_tail, sqe, sizeof(struct sqe));
    local_sq_tail = (local_sq_tail + 1) % SQ_CQ_SIZE;
    e1000_ring_sq_db(local_sq_tail);
    if (e1000_poll_cq()) {
        pr_err("cq failed\n");
        e1000_free_dma_buf(buf);
        kfree(sqe);
        return -1;
    }
    if (e1000_process_cqe()) {
        pr_err("process cqe failed\n");
        e1000_free_dma_buf(buf);
        kfree(sqe);
        return -1;
    }
    e1000_free_dma_buf(buf);
    kfree(sqe);
    pr_info("%s packet success\n", type == SQE_TX ? "send" : type == SQE_RX ? "recv" : "recv file");
    return 0;
}

static int init_e1000(void)
{
    pr_info("hello e1000\n");
    int ret;
    //ret = init_dev_reg();
    ret = e1000_init_share_mem();
    if (ret) {
        pr_err("init share mem failed\n");
        return ret;
    }
    e1000_init_qps();
    if (ret) {
        pr_err("init qps failed\n");
        return ret;
    }
    e1000_init_mmio_regs();
    if(e1000_transfer_packet(SQE_TX))
        pr_err("send packet failed\n");
    if(e1000_transfer_packet(SQE_RX))
        pr_err("recv packet failed\n");
    return 0;
}

static void exit_e1000(void)
{
    pr_notice("exit driver\n");
}

module_init(init_e1000);
module_exit(exit_e1000);