#include "m4_sreg_alpha_suite.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <uattr.h>

#define M4_ULIB_TEXT ATTR_ULIB_TEXT __attribute__((noinline))

typedef struct {
    uint32_t total_score;
    uint32_t weighted_score;
    uint32_t total_length;
    uint32_t unique_initials;
    const char *trace;
} m4_expected_result_t;

static void m4_migrated_log(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    fputs("[M4-ALPHA] ", stdout);
    vprintf(fmt, ap);
    fputc('\n', stdout);
    fflush(stdout);
    va_end(ap);
}

static int m4_is_lower(char ch)
{
    return ch >= 'a' && ch <= 'z';
}

static int m4_is_upper(char ch)
{
    return ch >= 'A' && ch <= 'Z';
}

static int m4_is_digit(char ch)
{
    return ch >= '0' && ch <= '9';
}

static char m4_to_upper(char ch)
{
    if (m4_is_lower(ch)) {
        return (char)(ch - ('a' - 'A'));
    }
    return ch;
}

static int m4_normalize_token_local(const char *src, char *dst, int dst_len)
{
    int src_idx = 0;
    int dst_idx = 0;

    if (src == 0 || dst == 0 || dst_len <= 1) {
        return -1;
    }

    while (src[src_idx] != '\0') {
        char ch = src[src_idx++];

        if (m4_is_lower(ch) || m4_is_upper(ch) || m4_is_digit(ch)) {
            if (dst_idx >= dst_len - 1) {
                return -2;
            }
            dst[dst_idx++] = m4_to_upper(ch);
        }
    }

    if (dst_idx == 0) {
        return -3;
    }

    dst[dst_idx] = '\0';
    return dst_idx;
}

static uint32_t m4_letter_score(char ch)
{
    if (ch >= 'A' && ch <= 'Z') {
        return (uint32_t)(ch - 'A' + 1);
    }
    if (ch >= '0' && ch <= '9') {
        return (uint32_t)(ch - '0');
    }
    return 0;
}

static uint32_t m4_score_token_local(const char *token)
{
    uint32_t total = 0;
    int idx = 0;

    while (token != 0 && token[idx] != '\0') {
        total += m4_letter_score(token[idx]);
        idx++;
    }
    return total;
}

static uint32_t m4_initial_mask_local(const char *token)
{
    char ch;

    if (token == 0 || token[0] == '\0') {
        return 0;
    }

    ch = token[0];
    if (ch >= 'A' && ch <= 'Z') {
        return (uint32_t)1u << (uint32_t)(ch - 'A');
    }
    if (ch >= '0' && ch <= '9') {
        return (uint32_t)1u << (uint32_t)(26 + (ch - '0'));
    }
    return 0;
}

static uint32_t m4_initial_mask_ch(char ch)
{
    if (ch >= 'A' && ch <= 'Z') {
        return (uint32_t)1u << (uint32_t)(ch - 'A');
    }
    if (ch >= '0' && ch <= '9') {
        return (uint32_t)1u << (uint32_t)(26 + (ch - '0'));
    }
    return 0;
}

static void m4_store_result(uint32_t total_score, uint32_t weighted_score,
                            uint32_t total_length, uint32_t unique_initials,
                            const char *trace)
{
    int idx;

    g_m4_alpha_shared.result0 = total_score;
    g_m4_alpha_shared.result1 = weighted_score;
    g_m4_alpha_shared.result2 = total_length;
    g_m4_alpha_shared.result3 = unique_initials;
    for (idx = 0; idx < 8; idx++) {
        g_m4_alpha_shared.trace[idx] = '\0';
    }
    if (trace != 0) {
        for (idx = 0; idx < 7 && trace[idx] != '\0'; idx++) {
            g_m4_alpha_shared.trace[idx] = trace[idx];
        }
        g_m4_alpha_shared.trace[idx] = '\0';
    }
}

static int m4_verify_positive_result(const char *label,
                                     const m4_expected_result_t *expect)
{
    if (g_m4_alpha_shared.aux0 == 0 || g_m4_alpha_shared.aux0 == M4_PLAIN_S1 ||
        g_m4_alpha_shared.aux1 == 0 || g_m4_alpha_shared.aux1 == M4_PLAIN_S11) {
        m4_migrated_log("VERIFY FAIL: %s token capture aux0=0x%016lx aux1=0x%016lx",
                        label, g_m4_alpha_shared.aux0, g_m4_alpha_shared.aux1);
        return -3;
    }

    if (g_m4_alpha_shared.result0 != expect->total_score ||
        g_m4_alpha_shared.result1 != expect->weighted_score ||
        g_m4_alpha_shared.result2 != expect->total_length ||
        g_m4_alpha_shared.result3 != expect->unique_initials ||
        strcmp(g_m4_alpha_shared.trace, expect->trace) != 0) {
        m4_migrated_log(
            "VERIFY FAIL: %s result total=%lu weighted=%lu len=%lu initials=%lu trace=%s",
            label,
            g_m4_alpha_shared.result0,
            g_m4_alpha_shared.result1,
            g_m4_alpha_shared.result2,
            g_m4_alpha_shared.result3,
            g_m4_alpha_shared.trace);
        return -1;
    }

    return 0;
}

