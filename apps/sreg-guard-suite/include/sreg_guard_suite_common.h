#ifndef SREG_GUARD_SUITE_COMMON_H
#define SREG_GUARD_SUITE_COMMON_H

int sreg_guard_parse_reg(const char *reg_name);
const char *sreg_guard_reg_name(int reg_index);

int sreg_guard_runtime_init(const char *reg_name, const char *case_name);
void sreg_guard_runtime_fini(void);
int sreg_guard_call_untrusted(void (*fn)(void));

int sreg_case_viol_write(int reg_index);
int sreg_case_viol_read(int reg_index);
int sreg_case_proto_mismatch(int reg_index);
int sreg_case_legal_save_restore(int reg_index);
int sreg_case_auth_tamper_bitflip(int reg_index);
int sreg_case_auth_tamper_overwrite(int reg_index);

#endif
