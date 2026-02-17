#include "s0_guard_common.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <udasics.h>

static int g_cfg_ids[4] = {-1, -1, -1, -1};
static int g_jcfg_id = -1;
static const char g_probe_text[] = "s0-guard-tests";
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

int s0_guard_runtime_init(const char *case_name) {
    extern char __ULIBTEXT_BEGIN__, __ULIBTEXT_END__;

    reset_cfg_ids();
    memset(g_probe_buf, 0, sizeof(g_probe_buf));
    g_jcfg_id = -1;

    register_udasics(0);
    g_jcfg_id = dasics_jumpcfg_alloc((uint64_t)&__ULIBTEXT_BEGIN__, (uint64_t)&__ULIBTEXT_END__);
    if (g_jcfg_id < 0) {
        printf("[S0-GUARD] INIT FAIL: DASICS jumpcfg alloc failed\n");
        unregister_udasics();
        return -1;
    }
    if (!alloc_runtime_cfg()) {
        printf("[S0-GUARD] INIT FAIL: DASICS libcfg alloc failed\n");
        dasics_jumpcfg_free(g_jcfg_id);
        g_jcfg_id = -1;
        unregister_udasics();
        return -1;
    }

    g_runtime_ready = 1;
    printf("[S0-GUARD] MODE: %s\n", case_name);
    fflush(stdout);
    return 0;
}

void s0_guard_runtime_fini(void) {
    int i;

    for (i = 0; i < 4; ++i) {
        if (g_cfg_ids[i] >= 0) {
            dasics_libcfg_free(g_cfg_ids[i]);
        }
    }
    if (g_jcfg_id >= 0) {
        dasics_jumpcfg_free(g_jcfg_id);
        g_jcfg_id = -1;
    }
    reset_cfg_ids();

    if (g_runtime_ready) {
        unregister_udasics();
        g_runtime_ready = 0;
    }
}

int s0_guard_call_untrusted(void (*fn)(void)) {
    lib_call((void *)fn);
    return 0;
}

int s0_case_viol_write(void) {
    printf("[S0-GUARD] case01_viol_write: step1 call untrusted (expect VIOL fault)\n");
    fflush(stdout);
    s0_guard_call_untrusted(s0_untrusted_viol_write);
    printf("[S0-GUARD] case01_viol_write: FAIL (unexpected return)\n");
    return 1;
}

int s0_case_viol_read(void) {
    printf("[S0-GUARD] case02_viol_read: step1 call untrusted (expect VIOL fault)\n");
    fflush(stdout);
    s0_guard_call_untrusted(s0_untrusted_viol_read);
    printf("[S0-GUARD] case02_viol_read: FAIL (unexpected return)\n");
    return 1;
}

int s0_case_proto_mismatch(void) {
    printf("[S0-GUARD] case03_proto_mismatch: step1 call untrusted (expect PROTO fault)\n");
    fflush(stdout);
    s0_guard_call_untrusted(s0_untrusted_proto_mismatch);
    printf("[S0-GUARD] case03_proto_mismatch: FAIL (unexpected return)\n");
    return 1;
}

int s0_case_legal_save_restore(void) {
    printf("[S0-GUARD] case04_legal_save_restore: step1 call untrusted (expect success)\n");
    fflush(stdout);
    return s0_guard_call_untrusted(s0_untrusted_legal_save_restore);
}
