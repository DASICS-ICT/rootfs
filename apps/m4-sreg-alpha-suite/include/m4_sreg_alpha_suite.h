#ifndef M4_SREG_ALPHA_SUITE_H
#define M4_SREG_ALPHA_SUITE_H

#include <stdint.h>

enum {
    M4_SLOT_S1 = 1,
    M4_SLOT_S2 = 2,
    M4_SLOT_S11 = 11,
};

#define M4_CASE00_NO_TOUCH_FAST_RETURN             "case00_no_touch_fast_return"
#define M4_CASE01_FIRST_READ_TOKENIZE_THEN_OPEN    "case01_first_read_tokenize_then_open"
#define M4_CASE02_TOKEN_STACK_ROUNDTRIP            "case02_token_passthrough_stack_roundtrip"
#define M4_CASE03_TOKEN_ARITH_CORRUPT_FAULT        "case03_token_arith_corrupt_fault"
#define M4_CASE04_FIRST_DEST_WRITE_UNUSED          "case04_first_dest_write_then_unused"
#define M4_CASE05_FIRST_DEST_WRITE_TRUST_OVERWRITE "case05_first_dest_write_then_trusted_overwrite"
#define M4_CASE06_FIRST_DEST_WRITE_TRUST_USE       "case06_first_dest_write_then_trusted_use_fault"
#define M4_CASE07_SWAP_S1_TO_S2_FAULT              "case07_swap_s1_to_s2_fault"
#define M4_CASE07_SWAP_S1_TO_S11_FAULT             "case07_swap_s1_to_s11_fault"
#define M4_CASE08_MULTI_SLOT_INDEPENDENCE          "case08_multi_slot_independence"
#define M4_CASE09_NATURAL_PROLOGUE_ONLY            "case09_natural_prologue_only"
#define M4_CASE10_NATURAL_SINGLE_LAYER             "case10_natural_single_layer"
#define M4_CASE11_NATURAL_SIMPLE_CHAIN             "case11_natural_simple_chain"
#define M4_CASE12_STACK_TOKEN_BITFLIP_FAULT        "case12_stack_token_bitflip_fault"
#define M4_CASE13_STACK_TOKEN_OVERFLOW_FAULT       "case13_stack_token_overflow_fault"

#define M4_PLAIN_S1   0x1111222233334444ULL
#define M4_PLAIN_S2   0x2222333344445555ULL
#define M4_PLAIN_S11  0xbbbbaaaa77776666ULL
#define M4_NEW_S1     0x55aa55aa55aa55aaULL
#define M4_RECORD_COUNT 4
#define M4_TEXT_LEN    16

typedef struct {
    uint64_t token0;
    uint64_t token1;
    uint64_t token2;
    uint64_t aux0;
    uint64_t aux1;
    uint64_t fail_code;
    uint64_t result0;
    uint64_t result1;
    uint64_t result2;
    uint64_t result3;
    char trace[8];
    char record_texts[M4_RECORD_COUNT][M4_TEXT_LEN];
    uint32_t record_weights[M4_RECORD_COUNT];
} m4_alpha_shared_t;

extern m4_alpha_shared_t g_m4_alpha_shared;

int m4_sreg_alpha_parse_case(const char *case_name);
const char *m4_sreg_alpha_case_name(int case_id);
const char *m4_sreg_alpha_slot_name(int slot_id);

int m4_sreg_alpha_runtime_init(const char *case_name);
void m4_sreg_alpha_runtime_fini(void);
int m4_call_untrusted(void (*fn)(void));

int m4_case00_no_touch_fast_return(void);
int m4_case01_first_read_tokenize_then_open(void);
int m4_case02_token_passthrough_stack_roundtrip(void);
int m4_case03_token_arith_corrupt_fault(void);
int m4_case04_first_dest_write_then_unused(void);
int m4_case05_first_dest_write_then_trusted_overwrite(void);
int m4_case06_first_dest_write_then_trusted_use_fault(void);
int m4_case07_swap_s1_to_s2_fault(void);
int m4_case07_swap_s1_to_s11_fault(void);
int m4_case08_multi_slot_independence(void);
int m4_case09_natural_prologue_only(void);
int m4_case10_natural_single_layer(void);
int m4_case11_natural_simple_chain(void);
int m4_case12_stack_token_bitflip_fault(void);
int m4_case13_stack_token_overflow_fault(void);

uint64_t m4_trusted_call_s1_readback(void (*fn)(void), uint64_t initial);
void m4_trusted_call_s1_no_readback(void (*fn)(void), uint64_t initial);
uint64_t m4_trusted_call_s1_overwrite_readback(void (*fn)(void), uint64_t initial,
                                               uint64_t overwrite);
uint64_t m4_trusted_call_s1_fault_readback(void (*fn)(void), uint64_t initial);
uint64_t m4_trusted_call_s2_fault_readback(void (*fn)(void), uint64_t initial);
uint64_t m4_trusted_call_s11_fault_readback(void (*fn)(void), uint64_t initial);
uint64_t m4_trusted_call_s1_s2_fault_readback(void (*fn)(void), uint64_t plain_s1,
                                              uint64_t plain_s2);
uint64_t m4_trusted_call_s1_s11_fault_readback(void (*fn)(void), uint64_t plain_s1,
                                               uint64_t plain_s11);
void m4_trusted_call_s1_s11_readback(void (*fn)(void), uint64_t plain_s1,
                                     uint64_t plain_s11,
                                     uint64_t *out_s1, uint64_t *out_s11);
void m4_trusted_call_safe_return(void (*fn)(void), uint64_t plain_s1,
                                 uint64_t plain_s11);

void m4_untrusted_no_touch(void);
void m4_untrusted_first_read_s1(void);
void m4_untrusted_token_roundtrip_s1(void);
void m4_untrusted_arith_corrupt_s1(void);
void m4_untrusted_arith_corrupt_s2(void);
void m4_untrusted_first_dest_write_s1(void);
void m4_untrusted_first_dest_write_s11(void);
void m4_untrusted_swap_s1_to_s2(void);
void m4_untrusted_swap_s1_to_s11(void);
void m4_untrusted_multi_slot_independence(void);
void m4_untrusted_case09_natural_prologue_only(void);
void m4_untrusted_case10_natural_single_layer(void);
void m4_untrusted_case11_natural_simple_chain(void);
void m4_untrusted_stack_token_bitflip_s1(void);
void m4_untrusted_stack_token_overflow_s1(void);

#endif
