// SPDX-License-Identifier: GPL-2.0-only
/*
 * Linux 5.10 dummy driver logic with its KSplit boundary calls lowered by
 * hand to the DASICS maincall ABI. The control flow and device semantics
 * remain those of drivers/net/dummy.c at Linux commit 623f5b2fd70f.
 */

#include <linux/etherdevice.h>
#include <linux/err.h>
#include <linux/ethtool.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/net_tstamp.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <net/rtnetlink.h>

#include "../dummy-abi.h"

#define DRV_NAME "dummy"

extern struct dummy_maincall_result dummy_maincall(
		unsigned long service, unsigned long arg0, unsigned long arg1,
		unsigned long arg2, unsigned long arg3, unsigned long arg4,
		unsigned long arg5, unsigned long arg6);

static int numdummies = 1;

static struct dummy_maincall_result dummy_call(
		unsigned long service, unsigned long arg0, unsigned long arg1,
		unsigned long arg2, unsigned long arg3, unsigned long arg4,
		unsigned long arg5, unsigned long arg6)
{
	return dummy_maincall(service, arg0, arg1, arg2, arg3, arg4,
			      arg5, arg6);
}

static __noreturn void dummy_transport_abort(void)
{
	/*
	 * A transport failure in a void callback must not be reported as a
	 * successful driver return. Jumping outside the compartment gives the
	 * trusted DASICS runtime a terminal, attributable fault.
	 */
	asm volatile("li t0, 0\n\tjr t0" : : : "t0", "memory");
	__builtin_unreachable();
}

static int dummy_call_status(unsigned long service)
{
	struct dummy_maincall_result result;

	result = dummy_call(service, 0, 0, 0, 0, 0, 0, 0);
	return result.status;
}

static int dummy_call_void(unsigned long service, unsigned long arg0,
			   unsigned long arg1)
{
	struct dummy_maincall_result result;

	result = dummy_call(service, arg0, arg1, 0, 0, 0, 0, 0);
	return result.status;
}

static long dummy_call_value(unsigned long service, unsigned long arg0,
			     unsigned long arg1, unsigned long arg2,
			     unsigned long arg3, unsigned long arg4,
			     unsigned long arg5, unsigned long arg6)
{
	struct dummy_maincall_result result;

	result = dummy_call(service, arg0, arg1, arg2, arg3, arg4,
			    arg5, arg6);
	return result.status ? result.status : (long)result.value;
}

static void dummy_require(int status)
{
	if (status)
		dummy_transport_abort();
}

/* fake multicast ability */
static void set_multicast_list(struct net_device *dev)
{
}

static void dummy_get_stats64(struct net_device *dev,
			      struct rtnl_link_stats64 *stats)
{
	dummy_require(dummy_call_void(DUMMY_MC_DEV_LSTATS_READ,
				      (unsigned long)dev,
				      (unsigned long)stats));
}

static netdev_tx_t dummy_xmit(struct sk_buff *skb, struct net_device *dev)
{
	unsigned int len = skb->len;

	dummy_require(dummy_call_void(DUMMY_MC_DEV_LSTATS_ADD,
				      (unsigned long)dev, len));
	dummy_require(dummy_call_void(DUMMY_MC_SKB_TIMESTAMP,
				      (unsigned long)skb, 0));
	dummy_require(dummy_call_void(DUMMY_MC_KFREE_SKB,
				      (unsigned long)skb, 0));
	return NETDEV_TX_OK;
}

static int dummy_dev_init(struct net_device *dev)
{
	long stats;

	stats = dummy_call_value(DUMMY_MC_ALLOC_PCPU_STATS,
				 (unsigned long)dev, 0, 0, 0, 0, 0, 0);
	if (IS_ERR_VALUE((unsigned long)stats))
		return stats;
	dev->lstats = (struct pcpu_lstats __percpu *)stats;
	if (!dev->lstats)
		return -ENOMEM;
	return 0;
}

static void dummy_dev_uninit(struct net_device *dev)
{
	dummy_require(dummy_call_void(DUMMY_MC_FREE_PERCPU,
				      (unsigned long)dev->lstats, 0));
}

static int dummy_change_carrier(struct net_device *dev, bool new_carrier)
{
	int status;

	status = dummy_call_void(new_carrier ? DUMMY_MC_CARRIER_ON :
					      DUMMY_MC_CARRIER_OFF,
				 (unsigned long)dev, 0);
	return status;
}

/*
 * These tables preserve the original callback topology for KSplit evidence.
 * Trusted glue substitutes trusted table addresses before the kernel stores
 * or invokes them.
 */
static const struct net_device_ops dummy_netdev_ops = {
	.ndo_init		= dummy_dev_init,
	.ndo_uninit		= dummy_dev_uninit,
	.ndo_start_xmit		= dummy_xmit,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_rx_mode	= set_multicast_list,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_get_stats64	= dummy_get_stats64,
	.ndo_change_carrier	= dummy_change_carrier,
};

static void dummy_get_drvinfo(struct net_device *dev,
			      struct ethtool_drvinfo *info)
{
	dummy_require(dummy_call_void(DUMMY_MC_STRLCPY_DRIVER,
				      (unsigned long)info->driver,
				      sizeof(info->driver)));
}

static const struct ethtool_ops dummy_ethtool_ops = {
	.get_drvinfo		= dummy_get_drvinfo,
	.get_ts_info		= ethtool_op_get_ts_info,
};

