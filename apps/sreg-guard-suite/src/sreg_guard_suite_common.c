#include "sreg_guard_suite_common.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <udasics.h>

typedef void (*sreg_untrusted_fn_t)(void);

static int g_cfg_ids[4] = {-1, -1, -1, -1};
static int g_jcfg_id = -1;
static const char g_probe_text[] = "sreg-guard-suite";
static uint8_t g_probe_buf[256];
static int g_runtime_ready = 0;

static const char *const g_sreg_names[] = {
    "s0", "s1", "s2", "s3", "s4", "s5",
    "s6", "s7", "s8", "s9", "s10", "s11",
};

#define SREG_GUARD_REG_COUNT ((int)(sizeof(g_sreg_names) / sizeof(g_sreg_names[0])))

#define DECLARE_SREG_CASES(reg_name) \
    void sreg_untrusted_viol_write_##reg_name(void); \
    void sreg_untrusted_viol_read_##reg_name(void); \
    void sreg_untrusted_proto_mismatch_##reg_name(void); \
    void sreg_untrusted_legal_save_restore_##reg_name(void); \
    void sreg_untrusted_post_save_cipher_read_##reg_name(void); \
    void sreg_untrusted_missing_restore_return_##reg_name(void); \
    void sreg_untrusted_auth_tamper_bitflip_##reg_name(void); \
    void sreg_untrusted_auth_tamper_overwrite_##reg_name(void)

DECLARE_SREG_CASES(s0);
DECLARE_SREG_CASES(s1);
DECLARE_SREG_CASES(s2);
DECLARE_SREG_CASES(s3);
DECLARE_SREG_CASES(s4);
DECLARE_SREG_CASES(s5);
DECLARE_SREG_CASES(s6);
DECLARE_SREG_CASES(s7);
DECLARE_SREG_CASES(s8);
DECLARE_SREG_CASES(s9);
DECLARE_SREG_CASES(s10);
DECLARE_SREG_CASES(s11);

static sreg_untrusted_fn_t const g_viol_write_fns[SREG_GUARD_REG_COUNT] = {
    sreg_untrusted_viol_write_s0, sreg_untrusted_viol_write_s1,
    sreg_untrusted_viol_write_s2, sreg_untrusted_viol_write_s3,
    sreg_untrusted_viol_write_s4, sreg_untrusted_viol_write_s5,
    sreg_untrusted_viol_write_s6, sreg_untrusted_viol_write_s7,
    sreg_untrusted_viol_write_s8, sreg_untrusted_viol_write_s9,
    sreg_untrusted_viol_write_s10, sreg_untrusted_viol_write_s11,
};

static sreg_untrusted_fn_t const g_viol_read_fns[SREG_GUARD_REG_COUNT] = {
    sreg_untrusted_viol_read_s0, sreg_untrusted_viol_read_s1,
    sreg_untrusted_viol_read_s2, sreg_untrusted_viol_read_s3,
    sreg_untrusted_viol_read_s4, sreg_untrusted_viol_read_s5,
    sreg_untrusted_viol_read_s6, sreg_untrusted_viol_read_s7,
    sreg_untrusted_viol_read_s8, sreg_untrusted_viol_read_s9,
    sreg_untrusted_viol_read_s10, sreg_untrusted_viol_read_s11,
};

static sreg_untrusted_fn_t const g_proto_mismatch_fns[SREG_GUARD_REG_COUNT] = {
    sreg_untrusted_proto_mismatch_s0, sreg_untrusted_proto_mismatch_s1,
    sreg_untrusted_proto_mismatch_s2, sreg_untrusted_proto_mismatch_s3,
    sreg_untrusted_proto_mismatch_s4, sreg_untrusted_proto_mismatch_s5,
    sreg_untrusted_proto_mismatch_s6, sreg_untrusted_proto_mismatch_s7,
    sreg_untrusted_proto_mismatch_s8, sreg_untrusted_proto_mismatch_s9,
    sreg_untrusted_proto_mismatch_s10, sreg_untrusted_proto_mismatch_s11,
};

static sreg_untrusted_fn_t const g_legal_save_restore_fns[SREG_GUARD_REG_COUNT] = {
    sreg_untrusted_legal_save_restore_s0, sreg_untrusted_legal_save_restore_s1,
    sreg_untrusted_legal_save_restore_s2, sreg_untrusted_legal_save_restore_s3,
    sreg_untrusted_legal_save_restore_s4, sreg_untrusted_legal_save_restore_s5,
    sreg_untrusted_legal_save_restore_s6, sreg_untrusted_legal_save_restore_s7,
    sreg_untrusted_legal_save_restore_s8, sreg_untrusted_legal_save_restore_s9,
    sreg_untrusted_legal_save_restore_s10, sreg_untrusted_legal_save_restore_s11,
};

