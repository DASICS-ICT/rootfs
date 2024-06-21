# Rootfs

## Introduction

This repository automates the process of setting up and managing a root filesystem (`rootfs`) with the necessary applications.

## Prerequisites

First, set the environment variables `RISCV` and `RISCV_ROOTFS_HOME`:

- `RISCV`: The top directory of your RISC-V toolchain binaries, such as `/opt/riscv`
- `RISCV_ROOTFS_HOME`: The top directory of this cloned repository

## Usage

### Initialization

Before building the `rootfs`, initialize the required git submodules:

```bash
make init
```

This command updates and initializes the submodules with a shallow clone (`--depth 1`).

### Building the Rootfs

To build the `rootfs`, run:

```bash
make all
```

This target compiles and installs the specified applications (in this case, `busybox`) and generates the initial RAM filesystem (`initramfs`) using a Python script located in the `utils` directory.

### Cleaning the Rootfs

To clean the build artifacts and directories, use:

```bash
make clean
```

This command cleans up the build directories of the specified applications and removes any generated `initramfs` files from the `rootfsimg` directory.

### Deinitialization

To deinitialize and remove all git submodules, run:

```bash
make deinit
```

This command forcefully deinitializes all submodules, which is useful when you are about to switch git branches.