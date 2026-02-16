#ifndef S0_GUARD_COMMON_H
#define S0_GUARD_COMMON_H

int s0_guard_runtime_init(const char *case_name);
void s0_guard_runtime_fini(void);
int s0_guard_call_untrusted(void (*fn)(void));

int s0_case_viol_write(void);
int s0_case_viol_read(void);
int s0_case_proto_mismatch(void);
int s0_case_legal_save_restore(void);

void s0_untrusted_viol_write(void);
void s0_untrusted_viol_read(void);
void s0_untrusted_proto_mismatch(void);
void s0_untrusted_legal_save_restore(void);

#endif
