// SPDX-License-Identifier: GPL-2.0-only
/*
 * Hand-written trusted DASICS glue for Linux 5.10's dummy network driver.
 *
 * KSplit supplies the call/callback boundary and object projections. This
 * file deliberately does not use KSplit IDLC marshal/shadow code: trusted
 * stubs validate raw pointers, grant temporary DASICS regions, and enter the
 * untrusted logic with the ordinary RISC-V C ABI.
 */

#include <linux/atomic.h>
#include <linux/dasics.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/module.h>
#include <linux/net_tstamp.h>
#include <linux/netdevice.h>
#include <linux/notifier.h>
#include <linux/random.h>
#include <linux/rtnetlink.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <net/rtnetlink.h>

#include "../dummy-abi.h"

#define DRV_NAME "dummy"
#define DUMMY_LOGIC_NAME "dummy_logic"
#define DUMMY_MAX_DEVICES 8

enum dummy_callback_kind {
	DUMMY_CALLBACK_NONE,
	DUMMY_CALLBACK_SETUP,
	DUMMY_CALLBACK_VALIDATE,
	DUMMY_CALLBACK_DEV_INIT,
	DUMMY_CALLBACK_DEV_UNINIT,
	DUMMY_CALLBACK_XMIT,
	DUMMY_CALLBACK_SET_RX_MODE,
	DUMMY_CALLBACK_GET_STATS64,
	DUMMY_CALLBACK_CHANGE_CARRIER,
	DUMMY_CALLBACK_GET_DRVINFO,
};

struct dummy_logic_targets {
	void *init_module;
	void *cleanup_module;
	void *setup;
	void *validate;
	void *dev_init;
	void *dev_uninit;
	void *xmit;
	void *set_rx_mode;
	void *get_stats64;
	void *change_carrier;
	void *get_drvinfo;
	unsigned long link_ops;
	unsigned long netdev_ops;
	unsigned long ethtool_ops;
	unsigned long numdummies;
};

struct dummy_callback_context {
	enum dummy_callback_kind kind;
	void *arg0;
	void *arg1;
	void *arg2;
	bool skb_freed;
};

static struct dasics_compartment dummy_compartment;
static struct dasics_call_frame dummy_callback_frame;
static struct dummy_logic_targets dummy_targets;
static struct dummy_callback_context dummy_context;
static atomic_t dummy_callback_busy = ATOMIC_INIT(0);
static struct net_device *dummy_devices[DUMMY_MAX_DEVICES];
static unsigned int dummy_nr_devices;
static int dummy_last_setup_error;
static bool dummy_self_ref;
static bool dummy_pernet_locked;
static bool dummy_rtnl_locked;
static bool dummy_link_registered;

static const struct net_device_ops dummy_glue_netdev_ops;
static const struct ethtool_ops dummy_glue_ethtool_ops;
static struct rtnl_link_ops dummy_glue_link_ops;

static bool dummy_args_zero(const struct dasics_maincall_request *request,
			    unsigned int first)
{
	unsigned int i;

	for (i = first; i < ARRAY_SIZE(request->args); i++) {
		if (request->args[i])
			return false;
	}
	return true;
}

static bool dummy_current_target(void *target)
{
	struct dasics_call_frame *frame = dasics_call_current_frame();

	return frame && frame->policy && frame->policy->target == target;
}

static bool dummy_in_callback(enum dummy_callback_kind kind)
{
	return atomic_read(&dummy_callback_busy) &&
	       READ_ONCE(dummy_context.kind) == kind;
}

static int dummy_add_region(struct dasics_region *regions, unsigned int *nr,
			    void *base, size_t size, unsigned int perms)
{
	if (!base || !size || *nr >= DASICS_POLICY_MAX_REGIONS)
		return -EINVAL;
	regions[*nr].base = (unsigned long)base;
	regions[*nr].size = size;
	regions[*nr].perms = perms;
	(*nr)++;
	return 0;
}

