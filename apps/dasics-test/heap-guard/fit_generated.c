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
    extern uint64_t __ULIBTEXT_FUNC1_BEGIN__, __ULIBTEXT_FUNC1_END__;
    extern uint64_t __ULIBTEXT_FUNC2_BEGIN__, __ULIBTEXT_FUNC2_END__;
    extern uint64_t __ULIBDATA_FUNC1_BEGIN__, __ULIBDATA_FUNC1_END__;
    extern uint64_t __ULIBDATA_FUNC2_BEGIN__, __ULIBDATA_FUNC2_END__;
    extern uint64_t __ULIBRODATA_FUNC1_BEGIN__, __ULIBRODATA_FUNC1_END__;
    extern uint64_t __ULIBRODATA_FUNC2_BEGIN__, __ULIBRODATA_FUNC2_END__;
    extern uint64_t __ULIBBSS_FUNC1_BEGIN__, __ULIBBSS_FUNC1_END__;
    extern uint64_t __ULIBBSS_FUNC2_BEGIN__, __ULIBBSS_FUNC2_END__;
    extern uint64_t __ULIBDATA_SHARE_BEGIN__, __ULIBDATA_SHARE_END__;

    /* Entry for func1 */
    {
        struct fit_entry *e = (struct fit_entry *)malloc(sizeof(struct fit_entry));
        if (!e) return -1;
        extern void func1(void);
        e->key = (void *)func1;
        e->argbound_alloc = NULL;
        e->argbound_free = NULL;
        e->syscalls_size = (__NR_syscalls + 7) / 8;
        e->maincalls_size = (Umaincall_UNKNOWN + 7) / 8;
        e->syscalls = bitmap_alloc(__NR_syscalls);
        e->maincalls = bitmap_alloc(Umaincall_UNKNOWN);
        if (!e->syscalls || !e->maincalls) {
            if (e->syscalls) free(e->syscalls);
            if (e->maincalls) free(e->maincalls);
            free(e);
            return -1;
        }
        e->maincalls[Umaincall_PRINT / 8] |= (1 << (Umaincall_PRINT % 8));
        e->maincalls[Umaincall_MALLOC / 8] |= (1 << (Umaincall_MALLOC % 8));
        e->library_id = 0;
        e->closure_id = 1;
        e->heap_alloc_done = 0;
        e->code_bounds_num = 0;
        e->mem_bounds_num = 0;

        ADD_CODE_BOUND(e, DASICS_LIBCFG_X, __ULIBTEXT_FUNC1_BEGIN__, __ULIBTEXT_FUNC1_END__);
        ADD_MEM_BOUND(e, DASICS_LIBCFG_R | DASICS_LIBCFG_W, __ULIBDATA_FUNC1_BEGIN__, __ULIBDATA_FUNC1_END__);
        ADD_MEM_BOUND(e, DASICS_LIBCFG_R, __ULIBRODATA_FUNC1_BEGIN__, __ULIBRODATA_FUNC1_END__);
        ADD_MEM_BOUND(e, DASICS_LIBCFG_R | DASICS_LIBCFG_W, __ULIBBSS_FUNC1_BEGIN__, __ULIBBSS_FUNC1_END__);
        ADD_MEM_BOUND(e, DASICS_LIBCFG_R | DASICS_LIBCFG_W, __ULIBDATA_SHARE_BEGIN__, __ULIBDATA_SHARE_END__);
        ADD_STACK_BOUND(e, 48);
        HASH_ADD_PTR(fit_table, key, e);
    }

    /* Entry for func2 */
    {
        struct fit_entry *e = (struct fit_entry *)malloc(sizeof(struct fit_entry));
        if (!e) return -1;
        extern void func2(void);
        e->key = (void *)func2;
        e->argbound_alloc = NULL;
        e->argbound_free = NULL;
        e->syscalls_size = (__NR_syscalls + 7) / 8;
        e->maincalls_size = (Umaincall_UNKNOWN + 7) / 8;
        e->syscalls = bitmap_alloc(__NR_syscalls);
        e->maincalls = bitmap_alloc(Umaincall_UNKNOWN);
        if (!e->syscalls || !e->maincalls) {
            if (e->syscalls) free(e->syscalls);
            if (e->maincalls) free(e->maincalls);
            free(e);
            return -1;
        }
        e->maincalls[Umaincall_PRINT / 8] |= (1 << (Umaincall_PRINT % 8));
        e->maincalls[Umaincall_MALLOC / 8] |= (1 << (Umaincall_MALLOC % 8));
        e->library_id = 0;
        e->closure_id = 2;
        e->heap_alloc_done = 0;
        e->code_bounds_num = 0;
        e->mem_bounds_num = 0;

        ADD_CODE_BOUND(e, DASICS_LIBCFG_X, __ULIBTEXT_FUNC2_BEGIN__, __ULIBTEXT_FUNC2_END__);
        ADD_MEM_BOUND(e, DASICS_LIBCFG_R | DASICS_LIBCFG_W, __ULIBDATA_FUNC2_BEGIN__, __ULIBDATA_FUNC2_END__);
        ADD_MEM_BOUND(e, DASICS_LIBCFG_R, __ULIBRODATA_FUNC2_BEGIN__, __ULIBRODATA_FUNC2_END__);
        ADD_MEM_BOUND(e, DASICS_LIBCFG_R | DASICS_LIBCFG_W, __ULIBBSS_FUNC2_BEGIN__, __ULIBBSS_FUNC2_END__);
        ADD_MEM_BOUND(e, DASICS_LIBCFG_R | DASICS_LIBCFG_W, __ULIBDATA_SHARE_BEGIN__, __ULIBDATA_SHARE_END__);
        ADD_STACK_BOUND(e, 32);
        HASH_ADD_PTR(fit_table, key, e);
    }

    return 0;
}
