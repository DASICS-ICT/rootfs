# sreg-guard-suite

用于验证 QEMU 中 `s0-s11` 保护行为的运行时测试套件（runtime validation suite）。

二进制：
- `/root/test_sreg_guard_suite`

用法示例：
- `/root/test_sreg_guard_suite --reg s1 --case viol_write`
- `/root/test_sreg_guard_suite --reg s7 --case viol_read`
- `/root/test_sreg_guard_suite --reg s10 --case proto_mismatch`
- `/root/test_sreg_guard_suite --reg s11 --case legal_save_restore`
- `/root/test_sreg_guard_suite --reg s4 --case post_save_cipher_read`
- `/root/test_sreg_guard_suite --reg s2 --case auth_tamper_bitflip`
- `/root/test_sreg_guard_suite --reg s3 --case auth_tamper_overwrite`

各测试项预期：
- `viol_write`：触发越权异常（VIOL fault），非 0 退出。
- `viol_read`：触发越权异常（VIOL fault），非 0 退出。
- `proto_mismatch`：触发协议异常（PROTO fault），非 0 退出。
- `legal_save_restore`：合法保存/恢复成功，0 退出。
- `post_save_cipher_read`：合法保存后验证寄存器当前值等于栈中密文，再执行合法恢复，0 退出。
- `auth_tamper_bitflip`：非可信区对栈中保存密文做单比特翻转（bit flip），返回恢复时触发认证异常（AUTH fault），非 0 退出。
- `auth_tamper_overwrite`：非可信区对栈中保存密文做整字覆盖（overwrite），返回恢复时触发认证异常（AUTH fault），非 0 退出。
