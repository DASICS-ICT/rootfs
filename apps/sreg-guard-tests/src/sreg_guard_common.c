#include "sreg_guard_common.h"

#include <stdio.h>
#include <string.h>

#include <udasics.h>

static int g_cfg_ids[4] = {-1, -1, -1, -1};
static const char g_probe_text[] = "sreg-guard-tests";
static uint8_t g_probe_buf[256];
static int g_runtime_ready = 0;

static void reset_cfg_ids(void) {
    int i;
    for (i = 0; i < 4; ++i) {
        g_cfg_ids[i] = -1;
    }
}

static int alloc_runtime_cfg(void) {
    register uint64_t sp asm("sp");
    int ok = 1;

    g_cfg_ids[0] = (int)LIBCFG_ALLOC(
        DASICS_LIBCFG_R | DASICS_LIBCFG_V,
        (void *)g_probe_text,
        sizeof(g_probe_text));
    g_cfg_ids[1] = (int)LIBCFG_ALLOC(
        DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        g_probe_buf,
        sizeof(g_probe_buf));
    g_cfg_ids[2] = (int)LIBCFG_ALLOC(
        DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        (void *)(uintptr_t)(sp - 0x2000),
        0x2000);
    g_cfg_ids[3] = (int)LIBCFG_ALLOC(
        DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        (void *)(uintptr_t)g_cfg_ids,
        sizeof(g_cfg_ids));

    if (g_cfg_ids[0] < 0 || g_cfg_ids[1] < 0 || g_cfg_ids[2] < 0 || g_cfg_ids[3] < 0) {
        ok = 0;
    }
    return ok;
}

int sreg_guard_runtime_init(const char *mode_name) {
    reset_cfg_ids();
    memset(g_probe_buf, 0, sizeof(g_probe_buf));

    register_udasics(0);
    if (!alloc_runtime_cfg()) {
        printf("[INIT] FAIL: DASICS libcfg alloc failed\n");
        return -1;
    }

    g_runtime_ready = 1;
    printf("[MODE] %s\n", mode_name);
    return 0;
}

void sreg_guard_runtime_fini(void) {
    int i;

    for (i = 0; i < 4; ++i) {
        if (g_cfg_ids[i] >= 0) {
            dasics_libcfg_free(g_cfg_ids[i]);
        }
    }
    reset_cfg_ids();

    if (g_runtime_ready) {
        unregister_udasics();
        g_runtime_ready = 0;
    }
}

void sreg_set_pattern(uint64_t seed) {
    sreg_set_pattern_asm(seed);
}

void sreg_take_snapshot(uint64_t out[SREG_COUNT]) {
    sreg_snapshot_asm(out);
}

void sreg_restore_snapshot(const uint64_t in[SREG_COUNT]) {
    sreg_restore_asm(in);
}

uint64_t sreg_simple_tag(const uint64_t regs[SREG_COUNT]) {
    uint64_t tag = 0x9e3779b97f4a7c15ULL;
    int i;
    for (i = 0; i < SREG_COUNT; ++i) {
        tag ^= (regs[i] + (uint64_t)(i + 1) * 0x100000001b3ULL);
    }
    return tag;
}

static int count_mismatch(
    const uint64_t expected[SREG_COUNT],
    const uint64_t actual[SREG_COUNT]) {
    int mismatch = 0;
    int i;
    for (i = 0; i < SREG_COUNT; ++i) {
        if (expected[i] != actual[i]) {
            ++mismatch;
        }
    }
    return mismatch;
}

int sreg_expect_match(
    const char *scene,
    const uint64_t expected[SREG_COUNT],
    const uint64_t actual[SREG_COUNT]) {
    int mismatch = count_mismatch(expected, actual);
    if (mismatch == 0) {
        printf("[CHECK] %s: PASS (s-reg snapshot matched)\n", scene);
        return 0;
    }

    printf("[CHECK] %s: FAIL (%d mismatch)\n", scene, mismatch);
    return 1;
}

int sreg_expect_mismatch(
    const char *scene,
    const uint64_t expected[SREG_COUNT],
    const uint64_t actual[SREG_COUNT]) {
    int mismatch = count_mismatch(expected, actual);
    if (mismatch > 0) {
        printf("[CHECK] %s: PASS (%d mismatch observed as expected)\n", scene, mismatch);
        return 0;
    }

    printf("[CHECK] %s: FAIL (no mismatch observed)\n", scene);
    return 1;
}

int sreg_call_untrusted_overwrite(void) {
    lib_call((void *)sreg_untrusted_overwrite);
    return 0;
}

int sreg_call_untrusted_save_then_use(void) {
    lib_call((void *)sreg_untrusted_save_then_use);
    return 0;
}

int sreg_run_legal_save_then_use_scene(void) {
    uint64_t before[SREG_COUNT];
    uint64_t after[SREG_COUNT];

    sreg_set_pattern(0x1000);
    sreg_take_snapshot(before);
    sreg_call_untrusted_save_then_use();
    sreg_take_snapshot(after);

    return sreg_expect_match("legal_save_then_use", before, after);
}
