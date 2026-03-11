#include <stdio.h>
#include <stdlib.h>
#include <asm/unistd.h>
#include <fit.h>

/* Only add a bound when begin and end symbols differ (non-empty region) */
#define ADD_CODE_BOUND(e, perm_bits, begin, end) do { \
    if ((uint64_t)&(begin) != (uint64_t)&(end) && (e)->code_bounds_num < FIT_CODE_BOUNDS_MAX) { \
        (e)->code_bounds[(e)->code_bounds_num].perm = (perm_bits); \
        (e)->code_bounds[(e)->code_bounds_num].lo = (uint64_t)&(begin); \
        (e)->code_bounds[(e)->code_bounds_num].hi = (uint64_t)&(end); \
        (e)->code_bounds_num++; \
    } \
} while (0)
#define ADD_MEM_BOUND(e, perm_bits, begin, end) do { \
    if ((uint64_t)&(begin) != (uint64_t)&(end) && (e)->mem_bounds_num < FIT_MEM_BOUNDS_MAX) { \
        (e)->mem_bounds[(e)->mem_bounds_num].perm = (perm_bits); \
        (e)->mem_bounds[(e)->mem_bounds_num].lo = (uint64_t)&(begin); \
        (e)->mem_bounds[(e)->mem_bounds_num].hi = (uint64_t)&(end); \
        (e)->mem_bounds_num++; \
    } \
} while (0)

