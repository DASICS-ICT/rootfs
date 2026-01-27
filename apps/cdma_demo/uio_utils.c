#include "uio_utils.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int find_uio_device_by_name(const char *target_name, char *out_dev, size_t len)
{
    const char *sys_uio = "/sys/class/uio";
    DIR *d = opendir(sys_uio);
    if (!d) return -1;
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (strncmp(ent->d_name, "uio", 3) != 0)
            continue;
        char name_path[300];
        snprintf(name_path, sizeof(name_path), "%s/%s/name", sys_uio, ent->d_name);
        FILE *f = fopen(name_path, "r");
        if (!f) continue;
        char buf[128];
        if (fgets(buf, sizeof(buf), f) != NULL) {
            size_t bl = strlen(buf);
            if (bl && buf[bl - 1] == '\n') buf[bl - 1] = '\0';
            if (strcmp(buf, target_name) == 0) {
                snprintf(out_dev, len, "/dev/%s", ent->d_name);
                fclose(f);
                closedir(d);
                return 0;
            }
        }
        fclose(f);
    }
    closedir(d);
    return -1;
}

const char *uio_basename_const(const char *path)
{
    const char *slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}

int uio_read_sysfs_hex(const char *path, uint64_t *value)
{
    if (!value) return -EINVAL;

    FILE *f = fopen(path, "r");
    if (!f) return -errno;

    unsigned long long temp = 0;
    int n = fscanf(f, "%llx", &temp);
    fclose(f);

    if (n != 1) return -EINVAL;

    *value = (uint64_t)temp;
    return 0;
}

int uio_map_regs(const char *uio_path, volatile uint8_t **regs_out, size_t *size_out)
{
    if (!uio_path || !regs_out || !size_out) return -EINVAL;

    char size_path[256];
    const char *uio_name = uio_basename_const(uio_path);
    snprintf(size_path, sizeof(size_path), "/sys/class/uio/%s/maps/map0/size", uio_name);

    uint64_t map_size = 0;
    int rc = uio_read_sysfs_hex(size_path, &map_size);
    if (rc) {
        fprintf(stderr, "Warning: could not read %s (%s), falling back to one page\n",
                size_path, strerror(-rc));
        map_size = (uint64_t)getpagesize();
    }

    int fd = open(uio_path, O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("open uio");
        return -errno;
    }

    void *regs = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    if (regs == MAP_FAILED) {
        perror("mmap uio");
        return -errno;
    }

    *regs_out = (volatile uint8_t *)regs;
    *size_out = (size_t)map_size;
    return 0;
}

/* MMIO access is provided via macros in uio_utils.h now. */
