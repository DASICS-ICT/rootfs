#!/usr/bin/env bash

set -euo pipefail

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
generator=$(realpath -- "$script_dir/../gen-initramfs-list.sh")
test_root=$(mktemp -d)

cleanup() {
  local status=$?

  trap - EXIT
  chmod -R u+w -- "$test_root" 2>/dev/null || true
  rm -rf -- "$test_root"
  exit "$status"
}

trap cleanup EXIT

riscv_root="$test_root/riscv"
initramfs_root="$test_root/initramfs"
output="$test_root/initramfs.txt"
private_root="$initramfs_root/my-dir/register-pressure/private-workloads/milc_scalar"

mkdir -p -- "$riscv_root/lib" "$private_root"
for library in \
  ld-linux-riscv64-lp64d.so.1 \
  libc.so.6 libdl.so.2 libm.so.6 libpthread.so.0 libresolv.so.2; do
  : >"$riscv_root/lib/$library"
done
printf 'program\n' >"$private_root/program"
printf 'data\n' >"$private_root/data"
chmod 0755 -- "$private_root/program"
chmod 0444 -- "$private_root/data"

RISCV="$riscv_root" INITRAMFS_ROOT="$initramfs_root" \
  "$generator" "$output"

program_line="file /my-dir/register-pressure/private-workloads/milc_scalar/program $private_root/program 755 0 0"
data_line="file /my-dir/register-pressure/private-workloads/milc_scalar/data $private_root/data 444 0 0"
[[ "$(grep -Fxc -- "$program_line" "$output")" -eq 1 ]] || {
  printf 'Expected one mode-preserving program entry\n' >&2
  exit 1
}
[[ "$(grep -Fxc -- "$data_line" "$output")" -eq 1 ]] || {
  printf 'Expected one mode-preserving data entry\n' >&2
  exit 1
}

printf 'ROOTFS_INITRAMFS_LIST_TEST=PASS program_mode=755 data_mode=444\n'