static long dummy_enter_callback(enum dummy_callback_kind kind, void *target,
				 void *arg0, void *arg1, void *arg2,
				 struct dasics_region *regions,
				 unsigned int nr_regions, bool has_result,
				 bool *skb_freed)
{
	struct dasics_call_policy policy = {
		.callee = &dummy_compartment,
		.target = target,
		.regions = regions,
		.nr_regions = nr_regions,
		.stack_size = DASICS_MODULE_STACK_SIZE,
	};
	struct dasics_call_regs regs = {
		.a0 = (unsigned long)arg0,
		.a1 = (unsigned long)arg1,
		.a2 = (unsigned long)arg2,
	};
	enum module_state state;
	long ret;

	if (!READ_ONCE(dummy_compartment.registered) ||
	    atomic_cmpxchg(&dummy_callback_busy, 0, 1))
		return -EBUSY;
	state = READ_ONCE(dummy_compartment.module->state);
	if (state == MODULE_STATE_COMING)
		policy.flags |= DASICS_CALL_ALLOW_COMING;
	else if (state == MODULE_STATE_GOING)
		policy.flags |= DASICS_CALL_ALLOW_GOING;
	else if (state != MODULE_STATE_LIVE) {
		atomic_set(&dummy_callback_busy, 0);
		return -ENODEV;
	}

	WRITE_ONCE(dummy_context.kind, kind);
	dummy_context.arg0 = arg0;
	dummy_context.arg1 = arg1;
	dummy_context.arg2 = arg2;
	dummy_context.skb_freed = false;
	ret = dasics_call(&dummy_callback_frame, &policy, &regs);
	if (skb_freed)
		*skb_freed = dummy_context.skb_freed;
	memset(&dummy_context, 0, sizeof(dummy_context));
	atomic_set(&dummy_callback_busy, 0);
	return ret ?: (has_result ? (long)regs.ret_a0 : 0);
}

static void dummy_setup_stub(struct net_device *dev)
{
	struct dasics_region regions[2];
	unsigned int nr = 0;
	long ret;

	if (!dev ||
	    dummy_add_region(regions, &nr, dev, sizeof(*dev),
			     DASICS_REGION_READ | DASICS_REGION_WRITE))
		return;
	if (dev->dev_addr &&
	    !((unsigned long)dev->dev_addr >= (unsigned long)dev &&
	      (unsigned long)dev->dev_addr + ETH_ALEN <=
	      (unsigned long)dev + sizeof(*dev)))
		dummy_add_region(regions, &nr, dev->dev_addr, ETH_ALEN,
				 DASICS_REGION_READ | DASICS_REGION_WRITE);

	ret = dummy_enter_callback(DUMMY_CALLBACK_SETUP, dummy_targets.setup,
				   dev, NULL, NULL, regions, nr, false, NULL);
	if (!ret && (dev->netdev_ops != &dummy_glue_netdev_ops ||
		     dev->ethtool_ops != &dummy_glue_ethtool_ops))
		ret = -EPERM;
	WRITE_ONCE(dummy_last_setup_error, ret);
}

static int dummy_validate_stub(struct nlattr *tb[], struct nlattr *data[],
			       struct netlink_ext_ack *extack)
{
	struct dasics_region regions[2];
	unsigned int nr = 0;
	struct nlattr *address;
	size_t attr_size;
	int ret;

	if (!tb)
		return -EINVAL;
	ret = dummy_add_region(regions, &nr, tb,
			       (IFLA_MAX + 1) * sizeof(tb[0]),
			       DASICS_REGION_READ);
	if (ret)
		return ret;
	address = tb[IFLA_ADDRESS];
	if (address) {
		attr_size = nla_total_size(nla_len(address));
		if (attr_size < NLA_HDRLEN || attr_size > 256)
			return -EINVAL;
		ret = dummy_add_region(regions, &nr, address, attr_size,
				       DASICS_REGION_READ);
		if (ret)
			return ret;
	}
	return dummy_enter_callback(DUMMY_CALLBACK_VALIDATE,
				    dummy_targets.validate, tb, data, extack,
				    regions, nr, true, NULL);
}

static int dummy_dev_init_stub(struct net_device *dev)
{
	struct dasics_region region;

	if (!dev)
		return -EINVAL;
	region.base = (unsigned long)dev;
	region.size = sizeof(*dev);
	region.perms = DASICS_REGION_READ | DASICS_REGION_WRITE;
	return dummy_enter_callback(DUMMY_CALLBACK_DEV_INIT,
				    dummy_targets.dev_init, dev, NULL, NULL,
				    &region, 1, true, NULL);
}

