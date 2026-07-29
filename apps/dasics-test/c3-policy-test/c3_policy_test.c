// SPDX-License-Identifier: GPL-2.0-only

#include <linux/dasics.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define C3_MAX_CASE_REGIONS 4

struct c3_policy_case {
	const char *name;
	struct dasics_region input[C3_MAX_CASE_REGIONS];
	unsigned int nr_input;
	unsigned int capacity;
	unsigned long target;
	int expected_ret;
	struct dasics_region expected[C3_MAX_CASE_REGIONS];
	unsigned int nr_expected;
};

static const struct dasics_code_range c3_code_ranges[] = {
	{ .base = 0x1000, .size = 0x100 },
	{ .base = 0x2000, .size = 0x80 },
};

static struct dasics_region c3_quota_input[DASICS_POLICY_MAX_REGIONS + 1];
static struct dasics_region c3_quota_normalized[DASICS_POLICY_MAX_REGIONS + 1];

static const struct c3_policy_case c3_policy_cases[] = {
	{
		.name = "empty region list",
		.capacity = C3_MAX_CASE_REGIONS,
		.target = 0x1000,
	},
	{
		.name = "sort disjoint regions",
		.input = {
			{ 0x3000, 0x10, DASICS_REGION_WRITE },
			{ 0x1000, 0x10, DASICS_REGION_READ },
			{ 0x2000, 0x10, DASICS_REGION_READ },
		},
		.nr_input = 3,
		.capacity = C3_MAX_CASE_REGIONS,
		.target = 0x107f,
		.expected = {
			{ 0x1000, 0x10, DASICS_REGION_READ },
			{ 0x2000, 0x10, DASICS_REGION_READ },
			{ 0x3000, 0x10, DASICS_REGION_WRITE },
		},
		.nr_expected = 3,
	},
	{
		.name = "merge adjacent equal permissions",
		.input = {
			{ 0x1010, 0x10, DASICS_REGION_READ },
			{ 0x1000, 0x10, DASICS_REGION_READ },
		},
		.nr_input = 2,
		.capacity = C3_MAX_CASE_REGIONS,
		.target = 0x207f,
		.expected = {
			{ 0x1000, 0x20, DASICS_REGION_READ },
		},
		.nr_expected = 1,
	},
	{
		.name = "merge overlapping equal permissions",
		.input = {
			{ 0x1008, 0x20, DASICS_REGION_READ | DASICS_REGION_WRITE },
			{ 0x1000, 0x10, DASICS_REGION_READ | DASICS_REGION_WRITE },
		},
		.nr_input = 2,
		.capacity = C3_MAX_CASE_REGIONS,
		.target = 0x10ff,
		.expected = {
			{ 0x1000, 0x28,
			  DASICS_REGION_READ | DASICS_REGION_WRITE },
		},
		.nr_expected = 1,
	},
	{
		.name = "keep adjacent different permissions",
		.input = {
			{ 0x1000, 0x10, DASICS_REGION_READ },
			{ 0x1010, 0x10,
			  DASICS_REGION_READ | DASICS_REGION_WRITE },
		},
		.nr_input = 2,
		.capacity = C3_MAX_CASE_REGIONS,
		.target = 0x1001,
		.expected = {
			{ 0x1000, 0x10, DASICS_REGION_READ },
			{ 0x1010, 0x10,
			  DASICS_REGION_READ | DASICS_REGION_WRITE },
		},
		.nr_expected = 2,
	},
	{
		.name = "reject overlapping different permissions",
		.input = {
			{ 0x1000, 0x20, DASICS_REGION_READ },
			{ 0x1010, 0x20,
			  DASICS_REGION_READ | DASICS_REGION_WRITE },
		},
		.nr_input = 2,
		.capacity = C3_MAX_CASE_REGIONS,
		.target = 0x1001,
		.expected_ret = -EINVAL,
	},
	{
		.name = "reject zero size",
		.input = {
			{ 0x1000, 0, DASICS_REGION_READ },
		},
		.nr_input = 1,
		.capacity = C3_MAX_CASE_REGIONS,
		.target = 0x1001,
		.expected_ret = -EINVAL,
	},
	{
		.name = "reject range overflow",
		.input = {
			{ ULONG_MAX - 7, 8, DASICS_REGION_READ },
		},
		.nr_input = 1,
		.capacity = C3_MAX_CASE_REGIONS,
		.target = 0x1001,
		.expected_ret = -EOVERFLOW,
	},
	{
		.name = "reject empty permissions",
		.input = {
			{ 0x1000, 8, 0 },
		},
		.nr_input = 1,
		.capacity = C3_MAX_CASE_REGIONS,
		.target = 0x1001,
		.expected_ret = -EINVAL,
	},
	{
		.name = "reject unknown permission bit",
		.input = {
			{ 0x1000, 8, BIT(2) },
		},
		.nr_input = 1,
		.capacity = C3_MAX_CASE_REGIONS,
		.target = 0x1001,
		.expected_ret = -EINVAL,
	},
	{
		.name = "reject target at exclusive end",
		.capacity = C3_MAX_CASE_REGIONS,
		.target = 0x1100,
		.expected_ret = -EPERM,
	},
	{
		.name = "reject target outside ranges",
		.capacity = C3_MAX_CASE_REGIONS,
		.target = 0x1800,
		.expected_ret = -EPERM,
	},
	{
		.name = "reject undersized output",
		.input = {
			{ 0x1000, 8, DASICS_REGION_READ },
			{ 0x2000, 8, DASICS_REGION_READ },
		},
		.nr_input = 2,
		.capacity = 1,
		.target = 0x1001,
		.expected_ret = -ENOSPC,
	},
};

