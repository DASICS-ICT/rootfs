#include "m3_cprog_feas.h"

typedef struct {
    uint32_t score;
    uint32_t weighted_score;
    uint32_t total_length;
    uint32_t initial_mask;
    char initial;
} m3_case01_step_t;

M3_ULIB_TEXT int m3_untrusted_process_case00_single_slot(const m3_request_t *req,
                                                         m3_result_t *res)
{
    register uint64_t keep1 asm("s1");

    if (req == 0 || res == 0 || req->record_count != M3_MAX_RECORDS) {
        return 10;
    }

    keep1 = req->record_count;
    keep1 += req->records[0].weight;
    keep1 += req->records[1].weight;

    asm volatile("" : "+r"(keep1));

    res->total_score = (uint32_t)keep1;
    res->weighted_score = (uint32_t)(keep1 + req->records[2].weight);
    res->total_length = req->record_count;
    res->unique_initials = 1;
    res->trace[0] = '\0';
    return 0;
}

M3_ULIB_TEXT int m3_untrusted_process_case00(const m3_request_t *req,
                                             m3_result_t *res)
{
    register uint64_t keep0 asm("s0");
    register uint64_t keep1 asm("s1");
    register uint64_t keep2 asm("s2");
    register uint64_t keep3 asm("s3");
    register uint64_t keep4 asm("s4");

    if (req == 0 || res == 0 || req->record_count != M3_MAX_RECORDS) {
        return 10;
    }

    keep0 = req->record_count;
    keep1 = req->records[0].weight;
    keep2 = req->records[1].weight;
    keep3 = req->records[2].weight;
    keep4 = req->records[3].weight;

    keep0 += keep1;
    keep2 += keep3;
    keep4 += keep0;
    keep4 += keep2;

    asm volatile("" : "+r"(keep0), "+r"(keep1), "+r"(keep2), "+r"(keep3), "+r"(keep4));

    res->total_score = (uint32_t)keep4;
    res->weighted_score = (uint32_t)(keep0 + keep2);
    res->total_length = (uint32_t)(keep1 + keep3);
    res->unique_initials = (uint32_t)req->record_count;
    res->trace[0] = '\0';
    return 0;
}

static int m3_is_lower(char ch)
{
    return ch >= 'a' && ch <= 'z';
}

static int m3_is_upper(char ch)
{
    return ch >= 'A' && ch <= 'Z';
}

static int m3_is_digit(char ch)
{
    return ch >= '0' && ch <= '9';
}

static char m3_to_upper(char ch)
{
    if (m3_is_lower(ch)) {
        return (char)(ch - ('a' - 'A'));
    }
    return ch;
}

static int m3_normalize_token_local(const char *src, char *dst, int dst_len)
{
    int src_idx = 0;
    int dst_idx = 0;

    if (src == 0 || dst == 0 || dst_len <= 1) {
        return -1;
    }

    while (src[src_idx] != '\0') {
        char ch = src[src_idx++];

        if (m3_is_lower(ch) || m3_is_upper(ch) || m3_is_digit(ch)) {
            if (dst_idx >= dst_len - 1) {
                return -2;
            }
            dst[dst_idx++] = m3_to_upper(ch);
        }
    }

    if (dst_idx == 0) {
        return -3;
    }

    dst[dst_idx] = '\0';
    return dst_idx;
}

static uint32_t m3_letter_score(char ch)
{
    if (ch >= 'A' && ch <= 'Z') {
        return (uint32_t)(ch - 'A' + 1);
    }
    if (ch >= '0' && ch <= '9') {
        return (uint32_t)(ch - '0');
    }
    return 0;
}

static uint32_t m3_score_token_local(const char *token)
{
    uint32_t total = 0;
    int idx = 0;

    if (token == 0) {
        return 0;
    }

    while (token[idx] != '\0') {
        total += m3_letter_score(token[idx]);
        idx++;
    }

    return total;
}

