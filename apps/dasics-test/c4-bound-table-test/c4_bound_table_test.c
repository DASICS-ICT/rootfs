// SPDX-License-Identifier: GPL-2.0-only

#include <linux/dasics.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>

static const struct dasics_code_range c4_code_range = {
	.base = 0x1000,
	.size = 0x100,
};

static const struct dasics_compartment c4_owner = {
	.code_ranges = &c4_code_range,
	.nr_code_ranges = 1,
};

static struct dasics_bound_table c4_table;
static struct dasics_bound_table c4_other_table;

struct c4_clear_context {
	unsigned int calls;
	unsigned int last_slot;
	int ret;
};

static int c4_clear_slot(unsigned int slot, void *data)
{
	struct c4_clear_context *context = data;

	context->calls++;
	context->last_slot = slot;
	return context->ret;
}

static int c4_register(struct dasics_bound_table *table, unsigned long base,
		       size_t size, unsigned int perms, unsigned int flags,
		       dasics_bound_handle_t *handle)
{
	struct dasics_region region = {
		.base = base,
		.size = size,
		.perms = perms,
	};

	return dasics_bound_register(table, &region,
				     DASICS_BOUND_LIFETIME_CALL, flags, handle);
}

static int c4_test_more_than_hardware_slots(void)
{
	dasics_bound_handle_t handles[DASICS_DATA_BOUND_SLOTS + 1];
	unsigned int i;
	int ret;

	ret = dasics_bound_table_init(&c4_table, &c4_owner,
				      DASICS_DATA_BOUND_SLOTS);
	if (ret)
		return ret;
	for (i = 0; i < ARRAY_SIZE(handles); i++) {
		ret = c4_register(&c4_table, 0x10000 + i * 0x10, 8,
				  DASICS_REGION_READ, 0, &handles[i]);
		if (ret || !handles[i])
			return -EINVAL;
	}
	if (c4_table.nr_entries != ARRAY_SIZE(handles))
		return -EINVAL;

	pr_info("C4: PASS 17 software bounds exceed 16 hardware slots\n");
	return 0;
}

static int c4_test_stable_handles_and_eviction(void)
{
	const struct dasics_bound_entry *entry;
	dasics_bound_handle_t handles[3];
	dasics_bound_handle_t victim;
	unsigned int slot;
	unsigned int i;
	int ret;

	ret = dasics_bound_table_init(&c4_table, &c4_owner, 2);
	if (ret)
		return ret;
	for (i = 0; i < ARRAY_SIZE(handles); i++) {
		ret = c4_register(&c4_table, 0x20000 + i * 0x10, 8,
				  DASICS_REGION_READ, 0, &handles[i]);
		if (ret)
			return ret;
	}

	ret = dasics_bound_select_slot(&c4_table, handles[0], &slot, &victim);
	if (ret || slot != 0 || victim)
		return -EINVAL;
	ret = dasics_bound_commit_resident(&c4_table, handles[0], slot);
	if (ret)
		return ret;
	ret = dasics_bound_select_slot(&c4_table, handles[1], &slot, &victim);
	if (ret || slot != 1 || victim)
		return -EINVAL;
	ret = dasics_bound_commit_resident(&c4_table, handles[1], slot);
	if (ret)
		return ret;

	ret = dasics_bound_select_slot(&c4_table, handles[2], &slot, &victim);
	if (ret || slot != 0 || victim != handles[0])
		return -EINVAL;
	ret = dasics_bound_commit_resident(&c4_table, handles[2], slot);
	if (ret)
		return ret;
	ret = dasics_bound_get(&c4_table, handles[0], &entry);
	if (ret || entry->resident_slot != -1)
		return -EINVAL;

	ret = dasics_bound_select_slot(&c4_table, handles[0], &slot, &victim);
	if (ret || slot != 1 || victim != handles[1])
		return -EINVAL;
	ret = dasics_bound_commit_resident(&c4_table, handles[0], slot);
	if (ret)
		return ret;
	ret = dasics_bound_get(&c4_table, handles[0], &entry);
	if (ret || entry->handle != handles[0] || entry->resident_slot != 1)
		return -EINVAL;

	pr_info("C4: PASS stable handles survive round-robin eviction\n");
	return 0;
}

static int c4_test_cross_frame_rejected(void)
{
	dasics_bound_handle_t first;
	dasics_bound_handle_t second;
	const struct dasics_bound_entry *entry;
	int ret;

	ret = dasics_bound_table_init(&c4_table, &c4_owner, 1);
	if (ret)
		return ret;
	ret = dasics_bound_table_init(&c4_other_table, &c4_owner, 1);
	if (ret)
		return ret;
	ret = c4_register(&c4_table, 0x30000, 8, DASICS_REGION_READ, 0,
			  &first);
	if (ret)
		return ret;
	ret = c4_register(&c4_other_table, 0x30000, 8, DASICS_REGION_READ,
			  0, &second);
	if (ret || first == second)
		return -EINVAL;
	if (dasics_bound_get(&c4_other_table, first, &entry) != -ENOENT)
		return -EINVAL;

	pr_info("C4: PASS handles cannot cross bound tables\n");
	return 0;
}