static void dummy_dev_uninit_stub(struct net_device *dev)
{
	struct dasics_region region;
	long ret;

	if (!dev)
		return;
	region.base = (unsigned long)dev;
	region.size = sizeof(*dev);
	region.perms = DASICS_REGION_READ;
	ret = dummy_enter_callback(DUMMY_CALLBACK_DEV_UNINIT,
				   dummy_targets.dev_uninit, dev, NULL, NULL,
				   &region, 1, false, NULL);
	if (ret)
		pr_err("dummy_glue: ndo_uninit failed: %ld\n", ret);
}

static netdev_tx_t dummy_xmit_stub(struct sk_buff *skb,
				   struct net_device *dev)
{
	struct dasics_region regions[2];
	unsigned int nr = 0;
	bool skb_freed = false;
	long ret;

	if (!skb || !dev ||
	    dummy_add_region(regions, &nr, skb, sizeof(*skb),
			     DASICS_REGION_READ) ||
	    dummy_add_region(regions, &nr, dev, sizeof(*dev),
			     DASICS_REGION_READ))
		return NETDEV_TX_BUSY;
	ret = dummy_enter_callback(DUMMY_CALLBACK_XMIT, dummy_targets.xmit,
				   skb, dev, NULL, regions, nr, true,
				   &skb_freed);
	if (skb_freed)
		return NETDEV_TX_OK;
	return ret ? NETDEV_TX_BUSY : NETDEV_TX_OK;
}

static void dummy_set_rx_mode_stub(struct net_device *dev)
{
	long ret;

	ret = dummy_enter_callback(DUMMY_CALLBACK_SET_RX_MODE,
				   dummy_targets.set_rx_mode, dev, NULL, NULL,
				   NULL, 0, false, NULL);
	if (ret)
		pr_err("dummy_glue: ndo_set_rx_mode failed: %ld\n", ret);
}

static void dummy_get_stats64_stub(struct net_device *dev,
				   struct rtnl_link_stats64 *stats)
{
	struct dasics_region regions[2];
	unsigned int nr = 0;
	long ret;

	if (!dev || !stats ||
	    dummy_add_region(regions, &nr, dev, sizeof(*dev),
			     DASICS_REGION_READ) ||
	    dummy_add_region(regions, &nr, stats, sizeof(*stats),
			     DASICS_REGION_READ | DASICS_REGION_WRITE))
		return;
	ret = dummy_enter_callback(DUMMY_CALLBACK_GET_STATS64,
				   dummy_targets.get_stats64, dev, stats, NULL,
				   regions, nr, false, NULL);
	if (ret)
		pr_err("dummy_glue: ndo_get_stats64 failed: %ld\n", ret);
}

static int dummy_change_carrier_stub(struct net_device *dev, bool carrier)
{
	struct dasics_region region;

	if (!dev)
		return -EINVAL;
	region.base = (unsigned long)dev;
	region.size = sizeof(*dev);
	region.perms = DASICS_REGION_READ;
	return dummy_enter_callback(DUMMY_CALLBACK_CHANGE_CARRIER,
				    dummy_targets.change_carrier, dev,
				    (void *)(unsigned long)carrier, NULL,
				    &region, 1, true, NULL);
}

static void dummy_get_drvinfo_stub(struct net_device *dev,
				   struct ethtool_drvinfo *info)
{
	struct dasics_region region;
	long ret;

	if (!info)
		return;
	region.base = (unsigned long)info;
	region.size = sizeof(*info);
	region.perms = DASICS_REGION_READ | DASICS_REGION_WRITE;
	ret = dummy_enter_callback(DUMMY_CALLBACK_GET_DRVINFO,
				   dummy_targets.get_drvinfo, dev, info, NULL,
				   &region, 1, false, NULL);
	if (ret)
		pr_err("dummy_glue: get_drvinfo failed: %ld\n", ret);
}

static const struct net_device_ops dummy_glue_netdev_ops = {
	.ndo_init		= dummy_dev_init_stub,
	.ndo_uninit		= dummy_dev_uninit_stub,
	.ndo_start_xmit		= dummy_xmit_stub,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_rx_mode	= dummy_set_rx_mode_stub,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_get_stats64	= dummy_get_stats64_stub,
	.ndo_change_carrier	= dummy_change_carrier_stub,
};

