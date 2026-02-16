# s0-guard-tests

Runtime validation suite for QEMU `s0` guard v1.

Binary:
- `/root/test_s0_guard_suite`

Usage:
- `/root/test_s0_guard_suite --case viol_write`
- `/root/test_s0_guard_suite --case viol_read`
- `/root/test_s0_guard_suite --case proto_mismatch`
- `/root/test_s0_guard_suite --case legal_save_restore`

Expected result:
- `viol_write`: fault expected (non-zero exit)
- `viol_read`: fault expected (non-zero exit)
- `proto_mismatch`: fault expected (non-zero exit)
- `legal_save_restore`: success expected (zero exit)