static int c4_test_pinned_exhaustion(void)
{
	dasics_bound_handle_t pinned[2];
	dasics_bound_handle_t other;
	dasics_bound_handle_t victim;
	unsigned int slot;
	unsigned int i;
	int ret;

	ret = dasics_bound_table_init(&c4_table, &c4_owner, 2);
	if (ret)
		return ret;
	for (i = 0; i < ARRAY_SIZE(pinned); i++) {
		ret = c4_register(&c4_table, 0x40000 + i * 0x10, 8,
				  DASICS_REGION_READ, DASICS_BOUND_PINNED,
				  &pinned[i]);
		if (ret)
			return ret;
		ret = dasics_bound_commit_resident(&c4_table, pinned[i], i);
		if (ret)
			return ret;
	}
	ret = c4_register(&c4_table, 0x40100, 8, DASICS_REGION_READ, 0,
			  &other);
	if (ret)
		return ret;
	ret = dasics_bound_select_slot(&c4_table, other, &slot, &victim);
	if (ret != -ENOSPC)
		return -EINVAL;

	pr_info("C4: PASS pinned bounds cannot be selected as victims\n");
	return 0;
}

static int c4_test_free(void)
{
	struct c4_clear_context context = { };
	const struct dasics_bound_entry *entry;
	dasics_bound_handle_t resident;
	dasics_bound_handle_t nonresident;
	int ret;

	ret = dasics_bound_table_init(&c4_table, &c4_owner, 2);
	if (ret)
		return ret;
	ret = c4_register(&c4_table, 0x50000, 8, DASICS_REGION_READ, 0,
			  &resident);
	if (ret)
		return ret;
	ret = c4_register(&c4_table, 0x50010, 8, DASICS_REGION_WRITE, 0,
			  &nonresident);
	if (ret)
		return ret;
	ret = dasics_bound_commit_resident(&c4_table, resident, 0);
	if (ret)
		return ret;

	context.ret = -EIO;
	ret = dasics_bound_free(&c4_table, resident, c4_clear_slot, &context);
	if (ret != -EIO || context.calls != 1 ||
	    dasics_bound_get(&c4_table, resident, &entry))
		return -EINVAL;
	context.ret = 0;
	ret = dasics_bound_free(&c4_table, resident, c4_clear_slot, &context);
	if (ret || context.calls != 2 || context.last_slot != 0 ||
	    c4_table.slot_to_handle[0] ||
	    dasics_bound_get(&c4_table, resident, &entry) != -ENOENT)
		return -EINVAL;
	ret = dasics_bound_free(&c4_table, nonresident, NULL, NULL);
	if (ret || c4_table.nr_entries)
		return -EINVAL;

	pr_info("C4: PASS resident and non-resident free semantics\n");
	return 0;
}

static int c4_test_lookup_and_permissions(void)
{
	dasics_bound_handle_t ro;
	dasics_bound_handle_t rw;
	dasics_bound_handle_t found;
	int ret;

	ret = dasics_bound_table_init(&c4_table, &c4_owner, 2);
	if (ret)
		return ret;
	ret = c4_register(&c4_table, 0x60000, 8, DASICS_REGION_READ, 0, &ro);
	if (ret)
		return ret;
	ret = c4_register(&c4_table, 0x60010, 8,
			  DASICS_REGION_READ | DASICS_REGION_WRITE, 0, &rw);
	if (ret)
		return ret;
	ret = dasics_bound_find(&c4_table, 0x60000, DASICS_REGION_READ,
				&found);
	if (ret || found != ro)
		return -EINVAL;
	if (dasics_bound_find(&c4_table, 0x60000, DASICS_REGION_WRITE,
			      &found) != -ENOENT)
		return -EINVAL;
	ret = dasics_bound_find(&c4_table, 0x60017, DASICS_REGION_WRITE,
				&found);
	if (ret || found != rw)
		return -EINVAL;
	if (dasics_bound_find(&c4_table, 0x60018, DASICS_REGION_READ,
			      &found) != -ENOENT)
		return -EINVAL;

	pr_info("C4: PASS address and permission lookup\n");
	return 0;
}

