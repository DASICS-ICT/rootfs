#include <compartment.h>

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

    /* Compartment for func1 */
    extern void func1(void);
    compartment_t *comp1 = compartment_create(func1);
    if (!comp1) return -1;

    compartment_permit_maincall(comp1, 2, Umaincall_PRINT, Umaincall_MALLOC);

    compartment_add_code_bound(comp1, DASICS_LIBCFG_X,
        (uint64_t)&__ULIBTEXT_FUNC1_BEGIN__, (uint64_t)&__ULIBTEXT_FUNC1_END__);
    compartment_add_mem_bound(comp1, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        (uint64_t)&__ULIBDATA_FUNC1_BEGIN__, (uint64_t)&__ULIBDATA_FUNC1_END__);
    compartment_add_mem_bound(comp1, DASICS_LIBCFG_R,
        (uint64_t)&__ULIBRODATA_FUNC1_BEGIN__, (uint64_t)&__ULIBRODATA_FUNC1_END__);
    compartment_add_mem_bound(comp1, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        (uint64_t)&__ULIBBSS_FUNC1_BEGIN__, (uint64_t)&__ULIBBSS_FUNC1_END__);
    compartment_add_mem_bound(comp1, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        (uint64_t)&__ULIBDATA_SHARE_BEGIN__, (uint64_t)&__ULIBDATA_SHARE_END__);

    compartment_set_stack(comp1, 0x30);

    /* Compartment for func2 */
    extern void func2(void);
    compartment_t *comp2 = compartment_create(func2);
    if (!comp2) return -1;

    compartment_permit_maincall(comp2, 2, Umaincall_PRINT, Umaincall_MALLOC);

    compartment_add_code_bound(comp2, DASICS_LIBCFG_X,
        (uint64_t)&__ULIBTEXT_FUNC2_BEGIN__, (uint64_t)&__ULIBTEXT_FUNC2_END__);
    compartment_add_mem_bound(comp2, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        (uint64_t)&__ULIBDATA_FUNC2_BEGIN__, (uint64_t)&__ULIBDATA_FUNC2_END__);
    compartment_add_mem_bound(comp2, DASICS_LIBCFG_R,
        (uint64_t)&__ULIBRODATA_FUNC2_BEGIN__, (uint64_t)&__ULIBRODATA_FUNC2_END__);
    compartment_add_mem_bound(comp2, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        (uint64_t)&__ULIBBSS_FUNC2_BEGIN__, (uint64_t)&__ULIBBSS_FUNC2_END__);
    compartment_add_mem_bound(comp2, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        (uint64_t)&__ULIBDATA_SHARE_BEGIN__, (uint64_t)&__ULIBDATA_SHARE_END__);

    compartment_set_stack(comp2, 0x20);

    return 0;
}
