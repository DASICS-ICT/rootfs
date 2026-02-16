#ifndef SREG_GUARD_COMMON_H
#define SREG_GUARD_COMMON_H

#include <stdint.h>

#define SREG_COUNT 11

int sreg_guard_runtime_init(const char *mode_name);
void sreg_guard_runtime_fini(void);

void sreg_set_pattern(uint64_t seed);
void sreg_take_snapshot(uint64_t out[SREG_COUNT]);
void sreg_restore_snapshot(const uint64_t in[SREG_COUNT]);
uint64_t sreg_simple_tag(const uint64_t regs[SREG_COUNT]);

int sreg_expect_match(
    const char *scene,
    const uint64_t expected[SREG_COUNT],
    const uint64_t actual[SREG_COUNT]);
int sreg_expect_mismatch(
    const char *scene,
    const uint64_t expected[SREG_COUNT],
    const uint64_t actual[SREG_COUNT]);

int sreg_run_legal_save_then_use_scene(void);
int sreg_call_untrusted_overwrite(void);
int sreg_call_untrusted_save_then_use(void);

/* Assembly helpers. */
void sreg_set_pattern_asm(uint64_t seed);
void sreg_snapshot_asm(uint64_t *out);
void sreg_restore_asm(const uint64_t *in);
void sreg_untrusted_overwrite(void);
void sreg_untrusted_save_then_use(void);

#endif