static bool c3_regions_equal(const struct dasics_region *left,
			     const struct dasics_region *right)
{
	return left->base == right->base && left->size == right->size &&
	       left->perms == right->perms;
}

static int c3_run_policy_case(const struct c3_policy_case *test_case)
{
	struct dasics_region normalized[C3_MAX_CASE_REGIONS];
	struct dasics_compartment callee = {
		.code_ranges = c3_code_ranges,
		.nr_code_ranges = ARRAY_SIZE(c3_code_ranges),
	};
	struct dasics_call_policy policy = {
		.callee = &callee,
		.target = (void *)test_case->target,
		.regions = test_case->input,
		.nr_regions = test_case->nr_input,
	};
	unsigned int nr_normalized = UINT_MAX;
	unsigned int i;
	int ret;

	ret = dasics_policy_normalize(&policy, normalized, test_case->capacity,
				      &nr_normalized);
	if (ret != test_case->expected_ret) {
		pr_err("C3: FAIL %s: ret=%d expected=%d\n", test_case->name,
		       ret, test_case->expected_ret);
		return -EINVAL;
	}
	if (ret) {
		pr_info("C3: PASS %s (rejected with %d)\n", test_case->name,
			ret);
		return 0;
	}
	if (nr_normalized != test_case->nr_expected) {
		pr_err("C3: FAIL %s: count=%u expected=%u\n", test_case->name,
		       nr_normalized, test_case->nr_expected);
		return -EINVAL;
	}
	for (i = 0; i < nr_normalized; i++) {
		if (!c3_regions_equal(&normalized[i], &test_case->expected[i])) {
			pr_err("C3: FAIL %s: region %u mismatch\n",
			       test_case->name, i);
			return -EINVAL;
		}
	}

	pr_info("C3: PASS %s\n", test_case->name);
	return 0;
}

static int c3_run_quota_case(unsigned int nr_regions, bool merge_to_one,
			     int expected_ret, unsigned int expected_count,
			     const char *name)
{
	struct dasics_code_range code_range = { .base = 0x1000, .size = 0x100 };
	struct dasics_compartment callee = {
		.code_ranges = &code_range,
		.nr_code_ranges = 1,
	};
	struct dasics_call_policy policy = {
		.callee = &callee,
		.target = (void *)0x1000UL,
		.regions = c3_quota_input,
		.nr_regions = nr_regions,
	};
	unsigned int nr_normalized = 0;
	unsigned int i;
	int ret;

	for (i = 0; i < nr_regions; i++) {
		c3_quota_input[i].base = 0x10000 +
					   i * (merge_to_one ? 8 : 16);
		c3_quota_input[i].size = 8;
		c3_quota_input[i].perms = DASICS_REGION_READ;
	}

	ret = dasics_policy_normalize(&policy, c3_quota_normalized,
				      ARRAY_SIZE(c3_quota_normalized),
				      &nr_normalized);
	if (ret != expected_ret || (!ret && nr_normalized != expected_count)) {
		pr_err("C3: FAIL %s: ret=%d count=%u expected_ret=%d expected_count=%u\n",
		       name, ret, nr_normalized, expected_ret, expected_count);
		return -EINVAL;
	}

	pr_info("C3: PASS %s\n", name);
	return 0;
}