int fit_init_static(void) {
    // Linker symbol declarations
    extern uint64_t __ULIBTEXT_TEST_RWX1_BEGIN__, __ULIBTEXT_TEST_RWX1_END__;
    extern uint64_t __ULIBTEXT_TEST_RWX2_BEGIN__, __ULIBTEXT_TEST_RWX2_END__;
    extern uint64_t __ULIBTEXT_SHARE_BEGIN__, __ULIBTEXT_SHARE_END__;
    extern uint64_t __ULIBDATA_TEST_RWX1_BEGIN__, __ULIBDATA_TEST_RWX1_END__;
    extern uint64_t __ULIBDATA_TEST_RWX2_BEGIN__, __ULIBDATA_TEST_RWX2_END__;
    extern uint64_t __ULIBDATA_SHARE_BEGIN__, __ULIBDATA_SHARE_END__;
    extern uint64_t __ULIBRODATA_TEST_RWX1_BEGIN__, __ULIBRODATA_TEST_RWX1_END__;
    extern uint64_t __ULIBRODATA_TEST_RWX2_BEGIN__, __ULIBRODATA_TEST_RWX2_END__;
    extern uint64_t __ULIBRODATA_SHARE_BEGIN__, __ULIBRODATA_SHARE_END__;
    extern uint64_t __ULIBBSS_TEST_RWX1_BEGIN__, __ULIBBSS_TEST_RWX1_END__;
    extern uint64_t __ULIBBSS_TEST_RWX2_BEGIN__, __ULIBBSS_TEST_RWX2_END__;
    extern uint64_t __ULIBBSS_SHARE_BEGIN__, __ULIBBSS_SHARE_END__;

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

    // Set library and closure IDs
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

    // Initialize code and memory bounds
    entry_test_rwx1->code_bounds_num = 0;
    entry_test_rwx1->mem_bounds_num = 0;
    entry_test_rwx1->temp_code_bounds_num = 0;
    entry_test_rwx1->temp_mem_bounds_num = 0;
    entry_test_rwx1->temp_times = 0;

    // Fill bounds data - using DASICS_LIBCFG_XX permissions
    // 1. Code segment (executable)
    ADD_CODE_BOUND(entry_test_rwx1, DASICS_LIBCFG_X, __ULIBTEXT_TEST_RWX1_BEGIN__, __ULIBTEXT_TEST_RWX1_END__);
    // 2. Data segment (read-write)
    ADD_MEM_BOUND(entry_test_rwx1, DASICS_LIBCFG_R | DASICS_LIBCFG_W, __ULIBDATA_TEST_RWX1_BEGIN__, __ULIBDATA_TEST_RWX1_END__);
    // 3. Read-only data segment (read-only)
    ADD_MEM_BOUND(entry_test_rwx1, DASICS_LIBCFG_R, __ULIBRODATA_TEST_RWX1_BEGIN__, __ULIBRODATA_TEST_RWX1_END__);
    // 4. BSS segment (read-write)
    ADD_MEM_BOUND(entry_test_rwx1, DASICS_LIBCFG_R | DASICS_LIBCFG_W, __ULIBBSS_TEST_RWX1_BEGIN__, __ULIBBSS_TEST_RWX1_END__);
    // 5. Shared code segment (executable)
    ADD_CODE_BOUND(entry_test_rwx1, DASICS_LIBCFG_X, __ULIBTEXT_SHARE_BEGIN__, __ULIBTEXT_SHARE_END__);
    // 6. Shared data segment (read-write)
    ADD_MEM_BOUND(entry_test_rwx1, DASICS_LIBCFG_R | DASICS_LIBCFG_W, __ULIBDATA_SHARE_BEGIN__, __ULIBDATA_SHARE_END__);
    // 7. Shared read-only data segment (read-only)
    ADD_MEM_BOUND(entry_test_rwx1, DASICS_LIBCFG_R, __ULIBRODATA_SHARE_BEGIN__, __ULIBRODATA_SHARE_END__);
    // 8. Shared BSS segment (read-write)
    ADD_MEM_BOUND(entry_test_rwx1, DASICS_LIBCFG_R | DASICS_LIBCFG_W, __ULIBBSS_SHARE_BEGIN__, __ULIBBSS_SHARE_END__);
    // 9. Stack frame (read-write), allocated dynamically
    entry_test_rwx1->stack_top = 0;
    entry_test_rwx1->stack_size = 96;
    // 10. va_list permission, allocated dynamically
    entry_test_rwx1->valist_size = 0;

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

    // Set library and closure IDs
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

    // Initialize code and memory bounds
    entry_test_rwx2->code_bounds_num = 0;
    entry_test_rwx2->mem_bounds_num = 0;
    entry_test_rwx2->temp_code_bounds_num = 0;
    entry_test_rwx2->temp_mem_bounds_num = 0;
    entry_test_rwx2->temp_times = 0;

    // Fill bounds data - using DASICS_LIBCFG_XX permissions
    // 1. Code segment (executable)
    ADD_CODE_BOUND(entry_test_rwx2, DASICS_LIBCFG_X, __ULIBTEXT_TEST_RWX2_BEGIN__, __ULIBTEXT_TEST_RWX2_END__);
    // 2. Data segment (read-write)
    ADD_MEM_BOUND(entry_test_rwx2, DASICS_LIBCFG_R | DASICS_LIBCFG_W, __ULIBDATA_TEST_RWX2_BEGIN__, __ULIBDATA_TEST_RWX2_END__);
    // 3. Read-only data segment (read-only)
    ADD_MEM_BOUND(entry_test_rwx2, DASICS_LIBCFG_R, __ULIBRODATA_TEST_RWX2_BEGIN__, __ULIBRODATA_TEST_RWX2_END__);
    // 4. BSS segment (read-write)
    ADD_MEM_BOUND(entry_test_rwx2, DASICS_LIBCFG_R | DASICS_LIBCFG_W, __ULIBBSS_TEST_RWX2_BEGIN__, __ULIBBSS_TEST_RWX2_END__);
    // 5. Shared code segment (executable)
    ADD_CODE_BOUND(entry_test_rwx2, DASICS_LIBCFG_X, __ULIBTEXT_SHARE_BEGIN__, __ULIBTEXT_SHARE_END__);
    // 6. Shared data segment (read-write)
    ADD_MEM_BOUND(entry_test_rwx2, DASICS_LIBCFG_R | DASICS_LIBCFG_W, __ULIBDATA_SHARE_BEGIN__, __ULIBDATA_SHARE_END__);
    // 7. Shared read-only data segment (read-only)
    ADD_MEM_BOUND(entry_test_rwx2, DASICS_LIBCFG_R, __ULIBRODATA_SHARE_BEGIN__, __ULIBRODATA_SHARE_END__);
    // 8. Shared BSS segment (read-write)
    ADD_MEM_BOUND(entry_test_rwx2, DASICS_LIBCFG_R | DASICS_LIBCFG_W, __ULIBBSS_SHARE_BEGIN__, __ULIBBSS_SHARE_END__);
    // 9. Stack frame (read-write), allocated dynamically
    entry_test_rwx2->stack_top = 0;
    entry_test_rwx2->stack_size = 96;
    // 10. va_list permission, allocated dynamically
    entry_test_rwx2->valist_size = 0;

    // Add to hash table (UTHash operation)
    HASH_ADD_PTR(fit_table, key, entry_test_rwx2);

    return 0;
}
