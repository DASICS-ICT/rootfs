#include <stdio.h>
#include <stdlib.h>
#include <asm/unistd.h>
#include <fit.h>

int fit_init_static(void) {
    // Linker symbol declarations
    extern uint64_t __ULIBTEXT_TEST_RWX1_BEGIN__;
    extern uint64_t __ULIBTEXT_TEST_RWX1_END__;
    extern uint64_t __ULIBTEXT_TEST_RWX2_BEGIN__;
    extern uint64_t __ULIBTEXT_TEST_RWX2_END__;
    extern uint64_t __ULIBTEXT_SHARE_BEGIN__;
    extern uint64_t __ULIBTEXT_SHARE_END__;
    extern uint64_t __ULIBDATA_TEST_RWX1_BEGIN__;
    extern uint64_t __ULIBDATA_TEST_RWX1_END__;
    extern uint64_t __ULIBDATA_TEST_RWX2_BEGIN__;
    extern uint64_t __ULIBDATA_TEST_RWX2_END__;
    extern uint64_t __ULIBDATA_SHARE_BEGIN__;
    extern uint64_t __ULIBDATA_SHARE_END__;
    extern uint64_t __ULIBRODATA_TEST_RWX1_BEGIN__;
    extern uint64_t __ULIBRODATA_TEST_RWX1_END__;
    extern uint64_t __ULIBRODATA_TEST_RWX2_BEGIN__;
    extern uint64_t __ULIBRODATA_TEST_RWX2_END__;
    extern uint64_t __ULIBRODATA_SHARE_BEGIN__;
    extern uint64_t __ULIBRODATA_SHARE_END__;
    extern uint64_t __ULIBBSS_TEST_RWX1_BEGIN__;
    extern uint64_t __ULIBBSS_TEST_RWX1_END__;
    extern uint64_t __ULIBBSS_TEST_RWX2_BEGIN__;
    extern uint64_t __ULIBBSS_TEST_RWX2_END__;
    extern uint64_t __ULIBBSS_SHARE_BEGIN__;
    extern uint64_t __ULIBBSS_SHARE_END__;

    // Create "test_rwx1" entry
    struct fit_entry *entry_test_rwx1 = (struct fit_entry *)malloc(sizeof(struct fit_entry));
    if (!entry_test_rwx1) return -1;

    // Set key value
    extern void test_rwx1(void);
    entry_test_rwx1->key = (void *)test_rwx1;  // Function address as key
    if (!entry_test_rwx1->key) {
        free(entry_test_rwx1);
        return -1;
    }

    // Set argument bound allocation and deallocation functions
    entry_test_rwx1->argbound_alloc = NULL;
    entry_test_rwx1->argbound_free = NULL;
    entry_test_rwx1->library_id = 0;
    entry_test_rwx1->closure_id = 1;
    entry_test_rwx1->heap_alloc_done = 0;

    // Calculate bitmap size and allocate
    entry_test_rwx1->syscalls_size = (__NR_syscalls + 7) / 8;
    entry_test_rwx1->maincalls_size = (Umaincall_UNKNOWN + 7) / 8;

    entry_test_rwx1->syscalls = bitmap_alloc(__NR_syscalls);
    entry_test_rwx1->maincalls = bitmap_alloc(Umaincall_UNKNOWN);

    if (!entry_test_rwx1->syscalls || !entry_test_rwx1->maincalls) {
        if (entry_test_rwx1->syscalls) free(entry_test_rwx1->syscalls);
        if (entry_test_rwx1->maincalls) free(entry_test_rwx1->maincalls);
        free(entry_test_rwx1);
        return -1;
    }

    entry_test_rwx1->maincalls[Umaincall_PRINT / 8] |= (1 << (Umaincall_PRINT % 8));  // Set maincall bitmap

    // Set bounds data (9 regions)
    entry_test_rwx1->bounds_num = 9;
    entry_test_rwx1->bounds_data = malloc(entry_test_rwx1->bounds_num * sizeof(struct fit_bounds));
    if (!entry_test_rwx1->bounds_data) {
        free(entry_test_rwx1->syscalls);
        free(entry_test_rwx1->maincalls);
        free(entry_test_rwx1);
        return -1;
    }

    // Fill bounds data - using DASICS_LIBCFG_XX permissions
    // 1. Code segment of test_rwx1 (executable)
    entry_test_rwx1->bounds_data[0].perm = DASICS_LIBCFG_X;
    entry_test_rwx1->bounds_data[0].lo = (uint64_t)&__ULIBTEXT_TEST_RWX1_BEGIN__;
    entry_test_rwx1->bounds_data[0].hi = (uint64_t)&__ULIBTEXT_TEST_RWX1_END__;

    // 2. Data segment of test_rwx1 (read-write)
    entry_test_rwx1->bounds_data[1].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
    entry_test_rwx1->bounds_data[1].lo = (uint64_t)&__ULIBDATA_TEST_RWX1_BEGIN__;
    entry_test_rwx1->bounds_data[1].hi = (uint64_t)&__ULIBDATA_TEST_RWX1_END__;

    // 3. Read-only data segment of test_rwx1 (read-only)
    entry_test_rwx1->bounds_data[2].perm = DASICS_LIBCFG_R;
    entry_test_rwx1->bounds_data[2].lo = (uint64_t)&__ULIBRODATA_TEST_RWX1_BEGIN__;
    entry_test_rwx1->bounds_data[2].hi = (uint64_t)&__ULIBRODATA_TEST_RWX1_END__;

    // 4. BSS segment of test_rwx1 (read-write)
    entry_test_rwx1->bounds_data[3].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
    entry_test_rwx1->bounds_data[3].lo = (uint64_t)&__ULIBBSS_TEST_RWX1_BEGIN__;
    entry_test_rwx1->bounds_data[3].hi = (uint64_t)&__ULIBBSS_TEST_RWX1_END__;

    // 5. Code segment of share (executable)
    entry_test_rwx1->bounds_data[4].perm = DASICS_LIBCFG_X;
    entry_test_rwx1->bounds_data[4].lo = (uint64_t)&__ULIBTEXT_SHARE_BEGIN__;
    entry_test_rwx1->bounds_data[4].hi = (uint64_t)&__ULIBTEXT_SHARE_END__;

    // 6. Data segment of share (read-write)
    entry_test_rwx1->bounds_data[5].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
    entry_test_rwx1->bounds_data[5].lo = (uint64_t)&__ULIBDATA_SHARE_BEGIN__;
    entry_test_rwx1->bounds_data[5].hi = (uint64_t)&__ULIBDATA_SHARE_END__;

    // 7. Read-only data segment of share (read-only)
    entry_test_rwx1->bounds_data[6].perm = DASICS_LIBCFG_R;
    entry_test_rwx1->bounds_data[6].lo = (uint64_t)&__ULIBRODATA_SHARE_BEGIN__;
    entry_test_rwx1->bounds_data[6].hi = (uint64_t)&__ULIBRODATA_SHARE_END__;

    // 8. BSS segment of share (read-write)
    entry_test_rwx1->bounds_data[7].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
    entry_test_rwx1->bounds_data[7].lo = (uint64_t)&__ULIBBSS_SHARE_BEGIN__;
    entry_test_rwx1->bounds_data[7].hi = (uint64_t)&__ULIBBSS_SHARE_END__;

    // 9. Stack frame (read-write)
    entry_test_rwx1->bounds_data[8].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
    entry_test_rwx1->bounds_data[8].lo = UINT64_MAX;  // Maximum uint64_t value indicates stack frame
    entry_test_rwx1->bounds_data[8].hi = 96;

    // Add to hash table (UTHash operation)
    HASH_ADD_PTR(fit_table, key, entry_test_rwx1);

    // Create "test_rwx2" entry
    struct fit_entry *entry_test_rwx2 = (struct fit_entry *)malloc(sizeof(struct fit_entry));
    if (!entry_test_rwx2) return -1;

    // Set key value
    extern void test_rwx2(void);
    entry_test_rwx2->key = (void *)test_rwx2;  // Function address as key
    if (!entry_test_rwx2->key) {
        free(entry_test_rwx2);
        return -1;
    }

    // Set argument bound allocation and deallocation functions
    entry_test_rwx2->argbound_alloc = NULL;
    entry_test_rwx2->argbound_free = NULL;
    entry_test_rwx2->library_id = 0;
    entry_test_rwx2->closure_id = 2;
    entry_test_rwx2->heap_alloc_done = 0;

    // Calculate bitmap size and allocate
    entry_test_rwx2->syscalls_size = (__NR_syscalls + 7) / 8;
    entry_test_rwx2->maincalls_size = (Umaincall_UNKNOWN + 7) / 8;

    entry_test_rwx2->syscalls = bitmap_alloc(__NR_syscalls);
    entry_test_rwx2->maincalls = bitmap_alloc(Umaincall_UNKNOWN);

    if (!entry_test_rwx2->syscalls || !entry_test_rwx2->maincalls) {
        if (entry_test_rwx2->syscalls) free(entry_test_rwx2->syscalls);
        if (entry_test_rwx2->maincalls) free(entry_test_rwx2->maincalls);
        free(entry_test_rwx2);
        return -1;
    }

    entry_test_rwx2->maincalls[Umaincall_PRINT / 8] |= (1 << (Umaincall_PRINT % 8));  // Set maincall bitmap

    // Set bounds data (9 regions)
    entry_test_rwx2->bounds_num = 9;
    entry_test_rwx2->bounds_data = malloc(entry_test_rwx2->bounds_num * sizeof(struct fit_bounds));
    if (!entry_test_rwx2->bounds_data) {
        free(entry_test_rwx2->syscalls);
        free(entry_test_rwx2->maincalls);
        free(entry_test_rwx2);
        return -1;
    }

    // Fill bounds data - using DASICS_LIBCFG_XX permissions
    // 1. Code segment of test_rwx2 (executable)
    entry_test_rwx2->bounds_data[0].perm = DASICS_LIBCFG_X;
    entry_test_rwx2->bounds_data[0].lo = (uint64_t)&__ULIBTEXT_TEST_RWX2_BEGIN__;
    entry_test_rwx2->bounds_data[0].hi = (uint64_t)&__ULIBTEXT_TEST_RWX2_END__;

    // 2. Data segment of test_rwx2 (read-write)
    entry_test_rwx2->bounds_data[1].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
    entry_test_rwx2->bounds_data[1].lo = (uint64_t)&__ULIBDATA_TEST_RWX2_BEGIN__;
    entry_test_rwx2->bounds_data[1].hi = (uint64_t)&__ULIBDATA_TEST_RWX2_END__;

    // 3. Read-only data segment of test_rwx2 (read-only)
    entry_test_rwx2->bounds_data[2].perm = DASICS_LIBCFG_R;
    entry_test_rwx2->bounds_data[2].lo = (uint64_t)&__ULIBRODATA_TEST_RWX2_BEGIN__;
    entry_test_rwx2->bounds_data[2].hi = (uint64_t)&__ULIBRODATA_TEST_RWX2_END__;

    // 4. BSS segment of test_rwx2 (read-write)
    entry_test_rwx2->bounds_data[3].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
    entry_test_rwx2->bounds_data[3].lo = (uint64_t)&__ULIBBSS_TEST_RWX2_BEGIN__;
    entry_test_rwx2->bounds_data[3].hi = (uint64_t)&__ULIBBSS_TEST_RWX2_END__;

    // 5. Code segment of share (executable)
    entry_test_rwx2->bounds_data[4].perm = DASICS_LIBCFG_X;
    entry_test_rwx2->bounds_data[4].lo = (uint64_t)&__ULIBTEXT_SHARE_BEGIN__;
    entry_test_rwx2->bounds_data[4].hi = (uint64_t)&__ULIBTEXT_SHARE_END__;

    // 6. Data segment of share (read-write)
    entry_test_rwx2->bounds_data[5].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
    entry_test_rwx2->bounds_data[5].lo = (uint64_t)&__ULIBDATA_SHARE_BEGIN__;
    entry_test_rwx2->bounds_data[5].hi = (uint64_t)&__ULIBDATA_SHARE_END__;

    // 7. Read-only data segment of share (read-only)
    entry_test_rwx2->bounds_data[6].perm = DASICS_LIBCFG_R;
    entry_test_rwx2->bounds_data[6].lo = (uint64_t)&__ULIBRODATA_SHARE_BEGIN__;
    entry_test_rwx2->bounds_data[6].hi = (uint64_t)&__ULIBRODATA_SHARE_END__;

    // 8. BSS segment of share (read-write)
    entry_test_rwx2->bounds_data[7].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
    entry_test_rwx2->bounds_data[7].lo = (uint64_t)&__ULIBBSS_SHARE_BEGIN__;
    entry_test_rwx2->bounds_data[7].hi = (uint64_t)&__ULIBBSS_SHARE_END__;

    // 9. Stack frame (read-write)
    entry_test_rwx2->bounds_data[8].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
    entry_test_rwx2->bounds_data[8].lo = UINT64_MAX;  // Maximum uint64_t value indicates stack frame
    entry_test_rwx2->bounds_data[8].hi = 96;

    // Add to hash table (UTHash operation)
    HASH_ADD_PTR(fit_table, key, entry_test_rwx2);

    return 0;
}