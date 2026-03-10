#include <stdio.h>
#include <stdlib.h>
#include <asm/unistd.h>
#include <fit.h>

#define BOUNDS_SLOTS 16

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
        e->bounds_num = 6;
        e->bounds_data = (struct fit_bounds *)malloc(BOUNDS_SLOTS * sizeof(struct fit_bounds));
        if (!e->bounds_data) {
            free(e->syscalls);
            free(e->maincalls);
            free(e);
            return -1;
        }
        e->bounds_data[0].perm = DASICS_LIBCFG_X;
        e->bounds_data[0].lo = (uint64_t)&__ULIBTEXT_FUNC1_BEGIN__;
        e->bounds_data[0].hi = (uint64_t)&__ULIBTEXT_FUNC1_END__;
        e->bounds_data[1].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
        e->bounds_data[1].lo = (uint64_t)&__ULIBDATA_FUNC1_BEGIN__;
        e->bounds_data[1].hi = (uint64_t)&__ULIBDATA_FUNC1_END__;
        e->bounds_data[2].perm = DASICS_LIBCFG_R;
        e->bounds_data[2].lo = (uint64_t)&__ULIBRODATA_FUNC1_BEGIN__;
        e->bounds_data[2].hi = (uint64_t)&__ULIBRODATA_FUNC1_END__;
        e->bounds_data[3].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
        e->bounds_data[3].lo = (uint64_t)&__ULIBBSS_FUNC1_BEGIN__;
        e->bounds_data[3].hi = (uint64_t)&__ULIBBSS_FUNC1_END__;
        e->bounds_data[4].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
        e->bounds_data[4].lo = (uint64_t)&__ULIBDATA_SHARE_BEGIN__;
        e->bounds_data[4].hi = (uint64_t)&__ULIBDATA_SHARE_END__;
        e->bounds_data[5].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
        e->bounds_data[5].lo = UINT64_MAX;
        e->bounds_data[5].hi = 48;
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
        e->bounds_num = 6;
        e->bounds_data = (struct fit_bounds *)malloc(BOUNDS_SLOTS * sizeof(struct fit_bounds));
        if (!e->bounds_data) {
            free(e->syscalls);
            free(e->maincalls);
            free(e);
            return -1;
        }
        e->bounds_data[0].perm = DASICS_LIBCFG_X;
        e->bounds_data[0].lo = (uint64_t)&__ULIBTEXT_FUNC2_BEGIN__;
        e->bounds_data[0].hi = (uint64_t)&__ULIBTEXT_FUNC2_END__;
        e->bounds_data[1].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
        e->bounds_data[1].lo = (uint64_t)&__ULIBDATA_FUNC2_BEGIN__;
        e->bounds_data[1].hi = (uint64_t)&__ULIBDATA_FUNC2_END__;
        e->bounds_data[2].perm = DASICS_LIBCFG_R;
        e->bounds_data[2].lo = (uint64_t)&__ULIBRODATA_FUNC2_BEGIN__;
        e->bounds_data[2].hi = (uint64_t)&__ULIBRODATA_FUNC2_END__;
        e->bounds_data[3].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
        e->bounds_data[3].lo = (uint64_t)&__ULIBBSS_FUNC2_BEGIN__;
        e->bounds_data[3].hi = (uint64_t)&__ULIBBSS_FUNC2_END__;
        e->bounds_data[4].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
        e->bounds_data[4].lo = (uint64_t)&__ULIBDATA_SHARE_BEGIN__;
        e->bounds_data[4].hi = (uint64_t)&__ULIBDATA_SHARE_END__;
        e->bounds_data[5].perm = DASICS_LIBCFG_R | DASICS_LIBCFG_W;
        e->bounds_data[5].lo = UINT64_MAX;
        e->bounds_data[5].hi = 32;
        HASH_ADD_PTR(fit_table, key, e);
    }

    return 0;
}
