#ifndef UIO_UTILS_H
#define UIO_UTILS_H

#include <stdint.h>
#include <stddef.h>

int find_uio_device_by_name(const char *target_name, char *out_dev, size_t len);
const char *uio_basename_const(const char *path);
int uio_read_sysfs_hex(const char *path, uint64_t *value);
int uio_map_regs(const char *uio_path, volatile uint8_t **regs_out, size_t *size_out);

uint32_t mmio_read32(const volatile void *base, size_t offset);
void mmio_write32(volatile void *base, size_t offset, uint32_t value);
uint64_t mmio_read64_lo_hi(const volatile void *base, size_t offset);
void mmio_write64_lo_hi(volatile void *base, size_t offset, uint64_t value);

#endif /* UIO_UTILS_H */
