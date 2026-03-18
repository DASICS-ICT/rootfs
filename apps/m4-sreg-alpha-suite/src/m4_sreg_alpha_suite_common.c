#include "m4_sreg_alpha_suite.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <udasics.h>

extern char __ULIBTEXT_BEGIN__, __ULIBTEXT_END__;

m4_alpha_shared_t g_m4_alpha_shared;

static int g_cfg_ids[2] = {-1, -1};
static int g_jumpcfg_id = -1;

static const char *const g_case_names[] = {
    M4_CASE00_NO_TOUCH_FAST_RETURN,
    M4_CASE01_FIRST_READ_TOKENIZE_THEN_OPEN,
    M4_CASE02_TOKEN_STACK_ROUNDTRIP,
    M4_CASE03_TOKEN_ARITH_CORRUPT_FAULT,
    M4_CASE04_FIRST_DEST_WRITE_UNUSED,
    M4_CASE05_FIRST_DEST_WRITE_TRUST_OVERWRITE,
    M4_CASE06_FIRST_DEST_WRITE_TRUST_USE,
    M4_CASE07_SWAP_S1_TO_S2_FAULT,
    M4_CASE07_SWAP_S1_TO_S11_FAULT,
    M4_CASE08_MULTI_SLOT_INDEPENDENCE,
    M4_CASE09_NATURAL_PROLOGUE_ONLY,
    M4_CASE10_NATURAL_SINGLE_LAYER,
    M4_CASE11_NATURAL_SIMPLE_CHAIN,
    M4_CASE12_STACK_TOKEN_BITFLIP_FAULT,
    M4_CASE13_STACK_TOKEN_OVERFLOW_FAULT,
};

static void m4_log(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    fputs("[M4-ALPHA] ", stdout);
    vprintf(fmt, ap);
    fputc('\n', stdout);
    fflush(stdout);
    va_end(ap);
}

static void m4_reset_runtime_handles(void)
{
    g_cfg_ids[0] = -1;
    g_cfg_ids[1] = -1;
    g_jumpcfg_id = -1;
    memset(&g_m4_alpha_shared, 0, sizeof(g_m4_alpha_shared));
}

static void m4_fill_migrated_inputs(void)
{
    strcpy(g_m4_alpha_shared.record_texts[0], "alpha-7");
    g_m4_alpha_shared.record_weights[0] = 3;

    strcpy(g_m4_alpha_shared.record_texts[1], "Beta42");
    g_m4_alpha_shared.record_weights[1] = 5;

    strcpy(g_m4_alpha_shared.record_texts[2], "delta");
    g_m4_alpha_shared.record_weights[2] = 2;

    strcpy(g_m4_alpha_shared.record_texts[3], "kappa9");
    g_m4_alpha_shared.record_weights[3] = 4;
}

static int m4_alloc_runtime_cfg(void)
{
    register uint64_t sp asm("sp");

    g_cfg_ids[0] = (int)LIBCFG_ALLOC(DASICS_LIBCFG_R | DASICS_LIBCFG_W,
                                     &g_m4_alpha_shared,
                                     sizeof(g_m4_alpha_shared));
    g_cfg_ids[1] = (int)LIBCFG_ALLOC(DASICS_LIBCFG_R | DASICS_LIBCFG_W,
                                     (void *)(uintptr_t)(sp - 0x2000),
                                     0x2200);
    return g_cfg_ids[0] >= 0 && g_cfg_ids[1] >= 0;
}

int m4_call_untrusted(void (*fn)(void))
{
    if (fn == NULL) {
        return -1;
    }
    lib_call((void *)fn);
    return 0;
}

int m4_sreg_alpha_parse_case(const char *case_name)
{
    int i;

    if (case_name == NULL) {
        return -1;
    }
    for (i = 0; i < (int)(sizeof(g_case_names) / sizeof(g_case_names[0])); ++i) {
        if (strcmp(case_name, g_case_names[i]) == 0) {
            return i;
        }
    }
    return -1;
}

const char *m4_sreg_alpha_case_name(int case_id)
{
    if (case_id < 0 || case_id >= (int)(sizeof(g_case_names) / sizeof(g_case_names[0]))) {
        return "invalid";
    }
    return g_case_names[case_id];
}

const char *m4_sreg_alpha_slot_name(int slot_id)
{
    switch (slot_id) {
    case M4_SLOT_S1:
        return "s1";
    case M4_SLOT_S2:
        return "s2";
    case M4_SLOT_S11:
        return "s11";
    default:
        return "s?";
    }
}

