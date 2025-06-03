#include <linux/etherdevice.h>
#include <linux/netdevice.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/export.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of_address.h>
#include <linux/gfp.h>
#include <asm/page.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#define DRV_NAME "vnic"

#define fpga

#ifdef fpga
    #include <asm/kdasics.h>
#endif

/*
    share mem layout
    totally 128MB, 64MB for fpga and 64MB for host
    fpga tx == host rx, fpga rx == host tx
    +---------------------------+ base
    |         fpga tx tail        |
    |           64-bit            |
    +---------------------------+ base + 8B
    |         fpga tx head        |
    |           64-bit            |
    +---------------------------+ base + 16B
    |       fpga tx data len      |
    |           64-bit            |
    +---------------------------+ base + 4KB
    |         fpga tx fifo        |
    +---------------------------+ base + 64MB
    |         host tx tail        |
    |           64-bit            |
    +---------------------------+ base + 64MB + 8B
    |         host tx head        |
    |           64-bit            |
    +---------------------------+ base + 64MB + 16B
    |       host tx data len      |
    |           64-bit            |
    +---------------------------+ base + 64MB + 4KB
    |         host tx fifo        |
    +---------------------------+ base + 128MB
*/

#define FPGA_TX_TAIL_OFFSET 0x0
#define FPGA_TX_HEAD_OFFSET 0x4
#define FPGA_TX_DATA_LEN_OFFSET 0x8
#define FPGA_TX_FIFO_OFFSET (1 << 12) // 4KB
#define HOST_TX_TAIL_OFFSET (1 << 26) // 64MB
#define HOST_TX_HEAD_OFFSET (1 << 26) + 0x4
#define HOST_TX_DATA_LEN_OFFSET (1 << 26) + 0x8
#define HOST_TX_FIFO_OFFSET (1 << 26) + (1 << 12) // 64MB + 4KB

#ifdef fpga
    #define TX_TAIL_OFFSET FPGA_TX_TAIL_OFFSET
    #define TX_HEAD_OFFSET FPGA_TX_HEAD_OFFSET
    #define TX_DATA_LEN_OFFSET FPGA_TX_DATA_LEN_OFFSET
    #define TX_FIFO_OFFSET FPGA_TX_FIFO_OFFSET
    #define RX_TAIL_OFFSET HOST_TX_TAIL_OFFSET
    #define RX_HEAD_OFFSET HOST_TX_HEAD_OFFSET
    #define RX_DATA_LEN_OFFSET HOST_TX_DATA_LEN_OFFSET
    #define RX_FIFO_OFFSET HOST_TX_FIFO_OFFSET
#else
    #define TX_TAIL_OFFSET HOST_TX_TAIL_OFFSET
    #define TX_HEAD_OFFSET HOST_TX_HEAD_OFFSET
    #define TX_DATA_LEN_OFFSET HOST_TX_DATA_LEN_OFFSET
    #define TX_FIFO_OFFSET HOST_TX_FIFO_OFFSET
    #define RX_TAIL_OFFSET FPGA_TX_TAIL_OFFSET
    #define RX_HEAD_OFFSET FPGA_TX_HEAD_OFFSET
    #define RX_DATA_LEN_OFFSET FPGA_TX_DATA_LEN_OFFSET
    #define RX_FIFO_OFFSET FPGA_TX_FIFO_OFFSET
#endif

static void __iomem *share_mem_virt;
static uint64_t *tx_tail;
static uint64_t *tx_head;
static uint64_t *tx_data_len;
static void *tx_fifo;
static uint64_t *rx_tail;
static uint64_t *rx_head;
static uint64_t *rx_data_len;
static void *rx_fifo;

static struct net_device *vnic_dev;
static struct task_struct *polling_thread;

static void vnic_setup(struct net_device *dev);
static int vnic_open(struct net_device *dev);
static int vnic_close(struct net_device *dev);
static netdev_tx_t vnic_start_xmit(struct sk_buff *skb, struct net_device *dev);
static int vnic_poll_rx(void *data);

static uint64_t vnic_read_share_mem(uint64_t *addr)
{
    #ifdef fpga
        return *addr;
    #else
        //todo: read via xdma api
        return 0;
    #endif
}

static void vnic_write_share_mem(uint64_t *addr, uint64_t val)
{
    #ifdef fpga
        *addr = val;
    #else
        //todo: write via xdma api
    #endif
}

static void vnic_memcpy_to_share_mem(uint64_t addr, void *data, size_t len)
{
    #ifdef fpga
        memcpy((void*)addr, data, len);
    #else
        //todo: memcpy via xdma api
    #endif
}

static void vnic_memcpy_from_share_mem(uint64_t addr, void *data, size_t len)
{
    #ifdef fpga
        memcpy(data, (void*)addr, len);
    #else
        //todo: memcpy via xdma api
    #endif
}

static int vnic_init_share_mem(void)
{
    #ifdef fpga
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
                (uint64_t)phys_addr, share_mem_virt);
    #else
        share_mem_virt = (void *)0xfc000000;
        pr_info("fpga reserved memory at fpga physical address: 0x%llx\n",
                (uint64_t)share_mem_virt);
    #endif

    tx_tail = (uint64_t *)(share_mem_virt + TX_TAIL_OFFSET);
    tx_head = (uint64_t *)(share_mem_virt + TX_HEAD_OFFSET);
    tx_data_len = (uint64_t *)(share_mem_virt + TX_DATA_LEN_OFFSET);
    tx_fifo = (void *)(share_mem_virt + TX_FIFO_OFFSET);
    rx_tail = (uint64_t *)(share_mem_virt + RX_TAIL_OFFSET);
    rx_head = (uint64_t *)(share_mem_virt + RX_HEAD_OFFSET);
    rx_data_len = (uint64_t *)(share_mem_virt + RX_DATA_LEN_OFFSET);
    rx_fifo = (void *)(share_mem_virt + RX_FIFO_OFFSET);

    return 0;
}

static void vnic_ring_tx(void)
{
    while (vnic_read_share_mem(tx_tail) != 0);
    vnic_write_share_mem(tx_tail, 1);
}

static void vnic_ring_rx(void)
{
    vnic_write_share_mem(rx_tail, 0);
}

static int vnic_send_packet(struct sk_buff *skb)
{
    pr_info("send packet: %p\n", skb);
    //memcpy(share_mem_virt + FPGA_TX_FIFO_OFFSET, skb->data, skb->len);
    vnic_memcpy_to_share_mem((uint64_t)tx_fifo, skb->data, skb->len);

    vnic_write_share_mem(tx_data_len, skb->len);

    //vnic_ring_tx();
    return 0;
}

static int vnic_recv_packet(void)
{
    if (vnic_read_share_mem(rx_tail) == 0) {
        //pr_info("No new data to receive\n");
        return 0;
    }

    uint64_t len = vnic_read_share_mem(rx_data_len);
    if (len == 0) {
        pr_err("No data to receive\n");
        return -EAGAIN;
    }
    
    struct sk_buff *skb = netdev_alloc_skb(vnic_dev, len + NET_IP_ALIGN);
    if (!skb) {
        pr_err("Failed to allocate skb\n");
        return -ENOMEM;
    }

    // 保留头部空间并复制数据
    skb_reserve(skb, NET_IP_ALIGN);
    skb_put(skb, len); // 扩展skb的tail指针
    //memcpy(skb->data, host_tx_fifo, len);
    vnic_memcpy_from_share_mem((uint64_t)rx_fifo, skb->data, len);
    //vnic_ring_rx();
    netif_rx(skb);  // 将数据包送回协议栈
    return len;
}