static sreg_untrusted_fn_t const g_post_save_cipher_read_fns[SREG_GUARD_REG_COUNT] = {
    sreg_untrusted_post_save_cipher_read_s0, sreg_untrusted_post_save_cipher_read_s1,
    sreg_untrusted_post_save_cipher_read_s2, sreg_untrusted_post_save_cipher_read_s3,
    sreg_untrusted_post_save_cipher_read_s4, sreg_untrusted_post_save_cipher_read_s5,
    sreg_untrusted_post_save_cipher_read_s6, sreg_untrusted_post_save_cipher_read_s7,
    sreg_untrusted_post_save_cipher_read_s8, sreg_untrusted_post_save_cipher_read_s9,
    sreg_untrusted_post_save_cipher_read_s10, sreg_untrusted_post_save_cipher_read_s11,
};

static sreg_untrusted_fn_t const g_missing_restore_return_fns[SREG_GUARD_REG_COUNT] = {
    sreg_untrusted_missing_restore_return_s0, sreg_untrusted_missing_restore_return_s1,
    sreg_untrusted_missing_restore_return_s2, sreg_untrusted_missing_restore_return_s3,
    sreg_untrusted_missing_restore_return_s4, sreg_untrusted_missing_restore_return_s5,
    sreg_untrusted_missing_restore_return_s6, sreg_untrusted_missing_restore_return_s7,
    sreg_untrusted_missing_restore_return_s8, sreg_untrusted_missing_restore_return_s9,
    sreg_untrusted_missing_restore_return_s10, sreg_untrusted_missing_restore_return_s11,
};

static sreg_untrusted_fn_t const g_auth_tamper_bitflip_fns[SREG_GUARD_REG_COUNT] = {
    sreg_untrusted_auth_tamper_bitflip_s0, sreg_untrusted_auth_tamper_bitflip_s1,
    sreg_untrusted_auth_tamper_bitflip_s2, sreg_untrusted_auth_tamper_bitflip_s3,
    sreg_untrusted_auth_tamper_bitflip_s4, sreg_untrusted_auth_tamper_bitflip_s5,
    sreg_untrusted_auth_tamper_bitflip_s6, sreg_untrusted_auth_tamper_bitflip_s7,
    sreg_untrusted_auth_tamper_bitflip_s8, sreg_untrusted_auth_tamper_bitflip_s9,
    sreg_untrusted_auth_tamper_bitflip_s10, sreg_untrusted_auth_tamper_bitflip_s11,
};

static sreg_untrusted_fn_t const g_auth_tamper_overwrite_fns[SREG_GUARD_REG_COUNT] = {
    sreg_untrusted_auth_tamper_overwrite_s0, sreg_untrusted_auth_tamper_overwrite_s1,
    sreg_untrusted_auth_tamper_overwrite_s2, sreg_untrusted_auth_tamper_overwrite_s3,
    sreg_untrusted_auth_tamper_overwrite_s4, sreg_untrusted_auth_tamper_overwrite_s5,
    sreg_untrusted_auth_tamper_overwrite_s6, sreg_untrusted_auth_tamper_overwrite_s7,
    sreg_untrusted_auth_tamper_overwrite_s8, sreg_untrusted_auth_tamper_overwrite_s9,
    sreg_untrusted_auth_tamper_overwrite_s10, sreg_untrusted_auth_tamper_overwrite_s11,
};

static void reset_cfg_ids(void)
{
    int i;
    for (i = 0; i < 4; ++i) {
        g_cfg_ids[i] = -1;
    }
}

static int alloc_runtime_cfg(void)
{
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

    if (g_cfg_ids[0] < 0 || g_cfg_ids[1] < 0 ||
        g_cfg_ids[2] < 0 || g_cfg_ids[3] < 0) {
        ok = 0;
    }
    return ok;
}

static sreg_untrusted_fn_t sreg_lookup_fn(sreg_untrusted_fn_t const *table, int reg_index)
{
    if (reg_index < 0 || reg_index >= SREG_GUARD_REG_COUNT) {
        return NULL;
    }
    return table[reg_index];
}

int sreg_guard_parse_reg(const char *reg_name)
{
    int i;

    if (reg_name == NULL) {
        return -1;
    }
    for (i = 0; i < SREG_GUARD_REG_COUNT; ++i) {
        if (strcmp(reg_name, g_sreg_names[i]) == 0) {
            return i;
        }
    }
    return -1;
}

const char *sreg_guard_reg_name(int reg_index)
{
    if (reg_index < 0 || reg_index >= SREG_GUARD_REG_COUNT) {
        return "invalid";
    }
    return g_sreg_names[reg_index];
}

int sreg_guard_runtime_init(const char *reg_name, const char *case_name)
{
    extern char __ULIBTEXT_BEGIN__, __ULIBTEXT_END__;

    reset_cfg_ids();
    memset(g_probe_buf, 0, sizeof(g_probe_buf));
    g_jcfg_id = -1;

    register_udasics(0);
    g_jcfg_id = dasics_jumpcfg_alloc((uint64_t)&__ULIBTEXT_BEGIN__, (uint64_t)&__ULIBTEXT_END__);
    if (g_jcfg_id < 0) {
        printf("[SREG-GUARD] INIT FAIL: DASICS jumpcfg alloc failed\n");
        unregister_udasics();
        return -1;
    }
    if (!alloc_runtime_cfg()) {
        printf("[SREG-GUARD] INIT FAIL: DASICS libcfg alloc failed\n");
        dasics_jumpcfg_free(g_jcfg_id);
        g_jcfg_id = -1;
        unregister_udasics();
        return -1;
    }

    g_runtime_ready = 1;
    printf("[SREG-GUARD] MODE: reg=%s case=%s\n", reg_name, case_name);
    fflush(stdout);
    return 0;
}