static int c3_run_invalid_registry_cases(void)
{
	struct dasics_code_range invalid_ranges[] = {
		{ .base = 0x1000, .size = 0 },
		{ .base = ULONG_MAX - 7, .size = 8 },
	};
	struct dasics_compartment callee = { };
	struct dasics_call_policy policy = {
		.callee = &callee,
		.target = (void *)0x1000UL,
	};
	struct dasics_region normalized;
	unsigned int count;
	int expected[] = { -EINVAL, -EOVERFLOW };
	unsigned int i;
	int ret;

	for (i = 0; i < ARRAY_SIZE(invalid_ranges); i++) {
		callee.code_ranges = &invalid_ranges[i];
		callee.nr_code_ranges = 1;
		ret = dasics_policy_normalize(&policy, &normalized, 1, &count);
		if (ret != expected[i]) {
			pr_err("C3: FAIL invalid code range %u: ret=%d expected=%d\n",
			       i, ret, expected[i]);
			return -EINVAL;
		}
	}

	pr_info("C3: PASS invalid code ranges fail closed\n");
	return 0;
}

static int c3_run_invalid_api_cases(void)
{
	struct dasics_code_range code_range = { .base = 0x1000, .size = 0x100 };
	struct dasics_compartment callee = {
		.code_ranges = &code_range,
		.nr_code_ranges = 1,
	};
	struct dasics_region input = {
		.base = 0x2000,
		.size = 8,
		.perms = DASICS_REGION_READ,
	};
	struct dasics_call_policy policy = {
		.callee = &callee,
		.target = (void *)0x1000UL,
		.regions = &input,
		.nr_regions = 1,
	};
	struct dasics_region normalized;
	unsigned int count;
	int ret;

	ret = dasics_policy_normalize(NULL, &normalized, 1, &count);
	if (ret != -EINVAL)
		goto fail;
	ret = dasics_policy_normalize(&policy, &normalized, 1, NULL);
	if (ret != -EINVAL)
		goto fail;
	policy.callee = NULL;
	ret = dasics_policy_normalize(&policy, &normalized, 1, &count);
	if (ret != -EINVAL)
		goto fail;
	policy.callee = &callee;
	policy.target = NULL;
	ret = dasics_policy_normalize(&policy, &normalized, 1, &count);
	if (ret != -EINVAL)
		goto fail;
	policy.target = (void *)0x1000UL;
	ret = dasics_policy_normalize(&policy, NULL, 1, &count);
	if (ret != -EINVAL)
		goto fail;
	policy.regions = NULL;
	ret = dasics_policy_normalize(&policy, &normalized, 1, &count);
	if (ret != -EINVAL)
		goto fail;

	pr_info("C3: PASS invalid API arguments fail closed\n");
	return 0;

fail:
	pr_err("C3: FAIL invalid API arguments: ret=%d\n", ret);
	return -EINVAL;
}

static int __init c3_policy_test_init(void)
{
	unsigned int i;
	int ret;

	for (i = 0; i < ARRAY_SIZE(c3_policy_cases); i++) {
		ret = c3_run_policy_case(&c3_policy_cases[i]);
		if (ret)
			return ret;
	}
	ret = c3_run_quota_case(17, false, 0, 17,
				"17 regions exceed hardware slots but meet policy quota");
	if (ret)
		return ret;
	ret = c3_run_quota_case(DASICS_POLICY_MAX_REGIONS + 1, false,
				-E2BIG, 0, "policy quota exhaustion");
	if (ret)
		return ret;
	ret = c3_run_quota_case(DASICS_POLICY_MAX_REGIONS + 1, true, 0, 1,
				"quota checked after normalization");
	if (ret)
		return ret;
	ret = c3_run_invalid_registry_cases();
	if (ret)
		return ret;
	ret = c3_run_invalid_api_cases();
	if (ret)
		return ret;

	pr_info("C3: PASS all policy validation and normalization tests\n");
	return 0;
}

static void __exit c3_policy_test_exit(void)
{
}

module_init(c3_policy_test_init);
module_exit(c3_policy_test_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DASICS C3 policy validation and normalization tests");
