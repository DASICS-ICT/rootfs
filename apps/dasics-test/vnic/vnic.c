#include "vnic.h"

#define DRV_NAME "vnic"

static struct net_device *vnic_dev;

/* 网络设备操作函数 */
static netdev_tx_t vnic_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    skb_tx_timestamp(skb);
    /* 数据包统计 */
    dev->stats.tx_packets++;
    dev->stats.tx_bytes += skb->len;

    /* 回环数据包 */
    skb->dev = dev;
    skb->pkt_type = PACKET_LOOPBACK;
    skb->ip_summed = CHECKSUM_UNNECESSARY;
    
    netif_rx(skb);  // 将数据包送回协议栈
    pr_info("vnic_start_xmit: dev = %llx, tx_packets %llx, tx_bytes = %llx\n", 
           (unsigned long long)dev, (unsigned long long)dev->stats.tx_packets,
           (unsigned long long)dev->stats.tx_bytes);
    //if (skb)
    //    dev_kfree_skb(skb);

    return NETDEV_TX_OK;
}

static int vnic_open(struct net_device *dev)
{
    pr_info("open vnic\n");
    netif_start_queue(dev);
    return 0;
}

static int vnic_close(struct net_device *dev)
{
    pr_info("close vnic\n");
    netif_stop_queue(dev);
    return 0;
}

/* 网络设备操作结构体 */
static const struct net_device_ops vnic_ops = {
    .ndo_open = vnic_open,
    .ndo_stop = vnic_close,
    .ndo_start_xmit = vnic_start_xmit,
};

/* 初始化网络设备 */
static void vnic_setup(struct net_device *dev)
{
    ether_setup(dev);  // 设置以太网相关参数
    
    dev->netdev_ops = &vnic_ops;
    dev->flags |= IFF_NOARP;  // 禁用ARP
    
    /* 随机MAC地址 */
    //eth_random_addr(dev->dev_addr);
    /* 手动设定mac地址为01:02:03:04:05:06 */
    memcpy(dev->dev_addr, "\x01\x02\x03\x04\x05\x06", ETH_ALEN);
}

static int __init vnic_init(void)
{
    int ret;
    
    /* 分配网络设备结构体 */
    vnic_dev = alloc_netdev(0, "vnic%d", NET_NAME_ENUM, vnic_setup);
    if (!vnic_dev)
        return -ENOMEM;
    
    /* 注册网络设备 */
    ret = register_netdev(vnic_dev);
    if (ret) {
        free_netdev(vnic_dev);
        return ret;
    }
    
    pr_info("Virtual network device %s registered\n", vnic_dev->name);
    return 0;
}

static void __exit vnic_exit(void)
{
    unregister_netdev(vnic_dev);
    free_netdev(vnic_dev);
    pr_info("Virtual network device unregistered\n");
}

module_init(vnic_init);
module_exit(vnic_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Guofeng Li");