void sreg_guard_runtime_fini(void)
{
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

int sreg_guard_call_untrusted(void (*fn)(void))
{
    if (fn == NULL) {
        return 2;
    }
    lib_call((void *)fn);
    return 0;
}

int sreg_case_viol_write(int reg_index)
{
    const char *reg_name = sreg_guard_reg_name(reg_index);

    printf("[SREG-GUARD] reg=%s case01_viol_write: step1 call untrusted (expect VIOL fault)\n",
           reg_name);
    fflush(stdout);
    sreg_guard_call_untrusted(sreg_lookup_fn(g_viol_write_fns, reg_index));
    printf("[SREG-GUARD] reg=%s case01_viol_write: FAIL (unexpected return)\n", reg_name);
    return 1;
}

int sreg_case_viol_read(int reg_index)
{
    const char *reg_name = sreg_guard_reg_name(reg_index);

    printf("[SREG-GUARD] reg=%s case02_viol_read: step1 call untrusted (expect VIOL fault)\n",
           reg_name);
    fflush(stdout);
    sreg_guard_call_untrusted(sreg_lookup_fn(g_viol_read_fns, reg_index));
    printf("[SREG-GUARD] reg=%s case02_viol_read: FAIL (unexpected return)\n", reg_name);
    return 1;
}

int sreg_case_proto_mismatch(int reg_index)
{
    const char *reg_name = sreg_guard_reg_name(reg_index);

    printf("[SREG-GUARD] reg=%s case03_proto_mismatch: step1 call untrusted (expect PROTO fault)\n",
           reg_name);
    fflush(stdout);
    sreg_guard_call_untrusted(sreg_lookup_fn(g_proto_mismatch_fns, reg_index));
    printf("[SREG-GUARD] reg=%s case03_proto_mismatch: FAIL (unexpected return)\n", reg_name);
    return 1;
}

int sreg_case_legal_save_restore(int reg_index)
{
    const char *reg_name = sreg_guard_reg_name(reg_index);

    printf("[SREG-GUARD] reg=%s case04_legal_save_restore: step1 call untrusted (expect success)\n",
           reg_name);
    fflush(stdout);
    return sreg_guard_call_untrusted(sreg_lookup_fn(g_legal_save_restore_fns, reg_index));
}

int sreg_case_post_save_cipher_read(int reg_index)
{
    const char *reg_name = sreg_guard_reg_name(reg_index);

    printf("[SREG-GUARD] reg=%s case05_post_save_cipher_read: step1 call untrusted (expect success with sreg=cipher after save)\n",
           reg_name);
    fflush(stdout);
    return sreg_guard_call_untrusted(sreg_lookup_fn(g_post_save_cipher_read_fns, reg_index));
}

int sreg_case_missing_restore_return(int reg_index)
{
    const char *reg_name = sreg_guard_reg_name(reg_index);

    printf("[SREG-GUARD] reg=%s case06_missing_restore_return: step1 call untrusted (expect PROTO fault at return gate)\n",
           reg_name);
    fflush(stdout);
    sreg_guard_call_untrusted(sreg_lookup_fn(g_missing_restore_return_fns, reg_index));
    printf("[SREG-GUARD] reg=%s case06_missing_restore_return: FAIL (unexpected return)\n", reg_name);
    return 1;
}

int sreg_case_auth_tamper_bitflip(int reg_index)
{
    const char *reg_name = sreg_guard_reg_name(reg_index);

    printf("[SREG-GUARD] reg=%s case07_auth_tamper_bitflip: step1 call untrusted (expect AUTH fault)\n",
           reg_name);
    fflush(stdout);
    sreg_guard_call_untrusted(sreg_lookup_fn(g_auth_tamper_bitflip_fns, reg_index));
    printf("[SREG-GUARD] reg=%s case07_auth_tamper_bitflip: FAIL (unexpected return)\n", reg_name);
    return 1;
}

int sreg_case_auth_tamper_overwrite(int reg_index)
{
    const char *reg_name = sreg_guard_reg_name(reg_index);

    printf("[SREG-GUARD] reg=%s case08_auth_tamper_overwrite: step1 call untrusted (expect AUTH fault)\n",
           reg_name);
    fflush(stdout);
    sreg_guard_call_untrusted(sreg_lookup_fn(g_auth_tamper_overwrite_fns, reg_index));
    printf("[SREG-GUARD] reg=%s case08_auth_tamper_overwrite: FAIL (unexpected return)\n", reg_name);
    return 1;
}
