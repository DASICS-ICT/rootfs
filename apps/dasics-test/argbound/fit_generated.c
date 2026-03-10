#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <asm/unistd.h>
#include <fit.h>
#include <udasics.h>

static fit_handles_t *handle_argbound_test_argbound = NULL;
static const size_t argbound_num_test_argbound = 3;

static void fit_argbound_alloc_test_argbound(va_list args) {
    // Allocate memory for the handle array
    handle_argbound_test_argbound = (fit_handles_t *)malloc(argbound_num_test_argbound * sizeof(fit_handles_t));
    if (!handle_argbound_test_argbound) {
        fprintf(stderr, "Memory allocation failed for handle_argbound_test_argbound\n");
        return;
    }

    // Initialize the handles and permissions for va_list args
    handle_argbound_test_argbound[0].handle = dasics_libcfg_alloc(DASICS_LIBCFG_R | DASICS_LIBCFG_W, (uint64_t)args, (uint64_t)args + sizeof(char *) * 2);
    handle_argbound_test_argbound[0].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;

    // Get the arguments
    char *src = va_arg(args, char *);
    char *dst = va_arg(args, char *);

    // Initialize the handles and permissions
    handle_argbound_test_argbound[1].handle = dasics_libcfg_alloc(DASICS_LIBCFG_R, (uint64_t)src, (uint64_t)src + 10);
    handle_argbound_test_argbound[1].perm = DASICS_LIBCFG_R;
    handle_argbound_test_argbound[2].handle = dasics_libcfg_alloc(DASICS_LIBCFG_W, (uint64_t)dst, (uint64_t)dst + 10);
    handle_argbound_test_argbound[2].perm = DASICS_LIBCFG_W;
}

static void fit_argbound_free_test_argbound(void) {
    // Free the allocated handles
    for (size_t i = 0; i < argbound_num_test_argbound; i++) {
        if (handle_argbound_test_argbound[i].perm & DASICS_LIBCFG_X) {
            dasics_jumpcfg_free(handle_argbound_test_argbound[i].handle);
        } else {
            dasics_libcfg_free(handle_argbound_test_argbound[i].handle);
        }
    }
    free(handle_argbound_test_argbound);
    handle_argbound_test_argbound = NULL;
}

int fit_init_static(void) {
    // Linker symbol declarations
    extern uint64_t __ULIBTEXT_TEST_ARGBOUND_BEGIN__;
    extern uint64_t __ULIBTEXT_TEST_ARGBOUND_END__;
    extern uint64_t __ULIBDATA_TEST_ARGBOUND_BEGIN__;
    extern uint64_t __ULIBDATA_TEST_ARGBOUND_END__;
    extern uint64_t __ULIBRODATA_TEST_ARGBOUND_BEGIN__;
    extern uint64_t __ULIBRODATA_TEST_ARGBOUND_END__;
    extern uint64_t __ULIBBSS_TEST_ARGBOUND_BEGIN__;
    extern uint64_t __ULIBBSS_TEST_ARGBOUND_END__;

    // Create "test_argbound" entry
    struct fit_entry *entry = (struct fit_entry *)malloc(sizeof(struct fit_entry));
    if (!entry) return -1;

    // Set key value
    extern int test_argbound_wrapper(va_list);
    entry->key = (void *)test_argbound_wrapper;  // Function address as key
    if (!entry->key) {
        free(entry);
        return -1;
    }

    // Set argument bound allocation and deallocation functions
    entry->argbound_alloc = fit_argbound_alloc_test_argbound;
    entry->argbound_free = fit_argbound_free_test_argbound;
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
    entry->bounds_data[0].lo = (uint64_t)&__ULIBTEXT_TEST_ARGBOUND_BEGIN__;
    entry->bounds_data[0].hi = (uint64_t)&__ULIBTEXT_TEST_ARGBOUND_END__;

    // 2. Data segment (read-write)
    entry->bounds_data[1].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
    entry->bounds_data[1].lo = (uint64_t)&__ULIBDATA_TEST_ARGBOUND_BEGIN__;
    entry->bounds_data[1].hi = (uint64_t)&__ULIBDATA_TEST_ARGBOUND_END__;

    // 3. Read-only data segment (read-only)
    entry->bounds_data[2].perm = DASICS_LIBCFG_R;
    entry->bounds_data[2].lo = (uint64_t)&__ULIBRODATA_TEST_ARGBOUND_BEGIN__;
    entry->bounds_data[2].hi = (uint64_t)&__ULIBRODATA_TEST_ARGBOUND_END__;

    // 4. BSS segment (read-write)
    entry->bounds_data[3].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
    entry->bounds_data[3].lo = (uint64_t)&__ULIBBSS_TEST_ARGBOUND_BEGIN__;
    entry->bounds_data[3].hi = (uint64_t)&__ULIBBSS_TEST_ARGBOUND_END__;

    // 5. Stack frame (read-write)
    entry->bounds_data[4].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
    entry->bounds_data[4].lo = UINT64_MAX;  // Maximum uint64_t value indicates stack frame
    entry->bounds_data[4].hi = 96;

    // Add to hash table (UTHash operation)
    HASH_ADD_PTR(fit_table, key, entry);

    return 0;
}