static void dummy_setup(struct net_device *dev)
{
	long ops;

	dummy_require(dummy_call_void(DUMMY_MC_ETHER_SETUP,
				      (unsigned long)dev, 0));

	ops = dummy_call_value(DUMMY_MC_GET_NETDEV_OPS, 0, 0, 0, 0, 0, 0, 0);
	if (IS_ERR_VALUE((unsigned long)ops))
		dummy_transport_abort();
	dev->netdev_ops = (const struct net_device_ops *)ops;
	ops = dummy_call_value(DUMMY_MC_GET_ETHTOOL_OPS, 0, 0, 0, 0, 0, 0, 0);
	if (IS_ERR_VALUE((unsigned long)ops))
		dummy_transport_abort();
	dev->ethtool_ops = (const struct ethtool_ops *)ops;
	dev->needs_free_netdev = true;

	dev->flags |= IFF_NOARP;
	dev->flags &= ~IFF_MULTICAST;
	dev->priv_flags |= IFF_LIVE_ADDR_CHANGE | IFF_NO_QUEUE;
	dev->features |= NETIF_F_SG | NETIF_F_FRAGLIST;
	dev->features |= NETIF_F_ALL_TSO;
	dev->features |= NETIF_F_HW_CSUM | NETIF_F_HIGHDMA | NETIF_F_LLTX;
	dev->features |= NETIF_F_GSO_ENCAP_ALL;
	dev->hw_features |= dev->features;
	dev->hw_enc_features |= dev->features;
	dev->addr_assign_type = NET_ADDR_RANDOM;
	dummy_require(dummy_call_void(DUMMY_MC_GET_RANDOM_BYTES,
				      (unsigned long)dev->dev_addr, ETH_ALEN));
	dev->dev_addr[0] &= 0xfe;
	dev->dev_addr[0] |= 0x02;

	dev->min_mtu = 0;
	dev->max_mtu = 0;
}

static int dummy_validate(struct nlattr *tb[], struct nlattr *data[],
			  struct netlink_ext_ack *extack)
{
	if (tb[IFLA_ADDRESS]) {
		if (nla_len(tb[IFLA_ADDRESS]) != ETH_ALEN)
			return -EINVAL;
		if (!is_valid_ether_addr(nla_data(tb[IFLA_ADDRESS])))
			return -EADDRNOTAVAIL;
	}
	return 0;
}

static struct rtnl_link_ops dummy_link_ops __read_mostly = {
	.kind		= DRV_NAME,
	.setup		= dummy_setup,
	.validate	= dummy_validate,
};

module_param(numdummies, int, 0);
MODULE_PARM_DESC(numdummies, "Number of dummy pseudo devices");

static int __init dummy_init_one(void)
{
	struct net_device *dev_dummy;
	long value;
	int err;

	value = dummy_call_value(DUMMY_MC_ALLOC_NETDEV, 0,
				 (unsigned long)"dummy%d", NET_NAME_ENUM,
				 (unsigned long)dummy_setup, 1, 1, 0);
	if (IS_ERR_VALUE((unsigned long)value))
		return value;
	dev_dummy = (struct net_device *)value;
	if (!dev_dummy)
		return -ENOMEM;

	value = dummy_call_value(DUMMY_MC_GET_LINK_OPS, 0, 0, 0, 0, 0, 0, 0);
	if (IS_ERR_VALUE((unsigned long)value)) {
		err = value;
		goto err;
	}
	dev_dummy->rtnl_link_ops = (struct rtnl_link_ops *)value;
	err = dummy_call_value(DUMMY_MC_REGISTER_NETDEVICE,
			       (unsigned long)dev_dummy, 0, 0, 0, 0, 0, 0);
	if (err < 0)
		goto err;
	return 0;

err:
	dummy_require(dummy_call_void(DUMMY_MC_FREE_NETDEV,
				      (unsigned long)dev_dummy, 0));
	return err;
}

static int __init dummy_init_module(void)
{
	int i;
	int err;

	err = dummy_call_status(DUMMY_MC_DOWN_WRITE);
	if (err)
		return err;
	err = dummy_call_status(DUMMY_MC_RTNL_LOCK);
	if (err)
		goto out_up_write;
	err = dummy_call_value(DUMMY_MC_LINK_REGISTER,
			       (unsigned long)&dummy_link_ops,
			       (unsigned long)&dummy_netdev_ops,
			       (unsigned long)&dummy_ethtool_ops,
			       0, 0, 0, 0);
	if (err < 0)
		goto out;

	for (i = 0; i < numdummies && !err; i++) {
		err = dummy_init_one();
		if (dummy_call_status(DUMMY_MC_COND_RESCHED) && !err)
			err = -EIO;
	}
	if (err < 0)
		dummy_require(dummy_call_void(DUMMY_MC_LINK_UNREGISTER,
					      (unsigned long)&dummy_link_ops,
					      0));

out:
	if (dummy_call_status(DUMMY_MC_RTNL_UNLOCK) && !err)
		err = -EIO;
out_up_write:
	if (dummy_call_status(DUMMY_MC_UP_WRITE) && !err)
		err = -EIO;
	return err;
}

static void __exit dummy_cleanup_module(void)
{
	dummy_require(dummy_call_void(DUMMY_MC_RTNL_LINK_UNREGISTER,
				      (unsigned long)&dummy_link_ops, 0));
}

module_init(dummy_init_module);
module_exit(dummy_cleanup_module);
MODULE_LICENSE("GPL");
MODULE_ALIAS_RTNL_LINK(DRV_NAME);
MODULE_DESCRIPTION("Untrusted DASICS dummy network driver logic");
