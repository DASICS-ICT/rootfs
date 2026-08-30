#!/usr/bin/env bash

set -euo pipefail

tests_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
test_script="$tests_dir/test-gen-initramfs-list.sh"
generator="$tests_dir/../gen-initramfs-list.sh"

bash -n "$generator" "$test_script"
shellcheck "$generator" "$test_script"
"$test_script"

printf 'ROOTFS_SCRIPT_TESTS=PASS\n'
