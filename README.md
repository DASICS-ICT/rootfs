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

The default `base` profile builds a minimal image and does not auto-run DASICS
tests:

```bash
make all
```

Select an explicit profile for other images:

```bash
# Current C6/D3/D4/E1/E5/F1 kernel-runtime regression modules.
make PROFILE=runtime-regression all

# Hand-written KSplit-IDL-derived dummy logic/glue modules.
make PROFILE=dummy all

# Historical user-space DASICS programs.
make PROFILE=legacy all
```

The runtime modules are copied into
`/root/modules/dasics/runtime/` in the guest. They are regular initramfs files,
not absolute host symlinks. Intermediate phase sources such as A2/C3/C4/C5/D1
remain buildable directly but are not packaged by a standard profile.

No profile loads a test automatically from `rc.local`; run the desired
`insmod` commands explicitly so a boot transcript has an unambiguous test
scope.

For the dummy profile, load trusted glue before untrusted logic:

```sh
insmod /root/modules/dasics/dummy/dummy_glue.ko trust=1
insmod /root/modules/dasics/dummy/dummy_logic.ko
ip link show dummy0
rmmod dummy_logic
rmmod dummy_glue
```

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