int m4_sreg_alpha_runtime_init(const char *case_name)
{
    m4_reset_runtime_handles();
    m4_fill_migrated_inputs();
    register_udasics(0);

    g_jumpcfg_id = dasics_jumpcfg_alloc((uint64_t)&__ULIBTEXT_BEGIN__,
                                        (uint64_t)&__ULIBTEXT_END__);
    if (g_jumpcfg_id < 0) {
        m4_log("INIT FAIL: jumpcfg alloc failed");
        unregister_udasics();
        return -1;
    }
    if (!m4_alloc_runtime_cfg()) {
        m4_log("INIT FAIL: libcfg alloc failed");
        return -2;
    }

    m4_log("MODE: detect-only no-dynamic-AD per-slot-hidden-subkeys trusted-untrusted-reuse-dasics");
    m4_log("CASE START: %s", case_name);
    return 0;
}

void m4_sreg_alpha_runtime_fini(void)
{
    int i;

    for (i = 0; i < 2; ++i) {
        if (g_cfg_ids[i] >= 0) {
            dasics_libcfg_free(g_cfg_ids[i]);
        }
    }
    if (g_jumpcfg_id >= 0) {
        dasics_jumpcfg_free(g_jumpcfg_id);
    }
    unregister_udasics();
    m4_reset_runtime_handles();
}

static int m4_check_token_not_plain(uint64_t token, uint64_t plain, const char *label)
{
    if (token == 0 || token == plain) {
        m4_log("VERIFY FAIL: %s token=0x%016lx plain=0x%016lx", label, token, plain);
        return -1;
    }
    return 0;
}

static int m4_verify_result_fields(const char *label,
                                   uint64_t total_score,
                                   uint64_t weighted_score,
                                   uint64_t total_length,
                                   uint64_t unique_initials,
                                   const char *trace)
{
    if (g_m4_alpha_shared.result0 != total_score ||
        g_m4_alpha_shared.result1 != weighted_score ||
        g_m4_alpha_shared.result2 != total_length ||
        g_m4_alpha_shared.result3 != unique_initials) {
        m4_log("VERIFY FAIL: %s result=(%lu,%lu,%lu,%lu) expected=(%lu,%lu,%lu,%lu)",
               label,
               g_m4_alpha_shared.result0,
               g_m4_alpha_shared.result1,
               g_m4_alpha_shared.result2,
               g_m4_alpha_shared.result3,
               total_score, weighted_score, total_length, unique_initials);
        return -1;
    }
    if (strcmp(g_m4_alpha_shared.trace, trace) != 0) {
        m4_log("VERIFY FAIL: %s trace=%s expected=%s",
               label, g_m4_alpha_shared.trace, trace);
        return -2;
    }
    return 0;
}

int m4_case00_no_touch_fast_return(void)
{
    return m4_call_untrusted(m4_untrusted_no_touch);
}

int m4_case01_first_read_tokenize_then_open(void)
{
    uint64_t plain;

    plain = m4_trusted_call_s1_readback(m4_untrusted_first_read_s1, M4_PLAIN_S1);
    if (m4_check_token_not_plain(g_m4_alpha_shared.token0, M4_PLAIN_S1, "case01") != 0) {
        return -2;
    }
    m4_log("BOUNDARY stage=seal slot=s1 kind=cipher token=0x%016lx", g_m4_alpha_shared.token0);
    if (plain != M4_PLAIN_S1) {
        m4_log("VERIFY FAIL: case01 plain=0x%016lx expected=0x%016lx",
               plain, (uint64_t)M4_PLAIN_S1);
        return -3;
    }
    return 0;
}

int m4_case02_token_passthrough_stack_roundtrip(void)
{
    uint64_t plain;

    plain = m4_trusted_call_s1_readback(m4_untrusted_token_roundtrip_s1, M4_PLAIN_S1);
    if (g_m4_alpha_shared.fail_code != 0) {
        m4_log("VERIFY FAIL: case02 fail_code=%lu", g_m4_alpha_shared.fail_code);
        return -2;
    }
    if (g_m4_alpha_shared.token0 != g_m4_alpha_shared.token1 ||
        g_m4_alpha_shared.token0 != g_m4_alpha_shared.token2) {
        m4_log("VERIFY FAIL: case02 token mismatch 0x%016lx 0x%016lx 0x%016lx",
               g_m4_alpha_shared.token0,
               g_m4_alpha_shared.token1,
               g_m4_alpha_shared.token2);
        return -3;
    }
    if (m4_check_token_not_plain(g_m4_alpha_shared.token0, M4_PLAIN_S1, "case02") != 0) {
        return -4;
    }
    m4_log("BOUNDARY stage=reg slot=s1 kind=cipher token=0x%016lx", g_m4_alpha_shared.token0);
    m4_log("BOUNDARY stage=stack slot=s1 kind=cipher token=0x%016lx", g_m4_alpha_shared.token1);
    m4_log("BOUNDARY stage=return slot=s1 kind=cipher token=0x%016lx", g_m4_alpha_shared.token2);
    if (plain != M4_PLAIN_S1) {
        m4_log("VERIFY FAIL: case02 plain=0x%016lx expected=0x%016lx",
               plain, (uint64_t)M4_PLAIN_S1);
        return -5;
    }
    return 0;
}

