#include <linux/etherdevice.h>
#include <linux/netdevice.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/export.h>

static void vnic_setup(struct net_device *dev);
static int vnic_open(struct net_device *dev);
static int vnic_close(struct net_device *dev);
static netdev_tx_t vnic_start_xmit(struct sk_buff *skb, struct net_device *dev);