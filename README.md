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

### Build Type

The `BUILD_TYPE` variable controls the optimization and debug settings. It defaults to `Release`.

```bash
# Release build (default): -O2, no debug symbols
make all

# Debug build: -O0 -g, with debug symbols
make all BUILD_TYPE=Debug
```

In Debug mode:

- Applications using `Makefile.compile` are compiled with `-O0 -g` instead of `-O2`.
- Applications with their own build systems (e.g., Busybox) will also be built with debug symbols. The exact flags or configuration may vary per application — see each application's `Makefile` for details.

### Cleaning the Rootfs

To clean the build artifacts and directories, use:

```bash
make clean
```

This command cleans up the build directories of the specified applications and removes any generated `initramfs` files from the `rootfsimg` directory.

### Cleaning submodules

To remove all generated files inside submodules, run:

```bash
make repoclean
```

This command not only initiates a `make clean` operation but also extends its reach to clean up any generated files residing within the submodules.

### Deinitialization

To deinitialize and remove all git submodules, run:

```bash
make distclean
```

This command forcefully deinitializes all submodules, which is useful when you are about to switch git branches.

### Generating initramfs manually

To manually generate initramfs.txt, use:

```bash
make initramfs
```

This command is particularly useful when you need to modify or update the rootfsimg either after `make all` or independently of it.