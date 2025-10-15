#!/bin/bash
#
# Build script for Zicfilp test programs in rootfs
#
# Usage: ./build-zicfilp.sh

set -e

# Set environment variables
export PATH=/data/share/toolchain/riscv64-toolchain-10/bin:$PATH
export RISCV=/data/share/toolchain/riscv64-toolchain-10
export RISCV_ROOTFS_HOME=$(pwd)
export CROSS_COMPILE=riscv64-unknown-linux-gnu-

echo "========================================"
echo "Building Zicfilp Test Programs"
echo "========================================"
echo "Toolchain: $RISCV"
echo "Cross-compile prefix: $CROSS_COMPILE"
echo ""

# Build and install zicfilp-test
echo "Building zicfilp-test..."
make -C apps/zicfilp-test install

echo ""
echo "========================================"
echo "Build completed successfully!"
echo "========================================"
echo ""
echo "Test programs installed to:"
echo "  rootfsimg/root/test_prctl"
echo "  rootfsimg/root/test_zicfilp_correct"
echo ""
echo "Next steps:"
echo "  1. Build full rootfs: make all"
echo "  2. Copy initramfs to kernel: cp rootfsimg/initramfs-*.txt /path/to/linux/"
echo "  3. Rebuild Linux kernel with initramfs"
echo "  4. Boot on NEMU and run tests"
echo ""