static const struct ethtool_ops dummy_glue_ethtool_ops = {
	.get_drvinfo		= dummy_get_drvinfo_stub,
	.get_ts_info		= ethtool_op_get_ts_info,
};

static struct rtnl_link_ops dummy_glue_link_ops = {
	.kind		= DRV_NAME,
	.setup		= dummy_setup_stub,
	.validate	= dummy_validate_stub,
};

static int dummy_track_device(struct net_device *dev)
{
	if (!dev || dummy_nr_devices >= ARRAY_SIZE(dummy_devices))
		return -EDQUOT;
	dummy_devices[dummy_nr_devices++] = dev;
	return 0;
}

static bool dummy_device_tracked(struct net_device *dev)
{
	unsigned int i;

	for (i = 0; i < dummy_nr_devices; i++) {
		if (dummy_devices[i] == dev)
			return true;
	}
	return false;
}

static void dummy_untrack_device(struct net_device *dev)
{
	unsigned int i;

	for (i = 0; i < dummy_nr_devices; i++) {
		if (dummy_devices[i] != dev)
			continue;
		dummy_devices[i] = dummy_devices[--dummy_nr_devices];
		dummy_devices[dummy_nr_devices] = NULL;
		return;
	}
}

static void dummy_clear_devices(void)
{
	memset(dummy_devices, 0, sizeof(dummy_devices));
	dummy_nr_devices = 0;
}

static int dummy_resolve_target(void **target, const char *name)
{
	*target = (void *)dasics_maincall_lookup_caller_symbol(name);
	return *target ? 0 : -ENOENT;
}

static int dummy_prepare_caller(void)
{
	struct dasics_region numdummies_region;
	bool attached = false;
	int ret;

	if (READ_ONCE(dummy_compartment.registered))
		goto grant_state;

	ret = dummy_resolve_target(&dummy_targets.init_module, "init_module");
	ret = ret ?: dummy_resolve_target(&dummy_targets.cleanup_module,
					  "cleanup_module");
	ret = ret ?: dummy_resolve_target(&dummy_targets.setup, "dummy_setup");
	ret = ret ?: dummy_resolve_target(&dummy_targets.validate,
					  "dummy_validate");
	ret = ret ?: dummy_resolve_target(&dummy_targets.dev_init,
					  "dummy_dev_init");
	ret = ret ?: dummy_resolve_target(&dummy_targets.dev_uninit,
					  "dummy_dev_uninit");
	ret = ret ?: dummy_resolve_target(&dummy_targets.xmit, "dummy_xmit");
	ret = ret ?: dummy_resolve_target(&dummy_targets.set_rx_mode,
					  "set_multicast_list");
	ret = ret ?: dummy_resolve_target(&dummy_targets.get_stats64,
					  "dummy_get_stats64");
	ret = ret ?: dummy_resolve_target(&dummy_targets.change_carrier,
					  "dummy_change_carrier");
	ret = ret ?: dummy_resolve_target(&dummy_targets.get_drvinfo,
					  "dummy_get_drvinfo");
	if (ret)
		return ret;
	dummy_targets.link_ops =
		dasics_maincall_lookup_caller_symbol("dummy_link_ops");
	dummy_targets.netdev_ops =
		dasics_maincall_lookup_caller_symbol("dummy_netdev_ops");
	dummy_targets.ethtool_ops =
		dasics_maincall_lookup_caller_symbol("dummy_ethtool_ops");
	dummy_targets.numdummies =
		dasics_maincall_lookup_caller_symbol("numdummies");
	if (!dummy_targets.link_ops || !dummy_targets.netdev_ops ||
	    !dummy_targets.ethtool_ops || !dummy_targets.numdummies)
		return -ENOENT;

	ret = dasics_maincall_attach_caller(&dummy_compartment,
					    dummy_targets.setup);
	if (ret)
		return ret;
	__module_get(THIS_MODULE);
	dummy_self_ref = true;
	attached = true;

grant_state:
	numdummies_region.base = dummy_targets.numdummies;
	numdummies_region.size = sizeof(int);
	numdummies_region.perms = DASICS_REGION_READ;
	ret = dasics_maincall_grant(&numdummies_region);
	if (ret && dummy_self_ref) {
		dasics_compartment_destroy(&dummy_compartment);
		memset(&dummy_targets, 0, sizeof(dummy_targets));
		dummy_self_ref = false;
		module_put(THIS_MODULE);
	}
	if (!ret && attached)
		pr_info("dummy_glue: attached untrusted dummy_logic policy\n");
	return ret;
}

