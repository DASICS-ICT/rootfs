# sreg-guard-suite

Runtime validation suite for QEMU `s0-s11` guard behavior.

Binary:
- `/root/test_sreg_guard_suite`

Usage:
- `/root/test_sreg_guard_suite --reg s1 --case viol_write`
- `/root/test_sreg_guard_suite --reg s7 --case viol_read`
- `/root/test_sreg_guard_suite --reg s10 --case proto_mismatch`
- `/root/test_sreg_guard_suite --reg s11 --case legal_save_restore`

Expected result:
- `viol_write`: fault expected (non-zero exit)
- `viol_read`: fault expected (non-zero exit)
- `proto_mismatch`: fault expected (non-zero exit)
- `legal_save_restore`: success expected (zero exit)
