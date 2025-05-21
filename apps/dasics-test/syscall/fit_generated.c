#include <stdio.h>
#include <stdlib.h>
#include <asm/unistd.h>
#include <fit.h>

int fit_init_static(void) {
    // Linker symbol declarations
    extern uint64_t __ULIBTEXT_TEST_SYSCALL_BEGIN__;
    extern uint64_t __ULIBTEXT_TEST_SYSCALL_END__;
    extern uint64_t __ULIBDATA_TEST_SYSCALL_BEGIN__;
    extern uint64_t __ULIBDATA_TEST_SYSCALL_END__;
    extern uint64_t __ULIBRODATA_TEST_SYSCALL_BEGIN__;
    extern uint64_t __ULIBRODATA_TEST_SYSCALL_END__;
    extern uint64_t __ULIBBSS_TEST_SYSCALL_BEGIN__;
    extern uint64_t __ULIBBSS_TEST_SYSCALL_END__;

    // Create "test_rwx" entry
    struct fit_entry *entry = (struct fit_entry *)malloc(sizeof(struct fit_entry));
    if (!entry) return -1;

    // Set key value
    extern void test_syscall(void);
    entry->key = (void *)test_syscall;  // Function address as key
    if (!entry->key) {
        free(entry);
        return -1;
    }

    // Calculate bitmap size and allocate
    entry->syscalls_size = (__NR_syscalls + 7) / 8;
    entry->maincalls_size = (Umaincall_UNKNOWN + 7) / 8;

    entry->syscalls = bitmap_alloc(__NR_syscalls);
    entry->maincalls = bitmap_alloc(Umaincall_UNKNOWN);

    if (!entry->syscalls || !entry->maincalls) {
        if (entry->syscalls) free(entry->syscalls);
        if (entry->maincalls) free(entry->maincalls);
        free(entry);
        return -1;
    }

    entry->syscalls[__NR_getpid / 8] |= (1 << (__NR_getpid % 8));  // Set syscall bitmap
    entry->maincalls[Umaincall_PRINT / 8] |= (1 << (Umaincall_PRINT % 8));  // Set maincall bitmap

    // Set bounds data (5 regions)
    entry->bounds_num = 5;
    entry->bounds_data = malloc(entry->bounds_num * sizeof(struct fit_bounds));
    if (!entry->bounds_data) {
        free(entry->syscalls);
        free(entry->maincalls);
        free(entry);
        return -1;
    }

    // Fill bounds data - using DASICS_LIBCFG_XX permissions
    // 1. Code segment (executable)
    entry->bounds_data[0].perm = DASICS_LIBCFG_X;
    entry->bounds_data[0].lo = (uint64_t)&__ULIBTEXT_TEST_SYSCALL_BEGIN__;
    entry->bounds_data[0].hi = (uint64_t)&__ULIBTEXT_TEST_SYSCALL_END__;

    // 2. Data segment (read-write)
    entry->bounds_data[1].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
    entry->bounds_data[1].lo = (uint64_t)&__ULIBDATA_TEST_SYSCALL_BEGIN__;
    entry->bounds_data[1].hi = (uint64_t)&__ULIBDATA_TEST_SYSCALL_END__;

    // 3. Read-only data segment (read-only)
    entry->bounds_data[2].perm = DASICS_LIBCFG_R;
    entry->bounds_data[2].lo = (uint64_t)&__ULIBRODATA_TEST_SYSCALL_BEGIN__;
    entry->bounds_data[2].hi = (uint64_t)&__ULIBRODATA_TEST_SYSCALL_END__;

    // 4. BSS segment (read-write)
    entry->bounds_data[3].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
    entry->bounds_data[3].lo = (uint64_t)&__ULIBBSS_TEST_SYSCALL_BEGIN__;
    entry->bounds_data[3].hi = (uint64_t)&__ULIBBSS_TEST_SYSCALL_END__;

    // 5. Stack frame (read-write)
    entry->bounds_data[4].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
    entry->bounds_data[4].lo = UINT64_MAX;  // Maximum uint64_t value indicates stack frame
    entry->bounds_data[4].hi = 144;

    // Add to hash table (UTHash operation)
    HASH_ADD_PTR(fit_table, key, entry);

    return 0;
}