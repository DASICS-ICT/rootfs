#ifndef M3_CPROG_FEAS_H
#define M3_CPROG_FEAS_H

#include <stdint.h>
#include <uattr.h>

#define M3_CASE_SINGLE_SLOT "case00_single_slot"
#define M3_CASE_PROLOGUE_ONLY "case00_prologue_only"
#define M3_CASE_SIMPLE_CHAIN "case01_simple_chain"
#define M3_CASE_NATURAL_SINGLE_LAYER "case10_natural_single_layer"
#define M3_CASE_NATURAL_SIMPLE_CHAIN "case11_natural_simple_chain"
#define M3_CASE_AUTH_TAMPER_BITFLIP_C "case20_auth_tamper_bitflip_c"
#define M3_CASE_AUTH_TAMPER_OVERWRITE_C "case21_auth_tamper_overwrite_c"
#define M3_CASE_AUTH_TAMPER_OVERFLOW_C "case22_auth_tamper_overflow_c"

#define M3_MAX_RECORDS 4
#define M3_MAX_TOKEN_LEN 16

#define M3_ULIB_TEXT ATTR_ULIB_TEXT __attribute__((noinline))

typedef struct {
    char text[M3_MAX_TOKEN_LEN];
    uint32_t weight;
} m3_record_t;

typedef struct {
    uint32_t record_count;
    m3_record_t records[M3_MAX_RECORDS];
} m3_request_t;

typedef struct {
    uint32_t total_score;
    uint32_t weighted_score;
    uint32_t total_length;
    uint32_t unique_initials;
    char trace[M3_MAX_RECORDS + 1];
} m3_result_t;

M3_ULIB_TEXT int m3_untrusted_process_case00_single_slot(const m3_request_t *req,
                                                         m3_result_t *res);
M3_ULIB_TEXT int m3_untrusted_process_case00(const m3_request_t *req,
                                             m3_result_t *res);
M3_ULIB_TEXT int m3_untrusted_process_case01(const m3_request_t *req,
                                             m3_result_t *res);
M3_ULIB_TEXT int m3_untrusted_process_case10_natural_single_layer(
    const m3_request_t *req, m3_result_t *res);
M3_ULIB_TEXT int m3_untrusted_process_case11_natural_simple_chain(
    const m3_request_t *req, m3_result_t *res);
M3_ULIB_TEXT int m3_untrusted_process_case20_auth_tamper_bitflip_c(
    const m3_request_t *req, m3_result_t *res);
M3_ULIB_TEXT int m3_untrusted_process_case21_auth_tamper_overwrite_c(
    const m3_request_t *req, m3_result_t *res);
M3_ULIB_TEXT int m3_untrusted_process_case22_auth_tamper_overflow_c(
    const m3_request_t *req, m3_result_t *res);
M3_ULIB_TEXT void m3_untrusted_case22_overflow_helper(volatile uint8_t *target);

#endif
