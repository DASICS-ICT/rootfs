#ifndef UIO_UTILS_H
#define UIO_UTILS_H

#include <stdint.h>
#include <stddef.h>

int find_uio_device_by_name(const char *target_name, char *out_dev, size_t len);
const char *uio_basename_const(const char *path);
int uio_read_sysfs_hex(const char *path, uint64_t *value);
int uio_map_regs(const char *uio_path, volatile uint8_t **regs_out, size_t *size_out);

/* MMIO access macros: use direct volatile accesses for performance and simplicity. */
#define mmio_read32(base, offset) \
	(*(const volatile uint32_t *)((const volatile uint8_t *)(base) + (offset)))

#define mmio_write32(base, offset, value) \
	(*(volatile uint32_t *)((volatile uint8_t *)(base) + (offset)) = (value))

#define mmio_read64_lo_hi(base, offset) ({ \
	const volatile uint32_t *__plo = (const volatile uint32_t *)((const volatile uint8_t *)(base) + (offset)); \
	const volatile uint32_t *__phi = (const volatile uint32_t *)((const volatile uint8_t *)(base) + (offset) + 4); \
	((uint64_t)(*__phi) << 32) | (uint64_t)(*__plo); \
})

#define mmio_write64_lo_hi(base, offset, value) do { \
	volatile uint32_t *__plo = (volatile uint32_t *)((volatile uint8_t *)(base) + (offset)); \
	volatile uint32_t *__phi = (volatile uint32_t *)((volatile uint8_t *)(base) + (offset) + 4); \
	*__plo = (uint32_t)((value) & 0xFFFFFFFFULL); \
	*__phi = (uint32_t)(((value) >> 32) & 0xFFFFFFFFULL); \
} while (0)

#endif /* UIO_UTILS_H */
