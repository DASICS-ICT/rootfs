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
#define ADD_STACK_BOUND(e, len) do { \
    if ((e)->mem_bounds_num < FIT_MEM_BOUNDS_MAX) { \
        (e)->mem_bounds[(e)->mem_bounds_num].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W; \
        (e)->mem_bounds[(e)->mem_bounds_num].lo = UINT64_MAX; \
        (e)->mem_bounds[(e)->mem_bounds_num].hi = (len); \
        (e)->mem_bounds_num++; \
    } \
} while (0)

int fit_init_static(void) {
    // Linker symbol declarations
    extern uint64_t __ULIBTEXT_TEST_RWX_BEGIN__;
    extern uint64_t __ULIBTEXT_TEST_RWX_END__;
    extern uint64_t __ULIBDATA_TEST_RWX_BEGIN__;
    extern uint64_t __ULIBDATA_TEST_RWX_END__;
    extern uint64_t __ULIBRODATA_TEST_RWX_BEGIN__;
    extern uint64_t __ULIBRODATA_TEST_RWX_END__;
    extern uint64_t __ULIBBSS_TEST_RWX_BEGIN__;
    extern uint64_t __ULIBBSS_TEST_RWX_END__;

    // Create "test_rwx" entry
    struct fit_entry *entry = (struct fit_entry *)malloc(sizeof(struct fit_entry));
    if (!entry) return -1;

    // Set key value
    extern void test_rwx(void);
    entry->key = (void *)test_rwx;  // Function address as key
    if (!entry->key) {
        free(entry);
        return -1;
    }

    // Set argument bound allocation and deallocation functions
    entry->argbound_alloc = NULL;  // No argument bound allocation function
    entry->argbound_free = NULL;   // No argument bound deallocation function
    entry->library_id = 0;
    entry->closure_id = 1;
    entry->heap_alloc_done = 0;

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

    entry->maincalls[Umaincall_PRINT / 8] |= (1 << (Umaincall_PRINT % 8));  // Set maincall bitmap

    // Initialize code and memory bounds
    entry->code_bounds_num = 0;
    entry->mem_bounds_num = 0;

    // Fill bounds data - using DASICS_LIBCFG_XX permissions
    // 1. Code segment (executable)
    ADD_CODE_BOUND(entry, DASICS_LIBCFG_X, __ULIBTEXT_TEST_RWX_BEGIN__, __ULIBTEXT_TEST_RWX_END__);
    // 2. Data segment (read-write)
    ADD_MEM_BOUND(entry, DASICS_LIBCFG_R | DASICS_LIBCFG_W, __ULIBDATA_TEST_RWX_BEGIN__, __ULIBDATA_TEST_RWX_END__);
    // 3. Read-only data segment (read-only)
    ADD_MEM_BOUND(entry, DASICS_LIBCFG_R, __ULIBRODATA_TEST_RWX_BEGIN__, __ULIBRODATA_TEST_RWX_END__);
    // 4. BSS segment (read-write)
    ADD_MEM_BOUND(entry, DASICS_LIBCFG_R | DASICS_LIBCFG_W, __ULIBBSS_TEST_RWX_BEGIN__, __ULIBBSS_TEST_RWX_END__);
    // 5. Stack frame (read-write)
    ADD_STACK_BOUND(entry, 32);

    // Add to hash table (UTHash operation)
    HASH_ADD_PTR(fit_table, key, entry);

    return 0;
}