static uint32_t m3_initial_mask_local(const char *token)
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

M3_ULIB_TEXT static int m3_untrusted_case01_step(const m3_record_t *rec,
                                                 m3_case01_step_t *step)
{
    register uint64_t keep1 asm("s1");
    register uint64_t keep2 asm("s2");
    char normalized[M3_MAX_TOKEN_LEN];
    int norm_len;

    if (rec == 0 || step == 0) {
        return -1;
    }

    norm_len = m3_normalize_token_local(rec->text, normalized, M3_MAX_TOKEN_LEN);
    if (norm_len <= 0) {
        return norm_len;
    }

    keep1 = m3_score_token_local(normalized);
    keep2 = keep1 * rec->weight;

    asm volatile("" : "+r"(keep1), "+r"(keep2));

    step->score = (uint32_t)keep1;
    step->weighted_score = (uint32_t)keep2;
    step->total_length = (uint32_t)norm_len;
    step->initial_mask = m3_initial_mask_local(normalized);
    step->initial = normalized[0];
    return 0;
}

M3_ULIB_TEXT int m3_untrusted_process_case01(const m3_request_t *req,
                                             m3_result_t *res)
{
    register uint64_t total_score asm("s1");
    register uint64_t weighted_score asm("s2");
    register uint64_t total_length asm("s3");
    register uint64_t initial_mask asm("s4");
    uint32_t unique_initials = 0;
    uint32_t idx;
    m3_case01_step_t step;

    if (req == 0 || res == 0 || req->record_count != M3_MAX_RECORDS) {
        return 10;
    }

    total_score = 0;
    weighted_score = 0;
    total_length = 0;
    initial_mask = 0;

    for (idx = 0; idx < req->record_count; idx++) {
        int rc = m3_untrusted_case01_step(&req->records[idx], &step);

        if (rc != 0) {
            return 20 + (int)idx;
        }

        total_score += step.score;
        weighted_score += step.weighted_score;
        total_length += step.total_length;
        initial_mask |= step.initial_mask;
        res->trace[idx] = step.initial;
    }

    while (initial_mask != 0) {
        unique_initials += (initial_mask & 1u);
        initial_mask >>= 1;
    }

    res->trace[req->record_count] = '\0';
    res->total_score = total_score;
    res->weighted_score = weighted_score;
    res->total_length = total_length;
    res->unique_initials = unique_initials;
    return 0;
}