static void dummy_release_caller(void)
{
	bool attached = READ_ONCE(dummy_compartment.registered);

	dasics_compartment_destroy(&dummy_compartment);
	memset(&dummy_targets, 0, sizeof(dummy_targets));
	dummy_clear_devices();
	dummy_pernet_locked = false;
	dummy_rtnl_locked = false;
	dummy_link_registered = false;
	if (dummy_self_ref) {
		dummy_self_ref = false;
		module_put(THIS_MODULE);
	}
	if (attached)
		pr_info("dummy_glue: detached untrusted dummy_logic policy\n");
}

static long dummy_maincall_invoke_impl(
		const struct dasics_maincall_request *request,
		unsigned long *value)
{
	struct net_device *dev;
	struct dasics_region region;
	unsigned long service = request->service_id;
	int ret;

	*value = 0;
	switch (service) {
	case DUMMY_MC_DOWN_WRITE:
		if (!dummy_current_target(dummy_targets.init_module) &&
		    dummy_targets.init_module)
			return -EPERM;
		ret = dummy_prepare_caller();
		if (ret || !dummy_current_target(dummy_targets.init_module) ||
		    !dummy_args_zero(request, 0) || dummy_pernet_locked)
			return ret ?: -EPERM;
		down_write(&pernet_ops_rwsem);
		dummy_pernet_locked = true;
		return 0;
	case DUMMY_MC_RTNL_LOCK:
		if (!dummy_current_target(dummy_targets.init_module) ||
		    !dummy_args_zero(request, 0) || !dummy_pernet_locked ||
		    dummy_rtnl_locked)
			return -EPERM;
		rtnl_lock();
		dummy_rtnl_locked = true;
		return 0;
	case DUMMY_MC_LINK_REGISTER:
		if (!dummy_current_target(dummy_targets.init_module) ||
		    !dummy_pernet_locked || !dummy_rtnl_locked ||
		    dummy_link_registered ||
		    request->args[0] != dummy_targets.link_ops ||
		    request->args[1] != dummy_targets.netdev_ops ||
		    request->args[2] != dummy_targets.ethtool_ops ||
		    !dummy_args_zero(request, 3))
			return -EPERM;
		*value = (unsigned long)(long)
			__rtnl_link_register(&dummy_glue_link_ops);
		if (!(long)*value)
			dummy_link_registered = true;
		return 0;
	case DUMMY_MC_COND_RESCHED:
		if (!dummy_current_target(dummy_targets.init_module) ||
		    !dummy_pernet_locked || !dummy_rtnl_locked ||
		    !dummy_args_zero(request, 0))
			return -EPERM;
		cond_resched();
		return 0;
	case DUMMY_MC_LINK_UNREGISTER:
		if (!dummy_current_target(dummy_targets.init_module) ||
		    !dummy_pernet_locked || !dummy_rtnl_locked ||
		    !dummy_link_registered ||
		    request->args[0] != dummy_targets.link_ops ||
		    !dummy_args_zero(request, 1))
			return -EPERM;
		__rtnl_link_unregister(&dummy_glue_link_ops);
		dummy_link_registered = false;
		dummy_clear_devices();
		return 0;
	case DUMMY_MC_RTNL_UNLOCK:
		if (!dummy_current_target(dummy_targets.init_module) ||
		    !dummy_rtnl_locked || !dummy_args_zero(request, 0))
			return -EPERM;
		rtnl_unlock();
		dummy_rtnl_locked = false;
		return 0;
	case DUMMY_MC_UP_WRITE:
		if (!dummy_current_target(dummy_targets.init_module) ||
		    !dummy_pernet_locked || dummy_rtnl_locked ||
		    !dummy_args_zero(request, 0))
			return -EPERM;
		up_write(&pernet_ops_rwsem);
		dummy_pernet_locked = false;
		return 0;
	case DUMMY_MC_ALLOC_NETDEV:
		if (!dummy_current_target(dummy_targets.init_module) ||
		    !dummy_pernet_locked || !dummy_rtnl_locked ||
		    request->args[0] ||
		    request->args[2] != NET_NAME_ENUM ||
		    request->args[3] != (unsigned long)dummy_targets.setup ||
		    request->args[4] != 1 || request->args[5] != 1 ||
		    request->args[6] ||
		    dasics_maincall_validate_range(request->args[1], 8,
						    DASICS_REGION_READ))
			return -EPERM;
		if (memcmp((void *)request->args[1], "dummy%d", 8))
			return -EINVAL;
		WRITE_ONCE(dummy_last_setup_error, 0);
		dev = alloc_netdev_mqs(0, "dummy%d", NET_NAME_ENUM,
				       dummy_setup_stub, 1, 1);
		if (!dev) {
			*value = 0;
			return 0;
		}
		if (READ_ONCE(dummy_last_setup_error)) {
			free_netdev(dev);
			return dummy_last_setup_error;
		}
		ret = dummy_track_device(dev);
		if (ret) {
			free_netdev(dev);
			return ret;
		}
		region.base = (unsigned long)dev;
		region.size = sizeof(*dev);
		region.perms = DASICS_REGION_READ | DASICS_REGION_WRITE;
		ret = dasics_maincall_grant(&region);
		if (!ret && dev->dev_addr &&
		    !((unsigned long)dev->dev_addr >= region.base &&
		      (unsigned long)dev->dev_addr + ETH_ALEN <=
		      region.base + region.size)) {
			region.base = (unsigned long)dev->dev_addr;
			region.size = ETH_ALEN;
			ret = dasics_maincall_grant(&region);
		}
		if (ret) {
			dummy_untrack_device(dev);
			free_netdev(dev);
			return ret;
		}
		*value = (unsigned long)dev;
		return 0;
	case DUMMY_MC_REGISTER_NETDEVICE:
		dev = (struct net_device *)request->args[0];
		if (!dummy_current_target(dummy_targets.init_module) ||
		    !dummy_pernet_locked || !dummy_rtnl_locked ||
		    !dummy_device_tracked(dev) ||
		    !dummy_args_zero(request, 1) ||
		    dasics_maincall_validate_range((unsigned long)dev,
						    sizeof(*dev),
						    DASICS_REGION_READ |
						    DASICS_REGION_WRITE) ||
		    dev->netdev_ops != &dummy_glue_netdev_ops ||
		    dev->ethtool_ops != &dummy_glue_ethtool_ops ||
		    dev->rtnl_link_ops != &dummy_glue_link_ops)
			return -EPERM;
		ret = register_netdevice(dev);
		*value = (unsigned long)(long)ret;
		if (!ret)
			pr_info("dummy_glue: registered %s through isolated callbacks\n",
				dev->name);
		return 0;
	case DUMMY_MC_FREE_NETDEV:
		dev = (struct net_device *)request->args[0];
		if (!dummy_current_target(dummy_targets.init_module) ||
		    !dummy_device_tracked(dev) ||
		    !dummy_args_zero(request, 1))
			return -EPERM;
		dummy_untrack_device(dev);
		free_netdev(dev);
		return 0;
	case DUMMY_MC_ETHER_SETUP:
		dev = (struct net_device *)request->args[0];
		if (!dummy_in_callback(DUMMY_CALLBACK_SETUP) ||
		    dummy_context.arg0 != dev || !dummy_args_zero(request, 1))
			return -EPERM;
		ether_setup(dev);
		return 0;
	case DUMMY_MC_GET_RANDOM_BYTES:
		if (!dummy_in_callback(DUMMY_CALLBACK_SETUP) ||
		    request->args[0] !=
			    (unsigned long)((struct net_device *)
				    dummy_context.arg0)->dev_addr ||
		    request->args[1] != ETH_ALEN ||
		    !dummy_args_zero(request, 2) ||
		    dasics_maincall_validate_range(request->args[0], ETH_ALEN,
						    DASICS_REGION_WRITE))
			return -EPERM;
		get_random_bytes((void *)request->args[0], ETH_ALEN);
		return 0;
	case DUMMY_MC_ALLOC_PCPU_STATS:
		dev = (struct net_device *)request->args[0];
		if (!dummy_in_callback(DUMMY_CALLBACK_DEV_INIT) ||
		    dummy_context.arg0 != dev || !dummy_args_zero(request, 1))
			return -EPERM;
		*value = (unsigned long)
			netdev_alloc_pcpu_stats(struct pcpu_lstats);
		return 0;
	case DUMMY_MC_FREE_PERCPU:
		dev = dummy_context.arg0;
		if (!dummy_in_callback(DUMMY_CALLBACK_DEV_UNINIT) ||
		    request->args[0] != (unsigned long)dev->lstats ||
		    !dummy_args_zero(request, 1))
			return -EPERM;
		free_percpu((void __percpu *)request->args[0]);
		return 0;
	case DUMMY_MC_DEV_LSTATS_ADD:
		dev = (struct net_device *)request->args[0];
		if (!dummy_in_callback(DUMMY_CALLBACK_XMIT) ||
		    dummy_context.arg1 != dev ||
		    request->args[1] !=
			    ((struct sk_buff *)dummy_context.arg0)->len ||
		    !dummy_args_zero(request, 2))
			return -EPERM;
		dev_lstats_add(dev, request->args[1]);
		return 0;
	case DUMMY_MC_SKB_TIMESTAMP:
		if (!dummy_in_callback(DUMMY_CALLBACK_XMIT) ||
		    dummy_context.arg0 != (void *)request->args[0] ||
		    !dummy_args_zero(request, 1))
			return -EPERM;
		skb_tx_timestamp((struct sk_buff *)request->args[0]);
		return 0;
	case DUMMY_MC_KFREE_SKB:
		if (!dummy_in_callback(DUMMY_CALLBACK_XMIT) ||
		    dummy_context.skb_freed ||
		    dummy_context.arg0 != (void *)request->args[0] ||
		    !dummy_args_zero(request, 1))
			return -EPERM;
		dummy_context.skb_freed = true;
		dev_kfree_skb((struct sk_buff *)request->args[0]);
		return 0;
	case DUMMY_MC_DEV_LSTATS_READ:
		dev = (struct net_device *)request->args[0];
		if (!dummy_in_callback(DUMMY_CALLBACK_GET_STATS64) ||
		    dummy_context.arg0 != dev ||
		    dummy_context.arg1 != (void *)request->args[1] ||
		    !dummy_args_zero(request, 2))
			return -EPERM;
		dev_lstats_read(dev,
			&((struct rtnl_link_stats64 *)request->args[1])->tx_packets,
			&((struct rtnl_link_stats64 *)request->args[1])->tx_bytes);
		return 0;
	case DUMMY_MC_CARRIER_ON:
	case DUMMY_MC_CARRIER_OFF:
		dev = (struct net_device *)request->args[0];
		if (!dummy_in_callback(DUMMY_CALLBACK_CHANGE_CARRIER) ||
		    dummy_context.arg0 != dev || !dummy_args_zero(request, 1) ||
		    (service == DUMMY_MC_CARRIER_ON) !=
			    (bool)(unsigned long)dummy_context.arg1)
			return -EPERM;
		if (service == DUMMY_MC_CARRIER_ON)
			netif_carrier_on(dev);
		else
			netif_carrier_off(dev);
		return 0;
	case DUMMY_MC_STRLCPY_DRIVER:
		if (!dummy_in_callback(DUMMY_CALLBACK_GET_DRVINFO) ||
		    request->args[0] !=
			    (unsigned long)((struct ethtool_drvinfo *)
				    dummy_context.arg1)->driver ||
		    request->args[1] !=
			    sizeof(((struct ethtool_drvinfo *)0)->driver) ||
		    !dummy_args_zero(request, 2) ||
		    dasics_maincall_validate_range(request->args[0],
						    request->args[1],
						    DASICS_REGION_WRITE))
			return -EPERM;
		*value = strlcpy((char *)request->args[0], DRV_NAME,
				 request->args[1]);
		return 0;
	case DUMMY_MC_GET_NETDEV_OPS:
		if (!dummy_in_callback(DUMMY_CALLBACK_SETUP) ||
		    !dummy_args_zero(request, 0))
			return -EPERM;
		*value = (unsigned long)&dummy_glue_netdev_ops;
		return 0;
	case DUMMY_MC_GET_ETHTOOL_OPS:
		if (!dummy_in_callback(DUMMY_CALLBACK_SETUP) ||
		    !dummy_args_zero(request, 0))
			return -EPERM;
		*value = (unsigned long)&dummy_glue_ethtool_ops;
		return 0;
	case DUMMY_MC_GET_LINK_OPS:
		if (!dummy_current_target(dummy_targets.init_module) ||
		    !dummy_args_zero(request, 0))
			return -EPERM;
		*value = (unsigned long)&dummy_glue_link_ops;
		return 0;
	case DUMMY_MC_RTNL_LINK_UNREGISTER:
		if (!dummy_current_target(dummy_targets.cleanup_module) ||
		    request->args[0] != dummy_targets.link_ops ||
		    !dummy_args_zero(request, 1) || !dummy_link_registered)
			return -EPERM;
		rtnl_link_unregister(&dummy_glue_link_ops);
		dummy_link_registered = false;
		dummy_clear_devices();
		dummy_release_caller();
		return 0;
	default:
		return -ENOSYS;
	}
}

