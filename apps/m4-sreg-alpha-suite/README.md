# m4-sreg-alpha-suite

这是一个独立的 `M4.Q1-alpha` rootfs 测试套件，用于验证 `detect-only`、`no-dynamic-AD`、`per-slot hidden subkeys`、`AUTH` 唯一 fault 路径，以及 trusted/untrusted 复用 DASICS judgment 的 lane。

二进制：
- `/root/test_m4_sreg_alpha_suite`

运行方式：
- `/root/test_m4_sreg_alpha_suite --case case02_token_passthrough_stack_roundtrip`

全量 case：
- `case00_no_touch_fast_return`
- `case01_first_read_tokenize_then_open`
- `case02_token_passthrough_stack_roundtrip`
- `case03_token_arith_corrupt_fault`
- `case04_first_dest_write_then_unused`
- `case05_first_dest_write_then_trusted_overwrite`
- `case06_first_dest_write_then_trusted_use_fault`
- `case07_swap_s1_to_s2_fault`
- `case07_swap_s1_to_s11_fault`
- `case08_multi_slot_independence`

关键日志标记：
- `[M4-ALPHA] MODE:`: 运行模式说明，固定包含 `detect-only` / `no-dynamic-AD` / `per-slot-hidden-subkeys`
- `[M4-ALPHA] CASE START:`: case 起点
- `[M4-ALPHA] BOUNDARY stage=seal|reg|mem|ret|trusted-open`: 记录 token 在 seal、寄存器、内存、返回、trusted open 边界上的观测值
- `[DASICS SREG] AUTH fault ... utval=0xX`: 复用现有 fault 打印，`utval` 记录消费槽位号
- `[M4-ALPHA] RESULT: PASS (...)`: shell lane 的 PASS 判据

判据摘要：
- `case02` 只在 `cipher` token 上通过，不允许在 seal 到 trusted open 的路径上出现明文 token
- `case07` 必须覆盖 `s1->s2` 与 `s1->s11`，并且 fault 日志中必须可见正确 `utval`
- `case08` 用于检查多个槽位的 token 彼此独立
