# sreg-guard-tests

This rootfs app builds four standalone test programs for software-stage
`s*` register guard validation:

- `test_sreg_guard_no_protect`
- `test_sreg_guard_eager`
- `test_sreg_guard_lazy`
- `test_sreg_guard_proposed`

All binaries are installed into `rootfsimg/root/`.

`/root/test_dasics` is kept as a compatibility symlink to
`/root/test_sreg_guard_proposed`.