M4_ULIB_TEXT static uint64_t m4_case09_fold_weights(uint64_t a, uint64_t b,
                                                    uint64_t c, uint64_t d,
                                                    uint64_t e)
{
    volatile uint64_t sink = 0;

    a += b;
    c += d;
    e += a;
    e += c;
    sink = e;
    return sink;
}

M4_ULIB_TEXT void m4_untrusted_case09_natural_prologue_only(void)
{
    volatile uint64_t warmup[4];
    uint64_t seed_s1;
    uint64_t seed_s11;
    uint64_t keep0 = 4;
    uint64_t keep1 = g_m4_alpha_shared.record_weights[0];
    uint64_t keep2 = g_m4_alpha_shared.record_weights[1];
    uint64_t keep3 = g_m4_alpha_shared.record_weights[2];
    uint64_t keep4 = g_m4_alpha_shared.record_weights[3];
    uint64_t total_score;

    asm volatile("mv %0, s1\n\tmv %1, s11"
                 : "=r"(seed_s1), "=r"(seed_s11));
    g_m4_alpha_shared.aux0 = seed_s1;
    g_m4_alpha_shared.aux1 = seed_s11;
    warmup[0] = 1;
    warmup[1] = 3;
    warmup[2] = 5;
    warmup[3] = 7;

    total_score = m4_case09_fold_weights(keep0, keep1, keep2, keep3, keep4);
    warmup[0] ^= total_score;

    m4_store_result((uint32_t)total_score,
                    (uint32_t)((keep0 + keep1) + (keep2 + keep3)),
                    (uint32_t)(keep1 + keep3),
                    4,
                    0);
    g_m4_alpha_shared.fail_code = warmup[0] == 0 ? 1 : 0;
}

M4_ULIB_TEXT void m4_untrusted_case10_natural_single_layer(void)
{
    uint64_t seed_s1;
    uint64_t seed_s11;
    uint32_t total_score = 0;
    uint32_t weighted_score = 0;
    uint32_t total_length = 0;
    uint32_t initial_mask = 0;
    uint32_t unique_initials = 0;
    uint32_t idx;
    char trace[5];

    asm volatile("mv %0, s1\n\tmv %1, s11"
                 : "=r"(seed_s1), "=r"(seed_s11));
    g_m4_alpha_shared.aux0 = seed_s1;
    g_m4_alpha_shared.aux1 = seed_s11;

    for (idx = 0; idx < 4; idx++) {
        const char *src = g_m4_alpha_shared.record_texts[idx];
        uint32_t score = 0;
        uint32_t src_idx = 0;
        uint32_t norm_len = 0;
        char first = '\0';

        for (;;) {
            char ch = src[src_idx++];

            if (ch == '\0') {
                break;
            }
            if (m4_is_lower(ch)) {
                ch = (char)(ch - ('a' - 'A'));
            }
            if (!(m4_is_upper(ch) || m4_is_digit(ch))) {
                continue;
            }
            if (norm_len == 0) {
                first = ch;
            }
            norm_len++;
            score += m4_letter_score(ch);
        }

        total_score += score;
        weighted_score += score * g_m4_alpha_shared.record_weights[idx];
        total_length += norm_len;
        initial_mask |= m4_initial_mask_ch(first);
        trace[idx] = first;
    }

    while (initial_mask != 0) {
        unique_initials += (initial_mask & 1u);
        initial_mask >>= 1;
    }
    trace[4] = '\0';
    m4_store_result(total_score, weighted_score, total_length,
                    unique_initials, trace);
}

M4_ULIB_TEXT void m4_untrusted_case13_overflow_helper(volatile uint64_t *saved_token)
{
    volatile uint8_t overflow_buf[8];
    intptr_t delta;
    uint32_t idx;

    overflow_buf[0] = 0;
    delta = (intptr_t)((volatile uint8_t *)saved_token -
                       (volatile uint8_t *)&overflow_buf[0]);
    for (idx = 0; idx < sizeof(*saved_token); idx++) {
        *(((volatile uint8_t *)&overflow_buf[0]) + delta + idx) = 0xffu;
    }
}

