import os
import subprocess
import sys
import datetime
import fnmatch

def write_timestamp_to_file(f):
    current_time = datetime.datetime.now()
    timestamp = (
        "##################################################\n"
        "#\n"
        "# Automatically generated at "
        f"{current_time.strftime('%Y-%m-%d %H:%M:%S')}\n"
        "#\n"
        "##################################################\n\n"
    )
    f.write(timestamp)

def find_dependencies(executable, sysroot, rootfsimg):
    try:
        readelf_output = subprocess.check_output(['readelf', '-d', executable]).decode('utf-8')
        dependencies = []
        for line in readelf_output.splitlines():
            if '(NEEDED)' in line:
                lib_name = line.split('[')[1].split(']')[0]
                lib_path = find_library_path(lib_name, sysroot, rootfsimg)
                if lib_path:
                    dependencies.append(lib_path)
                else:
                    raise FileNotFoundError(f"Dependency {lib_name} not found in sysroot or rootfsimg")
        return dependencies
    except subprocess.CalledProcessError:
        return []

def find_library_path(lib_name, sysroot, rootfsimg):
    for lib_dir in ['lib', 'usr/lib']:
        lib_path = os.path.join(sysroot, lib_dir, lib_name)
        if os.path.exists(lib_path):
            return lib_path
        lib_path = os.path.join(rootfsimg, lib_dir, lib_name)
        if os.path.exists(lib_path):
            return lib_path
    return None

def link_dependencies_to_rootfsimg(rootfsimg_path, sysroot_path, f):
    # Create default link for ld-linux-riscv64-lp64d.so.1
    ld_linux_src = os.path.join(sysroot_path, 'lib', 'ld-linux-riscv64-lp64d.so.1')
    ld_linux_dst = os.path.join(rootfsimg_path, 'lib', 'ld-linux-riscv64-lp64d.so.1')
    if not os.path.exists(ld_linux_dst):
        os.symlink(ld_linux_src, ld_linux_dst)

    for subdir in ['bin', 'sbin', 'usr/bin', 'usr/sbin', 'root', 'lib', 'usr/lib']:
        full_dir_path = os.path.join(rootfsimg_path, subdir)
        if os.path.exists(full_dir_path):
            for file in os.listdir(full_dir_path):
                file_path = os.path.join(full_dir_path, file)
                if os.path.isfile(file_path) and (os.access(file_path, os.X_OK) or file_path.split('.')[1] == 'so'):
                    dependencies = find_dependencies(file_path, sysroot_path, rootfsimg_path)
                    for dep in dependencies:
                        if dep.startswith(sysroot_path):
                            relative_dep_path = os.path.relpath(dep, rootfsimg_path).replace(os.sep, '/')
                            if '/usr/lib' in relative_dep_path:
                                link_dir = '/usr/lib'
                                target_dir = os.path.join(rootfsimg_path, 'usr/lib')
                            else:
                                link_dir = '/lib'
                                target_dir = os.path.join(rootfsimg_path, 'lib')

                            link_path = os.path.join(target_dir, os.path.basename(dep))
                            if not os.path.exists(link_path):
                                os.symlink(dep, link_path)

def import_dirs_to_initramfs(rootfsimg_path, f):
    f.write("# Create directories\n")
    for root, dirs, _ in os.walk(rootfsimg_path):
        for dir in dirs:
            dir_path = os.path.join(root, dir)
            relative_path = os.path.relpath(dir_path, rootfsimg_path).replace(os.sep, '/')
            f.write(f"dir /{relative_path} 755 0 0\n")
    f.write("\n")

def import_files_to_initramfs(rootfsimg_path, f):
    f.write("# Import initramfs files\n")
    for root, _, files in os.walk(rootfsimg_path):
        for file in files:
            if fnmatch.fnmatch(file, 'initramfs*.txt') or fnmatch.fnmatch(file, '.git*'):
                continue
            file_path = os.path.join(root, file)
            relative_path = os.path.relpath(file_path, rootfsimg_path).replace(os.sep, '/')
            env_file_path = os.sep.join(["${RISCV_ROOTFS_HOME}", "rootfsimg", relative_path])
            f.write(f"file /{relative_path} {env_file_path} 755 0 0\n")

    # Add slink for busybox
    f.write(f"slink /init /bin/busybox 755 0 0\n")

def write_device_nodes(f):
    f.write("# Create device nodes\n")
    f.writelines(["nod /dev/console 644 0 0 c 5 1\n", \
                  "nod /dev/null 644 0 0 c 1 3\n\n"])

def generate_initramfs_txt(rootfsimg_path, sysroot_path, output_file):
    with open(output_file, 'w') as f:
        # Write timestamp
        write_timestamp_to_file(f)

        # Import directories into initramfs
        import_dirs_to_initramfs(rootfsimg_path, f)

        # Write device information
        write_device_nodes(f)

        # Process dependencies and generate links
        link_dependencies_to_rootfsimg(rootfsimg_path, sysroot_path, f)

        # Import files into initramfs
        import_files_to_initramfs(rootfsimg_path, f)

if __name__ == "__main__":
    # Check environment variables
    if not os.getenv("RISCV_ROOTFS_HOME") or not os.getenv("RISCV"):
        print("Error: Environment variables RISCV_ROOTFS_HOME and RISCV must be set!")
        sys.exit(1)

    # Initialize paths
    rootfsimg_path = os.path.join(os.getenv("RISCV_ROOTFS_HOME"), "rootfsimg")
    sysroot_path = os.path.join(os.getenv("RISCV"), "sysroot")
    output_file = os.path.join(os.getenv("RISCV_ROOTFS_HOME"), "rootfsimg", "initramfs.txt")

    # Start to generate initramfs.txt
    generate_initramfs_txt(rootfsimg_path, sysroot_path, output_file)
    print(f"initramfs.txt has been generated successfully at {output_file}")
