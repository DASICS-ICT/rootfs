#include "vnic.h"


/* 网络设备操作函数 */
static netdev_tx_t vnic_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    skb_tx_timestamp(skb);
    
    if (skb_is_nonlinear(skb)) {
        if (skb_linearize(skb) != 0) {
            //dev_kfree_skb(skb);
            pr_info("skb_linearize failed\n");
            return NETDEV_TX_OK;
        }
    }

    vnic_send_packet(skb);
    
    pr_info("vnic_start_xmit: dev = %llx, tx_packets %llx, tx_bytes = %llx\n", 
           (unsigned long long)dev, (unsigned long long)dev->stats.tx_packets,
           (unsigned long long)dev->stats.tx_bytes);
    //if (skb)
    //    dev_kfree_skb(skb);

    return NETDEV_TX_OK;
}

static int vnic_poll_rx(void *data)
{
    int ret;
    while (!kthread_should_stop()) { // 检查线程是否应停止
        ret = vnic_recv_packet();       
        if (ret < 0) {
            pr_err("Failed to receive packet: %d\n", ret);
            continue; // 如果接收失败，继续下一次循环
        }
        else if (ret > 0)
            pr_info("recv packet len %llx\n", ret);

        msleep(5); // 休眠5毫秒降低CPU占用
    }
    return 0;
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
    
    /* random mac */
    //eth_random_addr(dev->dev_addr);
    /* manually set mac address */
    #ifdef fpga
        memcpy(dev->dev_addr, "\x01\x02\x03\x04\x05\x06", ETH_ALEN);
    #else
        memcpy(dev->dev_addr, "\x11\x22\x33\x44\x55\x66", ETH_ALEN);
    #endif

    if (vnic_init_share_mem()) {
        pr_err("Failed to initialize shared memory\n");
        return;
    }
    pr_info("share_mem_virt: 0x%llx\n", (uint64_t)share_mem_virt);
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

    polling_thread = kthread_run(vnic_poll_rx, NULL, "vnic_poll");
    if (IS_ERR(polling_thread)) {
        pr_err("Failed to start polling thread\n");
        return PTR_ERR(polling_thread);
    }
    
    pr_info("Virtual network device %s registered\n", vnic_dev->name);
    return 0;
}

static void __exit vnic_exit(void)
{
    unregister_netdev(vnic_dev);
    free_netdev(vnic_dev);
    if (polling_thread) {
        kthread_stop(polling_thread); // 停止线程
        polling_thread = NULL;
    }
    iounmap(share_mem_virt); // 取消映射
    pr_info("Virtual network device unregistered\n");
}

module_init(vnic_init);
module_exit(vnic_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Guofeng Li");