static long dummy_maincall_invoke(
		const struct dasics_maincall_request *request,
		unsigned long *value)
{
	long ret;

	ret = dummy_maincall_invoke_impl(request, value);
	if (ret)
		pr_err("dummy_glue: rejected service %#lx: %ld\n",
		       request->service_id, ret);
	return ret;
}

static unsigned int dummy_maincall_service_flags(unsigned long service)
{
	switch (service) {
	case DUMMY_MC_DOWN_WRITE:
	case DUMMY_MC_RTNL_LOCK:
	case DUMMY_MC_LINK_REGISTER:
	case DUMMY_MC_COND_RESCHED:
	case DUMMY_MC_LINK_UNREGISTER:
	case DUMMY_MC_ALLOC_NETDEV:
	case DUMMY_MC_REGISTER_NETDEVICE:
	case DUMMY_MC_FREE_NETDEV:
	case DUMMY_MC_ALLOC_PCPU_STATS:
	case DUMMY_MC_FREE_PERCPU:
	case DUMMY_MC_RTNL_LINK_UNREGISTER:
		return DASICS_MAINCALL_PROVIDER_MAY_SLEEP;
	default:
		return 0;
	}
}

static const struct dasics_maincall_provider dummy_maincall_provider = {
	.module_name = DUMMY_LOGIC_NAME,
	.first_service = DUMMY_MAINCALL_FIRST,
	.last_service = DUMMY_MC_LAST,
	.flags = DASICS_MAINCALL_PROVIDER_MAY_SLEEP,
	.owner = THIS_MODULE,
	.service_flags = dummy_maincall_service_flags,
	.invoke = dummy_maincall_invoke,
};

static int dummy_module_notify(struct notifier_block *notifier,
			       unsigned long action, void *data)
{
	struct module *module = data;

	if (action == MODULE_STATE_GOING &&
	    READ_ONCE(dummy_compartment.registered) &&
	    dummy_compartment.module == module)
		dummy_release_caller();
	return NOTIFY_DONE;
}

static struct notifier_block dummy_module_notifier = {
	.notifier_call = dummy_module_notify,
};

static int __init dummy_glue_init(void)
{
	int ret;

	ret = register_module_notifier(&dummy_module_notifier);
	if (ret)
		return ret;
	ret = dasics_maincall_provider_register(&dummy_maincall_provider);
	if (ret)
		unregister_module_notifier(&dummy_module_notifier);
	else
		pr_info("dummy_glue: trusted maincall provider ready\n");
	return ret;
}

static void __exit dummy_glue_exit(void)
{
	dasics_maincall_provider_unregister(&dummy_maincall_provider);
	unregister_module_notifier(&dummy_module_notifier);
	dummy_release_caller();
	pr_info("dummy_glue: provider removed\n");
}

module_init(dummy_glue_init);
module_exit(dummy_glue_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Trusted hand-written DASICS glue for dummy_logic");
