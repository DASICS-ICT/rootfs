#!/usr/bin/env python3
import json
import struct
import argparse
import os
import subprocess
import tempfile

def process_json_to_elf(json_data, elf_file):
    temp_files = []

    try:
        for key, value in json_data.items():
            if isinstance(value, list):
                for item in value:
                    if 'syscalls' in item:
                        syscalls = item['syscalls']
                        syscall_numbers = [int(sc['syscall_number']) for sc in syscalls]
                        section_name = f".syscalls.{key}"
                        temp_file = create_temp_bin(syscall_numbers)
                        temp_files.append((temp_file, section_name))

                    if 'maincalls' in item:
                        maincalls = item['maincalls']
                        maincall_numbers = [int(mc['maincall_number']) for mc in maincalls]
                        section_name = f".maincalls.{key}"
                        temp_file = create_temp_bin(maincall_numbers)
                        temp_files.append((temp_file, section_name))

        for temp_file, section_name in temp_files:
            embed_section_to_elf(elf_file, section_name, temp_file)

    finally:
        # Clean up temp files
        for temp_file, _ in temp_files:
            if os.path.exists(temp_file):
                os.unlink(temp_file)

def create_temp_bin(numbers):
    count = len(numbers)

    fd, temp_file = tempfile.mkstemp(suffix='.bin')
    with os.fdopen(fd, 'wb') as f:
        # 使用2个字节(H)存储数量
        f.write(struct.pack('H', count))

        # 每个条目用2个字节(H)存储
        for num in numbers:
            f.write(struct.pack('H', num))

    return temp_file

def embed_section_to_elf(elf_file, section_name, data_file):
    # Get file size for symbol value
    file_size = os.path.getsize(data_file)

    # 1. First add the section
    cmd_add_section = [
        "riscv64-unknown-linux-gnu-objcopy",
        "--add-section",
        f"{section_name}={data_file}",
        elf_file  # 直接修改原文件
    ]
    subprocess.run(cmd_add_section, check=True)

    # 2. Add start symbol
    start_symbol = f"__{section_name.replace('.', '_')}_start"
    cmd_add_start = [
        "riscv64-unknown-linux-gnu-objcopy",
        "--add-symbol",
        f"{start_symbol}={section_name}:0,global",
        elf_file  # 直接修改原文件
    ]
    subprocess.run(cmd_add_start, check=True)

    # 3. Add end symbol
    end_symbol = f"__{section_name.replace('.', '_')}_end"
    cmd_add_end = [
        "riscv64-unknown-linux-gnu-objcopy",
        "--add-symbol",
        f"{end_symbol}={section_name}:{file_size},global",
        elf_file  # 直接修改原文件
    ]
    subprocess.run(cmd_add_end, check=True)

    print(f"Added to {elf_file}:")
    print(f"  Section: {section_name}")
    print(f"  Symbols: {start_symbol}, {end_symbol}")


def main():
    parser = argparse.ArgumentParser(description='Embed syscall/maincall data as sections in ELF file')
    parser.add_argument('-i', '--input', required=True, help='Input JSON file path')
    parser.add_argument('-e', '--elf', required=True, help='ELF file to embed sections into')

    args = parser.parse_args()

    try:
        with open(args.input, 'r') as f:
            json_data = json.load(f)

        process_json_to_elf(json_data, args.elf)
        print("Operation completed successfully!")
    except Exception as e:
        print(f"Error: {str(e)}")
        raise

if __name__ == "__main__":
    main()