typedef struct {
    uint32_t score;
    uint32_t weighted_score;
    uint32_t total_length;
    uint32_t initial_mask;
    char initial;
} m4_case11_step_t;

M4_ULIB_TEXT static int m4_case11_step(const char *text, uint32_t weight,
                                       m4_case11_step_t *step)
{
    char normalized[16];
    int norm_len;

    if (text == 0 || step == 0) {
        return -1;
    }

    norm_len = m4_normalize_token_local(text, normalized, sizeof(normalized));
    if (norm_len <= 0) {
        return norm_len;
    }

    step->score = m4_score_token_local(normalized);
    step->weighted_score = step->score * weight;
    step->total_length = (uint32_t)norm_len;
    step->initial_mask = m4_initial_mask_local(normalized);
    step->initial = normalized[0];
    return 0;
}

M4_ULIB_TEXT void m4_untrusted_case11_natural_simple_chain(void)
{
    uint64_t seed_s1;
    uint64_t seed_s11;
    uint32_t total_score = 0;
    uint32_t weighted_score = 0;
    uint32_t total_length = 0;
    uint32_t initial_mask = 0;
    uint32_t unique_initials = 0;
    uint32_t idx;
    char trace[5];

    asm volatile("mv %0, s1\n\tmv %1, s11"
                 : "=r"(seed_s1), "=r"(seed_s11));
    g_m4_alpha_shared.aux0 = seed_s1;
    g_m4_alpha_shared.aux1 = seed_s11;

    for (idx = 0; idx < 4; idx++) {
        m4_case11_step_t step;
        int rc = m4_case11_step(g_m4_alpha_shared.record_texts[idx],
                                g_m4_alpha_shared.record_weights[idx], &step);

        if (rc != 0) {
            g_m4_alpha_shared.fail_code = (uint64_t)(20 + idx);
            return;
        }

        total_score += step.score;
        weighted_score += step.weighted_score;
        total_length += step.total_length;
        initial_mask |= step.initial_mask;
        trace[idx] = step.initial;
    }

    while (initial_mask != 0) {
        unique_initials += (initial_mask & 1u);
        initial_mask >>= 1;
    }

    trace[4] = '\0';
    m4_store_result(total_score, weighted_score, total_length,
                    unique_initials, trace);
}

int m4_case09_natural_prologue_only(void)
{
    static const m4_expected_result_t expect = {18, 14, 5, 4, ""};

    m4_trusted_call_safe_return(m4_untrusted_case09_natural_prologue_only,
                                M4_PLAIN_S1, M4_PLAIN_S11);
    if (g_m4_alpha_shared.fail_code != 0) {
        m4_migrated_log("VERIFY FAIL: case09 fail_code=%lu", g_m4_alpha_shared.fail_code);
        return -2;
    }
    return m4_verify_positive_result("case09", &expect);
}

int m4_case10_natural_single_layer(void)
{
    static const m4_expected_result_t expect = {175, 605, 23, 4, "ABDK"};

    m4_trusted_call_safe_return(m4_untrusted_case10_natural_single_layer,
                                M4_PLAIN_S1, M4_PLAIN_S11);
    return m4_verify_positive_result("case10", &expect);
}

int m4_case11_natural_simple_chain(void)
{
    static const m4_expected_result_t expect = {175, 605, 23, 4, "ABDK"};

    m4_trusted_call_safe_return(m4_untrusted_case11_natural_simple_chain,
                                M4_PLAIN_S1, M4_PLAIN_S11);
    if (g_m4_alpha_shared.fail_code != 0) {
        m4_migrated_log("VERIFY FAIL: case11 fail_code=%lu", g_m4_alpha_shared.fail_code);
        return -2;
    }
    return m4_verify_positive_result("case11", &expect);
}

int m4_case12_stack_token_bitflip_fault(void)
{
    uint64_t observed;

    observed = m4_trusted_call_s1_fault_readback(m4_untrusted_stack_token_bitflip_s1,
                                                 M4_PLAIN_S1);
    m4_migrated_log(
        "VERIFY FAIL: case12 unexpected return value=0x%016lx token_before=0x%016lx token_after=0x%016lx",
        observed, g_m4_alpha_shared.token0, g_m4_alpha_shared.token1);
    return -2;
}

int m4_case13_stack_token_overflow_fault(void)
{
    uint64_t observed;

    observed = m4_trusted_call_s1_fault_readback(m4_untrusted_stack_token_overflow_s1,
                                                 M4_PLAIN_S1);
    m4_migrated_log(
        "VERIFY FAIL: case13 unexpected return value=0x%016lx token_before=0x%016lx token_after=0x%016lx",
        observed, g_m4_alpha_shared.token0, g_m4_alpha_shared.token1);
    return -2;
}