static int c4_test_validation_and_quota(void)
{
	dasics_bound_handle_t handles[DASICS_POLICY_MAX_REGIONS];
	dasics_bound_handle_t ignored;
	struct dasics_region invalid;
	unsigned int i;
	int ret;

	ret = dasics_bound_table_init(&c4_table, &c4_owner, 1);
	if (ret)
		return ret;
	invalid = (struct dasics_region) {
		.base = ULONG_MAX - 7,
		.size = 8,
		.perms = DASICS_REGION_READ,
	};
	ret = dasics_bound_register(&c4_table, &invalid,
				    DASICS_BOUND_LIFETIME_CALL, 0, &ignored);
	if (ret != -EOVERFLOW)
		return -EINVAL;
	invalid = (struct dasics_region) {
		.base = 0x70000,
		.size = 8,
		.perms = BIT(2),
	};
	ret = dasics_bound_register(&c4_table, &invalid,
				    DASICS_BOUND_LIFETIME_CALL, 0, &ignored);
	if (ret != -EINVAL)
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(handles); i++) {
		ret = c4_register(&c4_table, 0x70000 + i * 0x10, 8,
				  DASICS_REGION_READ, 0, &handles[i]);
		if (ret)
			return ret;
	}
	ret = c4_register(&c4_table, 0x80000, 8, DASICS_REGION_READ, 0,
			  &ignored);
	if (ret != -EDQUOT)
		return -EINVAL;
	if (dasics_jump_policy_validate(DASICS_JUMP_BOUND_SLOTS) ||
	    dasics_jump_policy_validate(DASICS_JUMP_BOUND_SLOTS + 1) != -E2BIG)
		return -EINVAL;

	pr_info("C4: PASS validation, software quota, and jump quota\n");
	return 0;
}

static int c4_test_overlap_rejected(void)
{
	dasics_bound_handle_t first;
	dasics_bound_handle_t second;
	int ret;

	ret = dasics_bound_table_init(&c4_table, &c4_owner, 1);
	if (ret)
		return ret;
	ret = c4_register(&c4_table, 0x90000, 0x20, DASICS_REGION_READ, 0,
			  &first);
	if (ret)
		return ret;
	ret = c4_register(&c4_table, 0x90010, 0x20, DASICS_REGION_WRITE, 0,
			  &second);
	if (ret != -EEXIST)
		return -EINVAL;

	pr_info("C4: PASS overlapping software authorization rejected\n");
	return 0;
}

static int c4_test_corrupt_residency_rejected(void)
{
	struct c4_clear_context context = { };
	dasics_bound_handle_t handle;
	dasics_bound_handle_t victim;
	unsigned int slot;
	int ret;

	ret = dasics_bound_table_init(&c4_table, &c4_owner, 1);
	if (ret)
		return ret;
	ret = c4_register(&c4_table, 0xa0000, 8, DASICS_REGION_READ, 0,
			  &handle);
	if (ret)
		return ret;
	ret = dasics_bound_commit_resident(&c4_table, handle, 0);
	if (ret)
		return ret;
	c4_table.slot_to_handle[0] = DASICS_BOUND_INVALID_HANDLE;
	if (dasics_bound_select_slot(&c4_table, handle, &slot, &victim) !=
	    -EUCLEAN)
		return -EINVAL;
	if (dasics_bound_free(&c4_table, handle, c4_clear_slot, &context) !=
	    -EUCLEAN || context.calls)
		return -EINVAL;
	c4_table.slot_to_handle[0] = handle;
	c4_table.entries[0].resident_slot = -2;
	if (dasics_bound_select_slot(&c4_table, handle, &slot, &victim) !=
	    -EUCLEAN)
		return -EINVAL;

	pr_info("C4: PASS corrupt residency metadata rejected\n");
	return 0;
}

struct c4_test_case {
	const char *name;
	int (*run)(void);
};

static const struct c4_test_case c4_tests[] = {
	{ "more than hardware slots", c4_test_more_than_hardware_slots },
	{ "stable handles and eviction", c4_test_stable_handles_and_eviction },
	{ "cross-table handles", c4_test_cross_frame_rejected },
	{ "pinned exhaustion", c4_test_pinned_exhaustion },
	{ "free", c4_test_free },
	{ "lookup", c4_test_lookup_and_permissions },
	{ "validation and quota", c4_test_validation_and_quota },
	{ "overlap", c4_test_overlap_rejected },
	{ "corrupt residency", c4_test_corrupt_residency_rejected },
};

static int __init c4_bound_table_test_init(void)
{
	unsigned int i;
	int ret;

	for (i = 0; i < ARRAY_SIZE(c4_tests); i++) {
		ret = c4_tests[i].run();
		if (ret) {
			pr_err("C4: FAIL %s: ret=%d\n", c4_tests[i].name, ret);
			return ret;
		}
	}

	pr_info("C4: PASS all software data-bound table tests\n");
	return 0;
}

static void __exit c4_bound_table_test_exit(void)
{
}

module_init(c4_bound_table_test_init);
module_exit(c4_bound_table_test_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DASICS C4 trusted software data-bound table tests");