M3_ULIB_TEXT int m3_untrusted_process_case10_natural_single_layer(
    const m3_request_t *req, m3_result_t *res)
{
    uint32_t total_score = 0;
    uint32_t weighted_score = 0;
    uint32_t total_length = 0;
    uint32_t initial_mask = 0;
    uint32_t unique_initials = 0;
    uint32_t idx;

    if (req == 0 || res == 0 || req->record_count != M3_MAX_RECORDS) {
        return 10;
    }

    for (idx = 0; idx < req->record_count; idx++) {
        const char *src = req->records[idx].text;
        uint32_t score = 0;
        uint32_t src_idx = 0;
        uint32_t norm_len = 0;
        char first = '\0';

        for (;;) {
            char ch = src[src_idx++];

            if (ch == '\0') {
                break;
            }
            if (ch >= 'a' && ch <= 'z') {
                ch = (char)(ch - ('a' - 'A'));
            }
            if (!((ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'))) {
                continue;
            }
            if (norm_len == 0) {
                first = ch;
            }
            norm_len++;
            if (ch >= 'A' && ch <= 'Z') {
                score += (uint32_t)(ch - 'A' + 1);
            } else {
                score += (uint32_t)(ch - '0');
            }
        }

        if (norm_len == 0) {
            return 20 + (int)idx;
        }

        total_score += score;
        weighted_score += score * req->records[idx].weight;
        total_length += norm_len;
        if (first >= 'A' && first <= 'Z') {
            initial_mask |= (uint32_t)1u << (uint32_t)(first - 'A');
        } else if (first >= '0' && first <= '9') {
            initial_mask |= (uint32_t)1u << (uint32_t)(26 + (first - '0'));
        }
        res->trace[idx] = first;
    }

    while (initial_mask != 0) {
        unique_initials += (initial_mask & 1u);
        initial_mask >>= 1;
    }

    res->trace[req->record_count] = '\0';
    res->total_score = total_score;
    res->weighted_score = weighted_score;
    res->total_length = total_length;
    res->unique_initials = unique_initials;
    return 0;
}

typedef struct {
    uint32_t score;
    uint32_t weighted_score;
    uint32_t total_length;
    uint32_t initial_mask;
    char initial;
} m3_case11_step_t;

M3_ULIB_TEXT static int m3_untrusted_case11_step(const m3_record_t *rec,
                                                 m3_case11_step_t *step)
{
    const char *src;
    uint32_t score = 0;
    uint32_t src_idx = 0;
    uint32_t norm_len = 0;
    char first = '\0';

    if (rec == 0 || step == 0) {
        return -1;
    }

    src = rec->text;
    for (;;) {
        char ch = src[src_idx++];

        if (ch == '\0') {
            break;
        }
        if (ch >= 'a' && ch <= 'z') {
            ch = (char)(ch - ('a' - 'A'));
        }
        if (!((ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'))) {
            continue;
        }
        if (norm_len == 0) {
            first = ch;
        }
        norm_len++;
        if (ch >= 'A' && ch <= 'Z') {
            score += (uint32_t)(ch - 'A' + 1);
        } else {
            score += (uint32_t)(ch - '0');
        }
    }

    if (norm_len == 0) {
        return -3;
    }

    step->score = score;
    step->weighted_score = score * rec->weight;
    step->total_length = norm_len;
    if (first >= 'A' && first <= 'Z') {
        step->initial_mask = (uint32_t)1u << (uint32_t)(first - 'A');
    } else {
        step->initial_mask = (uint32_t)1u << (uint32_t)(26 + (first - '0'));
    }
    step->initial = first;
    return 0;
}

M3_ULIB_TEXT int m3_untrusted_process_case11_natural_simple_chain(
    const m3_request_t *req, m3_result_t *res)
{
    uint32_t total_score = 0;
    uint32_t weighted_score = 0;
    uint32_t total_length = 0;
    uint32_t initial_mask = 0;
    uint32_t unique_initials = 0;
    uint32_t idx;

    if (req == 0 || res == 0 || req->record_count != M3_MAX_RECORDS) {
        return 10;
    }

    for (idx = 0; idx < req->record_count; idx++) {
        m3_case11_step_t step;
        int rc = m3_untrusted_case11_step(&req->records[idx], &step);

        if (rc != 0) {
            return 20 + (int)idx;
        }

        total_score += step.score;
        weighted_score += step.weighted_score;
        total_length += step.total_length;
        initial_mask |= step.initial_mask;
        res->trace[idx] = step.initial;
    }

    while (initial_mask != 0) {
        unique_initials += (initial_mask & 1u);
        initial_mask >>= 1;
    }

    res->trace[req->record_count] = '\0';
    res->total_score = total_score;
    res->weighted_score = weighted_score;
    res->total_length = total_length;
    res->unique_initials = unique_initials;
    return 0;
}

M3_ULIB_TEXT void m3_untrusted_case22_overflow_helper(volatile uint8_t *target)
{
    volatile uint8_t overflow_buf[8];
    intptr_t delta;
    int i;

    overflow_buf[0] = 0;
    delta = (intptr_t)target - (intptr_t)&overflow_buf[0];
    for (i = 0; i < 8; i++) {
        *(((volatile uint8_t *)&overflow_buf[0]) + delta + i) = 0xffu;
    }
}