int m4_case03_token_arith_corrupt_fault(void)
{
    uint64_t observed = m4_trusted_call_s2_fault_readback(m4_untrusted_arith_corrupt_s2,
                                                          M4_PLAIN_S2);
    m4_log("VERIFY FAIL: case03 unexpected return value=0x%016lx token_before=0x%016lx token_after=0x%016lx",
           observed, g_m4_alpha_shared.token0, g_m4_alpha_shared.token1);
    return -2;
}

int m4_case04_first_dest_write_then_unused(void)
{
    m4_trusted_call_s1_no_readback(m4_untrusted_first_dest_write_s1, M4_PLAIN_S1);
    return 0;
}

int m4_case05_first_dest_write_then_trusted_overwrite(void)
{
    uint64_t value;

    value = m4_trusted_call_s1_overwrite_readback(m4_untrusted_first_dest_write_s1,
                                                  M4_PLAIN_S1, M4_NEW_S1);
    if (value != M4_NEW_S1) {
        m4_log("VERIFY FAIL: case05 value=0x%016lx expected=0x%016lx",
               value, (uint64_t)M4_NEW_S1);
        return -2;
    }
    return 0;
}

int m4_case06_first_dest_write_then_trusted_use_fault(void)
{
    uint64_t observed = m4_trusted_call_s11_fault_readback(m4_untrusted_first_dest_write_s11,
                                                           M4_PLAIN_S11);
    m4_log("VERIFY FAIL: case06 unexpected return value=0x%016lx untrusted_written=0x%016lx",
           observed, g_m4_alpha_shared.token0);
    return -2;
}

int m4_case07_swap_s1_to_s2_fault(void)
{
    uint64_t observed;

    m4_log("CASE07 pair=s1->s2 expect_fault_slot=s2");
    observed = m4_trusted_call_s1_s2_fault_readback(m4_untrusted_swap_s1_to_s2,
                                                    M4_PLAIN_S1, M4_PLAIN_S2);
    m4_log("VERIFY FAIL: case07 s1->s2 unexpected return value=0x%016lx token_src=0x%016lx token_dst=0x%016lx",
           observed, g_m4_alpha_shared.token0, g_m4_alpha_shared.token1);
    return -2;
}

int m4_case07_swap_s1_to_s11_fault(void)
{
    uint64_t observed;

    m4_log("CASE07 pair=s1->s11 expect_fault_slot=s11");
    observed = m4_trusted_call_s1_s11_fault_readback(m4_untrusted_swap_s1_to_s11,
                                                     M4_PLAIN_S1, M4_PLAIN_S11);
    m4_log("VERIFY FAIL: case07 s1->s11 unexpected return value=0x%016lx token_src=0x%016lx token_dst=0x%016lx",
           observed, g_m4_alpha_shared.token0, g_m4_alpha_shared.token1);
    return -2;
}

int m4_case08_multi_slot_independence(void)
{
    uint64_t v1;
    uint64_t v11;

    m4_trusted_call_s1_s11_readback(m4_untrusted_multi_slot_independence,
                                    M4_PLAIN_S1, M4_PLAIN_S11, &v1, &v11);
    if (m4_check_token_not_plain(g_m4_alpha_shared.token0, M4_PLAIN_S1, "case08.s1") != 0) {
        return -2;
    }
    if (m4_check_token_not_plain(g_m4_alpha_shared.token1, M4_PLAIN_S11, "case08.s11") != 0) {
        return -3;
    }
    if (g_m4_alpha_shared.token0 == g_m4_alpha_shared.token1) {
        m4_log("VERIFY FAIL: case08 token alias 0x%016lx", g_m4_alpha_shared.token0);
        return -4;
    }
    if (v1 != M4_PLAIN_S1 || v11 != M4_PLAIN_S11) {
        m4_log("VERIFY FAIL: case08 s1=0x%016lx s11=0x%016lx", v1, v11);
        return -5;
    }
    return 0;
}
