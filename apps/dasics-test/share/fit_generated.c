#include <compartment.h>

int fit_init_static(void) {
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

    /* Compartment for test_rwx1 */
    extern void test_rwx1(void);
    compartment_t *comp1 = compartment_create(test_rwx1);
    if (!comp1) return -1;

    compartment_permit_maincall(comp1, 1, Umaincall_PRINT);

    /* Private bounds */
    compartment_add_code_bound(comp1, DASICS_LIBCFG_X,
        (uint64_t)&__ULIBTEXT_TEST_RWX1_BEGIN__, (uint64_t)&__ULIBTEXT_TEST_RWX1_END__);
    compartment_add_mem_bound(comp1, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        (uint64_t)&__ULIBDATA_TEST_RWX1_BEGIN__, (uint64_t)&__ULIBDATA_TEST_RWX1_END__);
    compartment_add_mem_bound(comp1, DASICS_LIBCFG_R,
        (uint64_t)&__ULIBRODATA_TEST_RWX1_BEGIN__, (uint64_t)&__ULIBRODATA_TEST_RWX1_END__);
    compartment_add_mem_bound(comp1, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        (uint64_t)&__ULIBBSS_TEST_RWX1_BEGIN__, (uint64_t)&__ULIBBSS_TEST_RWX1_END__);
    /* Shared bounds */
    compartment_add_code_bound(comp1, DASICS_LIBCFG_X,
        (uint64_t)&__ULIBTEXT_SHARE_BEGIN__, (uint64_t)&__ULIBTEXT_SHARE_END__);
    compartment_add_mem_bound(comp1, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        (uint64_t)&__ULIBDATA_SHARE_BEGIN__, (uint64_t)&__ULIBDATA_SHARE_END__);
    compartment_add_mem_bound(comp1, DASICS_LIBCFG_R,
        (uint64_t)&__ULIBRODATA_SHARE_BEGIN__, (uint64_t)&__ULIBRODATA_SHARE_END__);
    compartment_add_mem_bound(comp1, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        (uint64_t)&__ULIBBSS_SHARE_BEGIN__, (uint64_t)&__ULIBBSS_SHARE_END__);

    compartment_set_stack(comp1, 0x60 + 0x50);

    /* Compartment for test_rwx2 */
    extern void test_rwx2(void);
    compartment_t *comp2 = compartment_create(test_rwx2);
    if (!comp2) return -1;

    compartment_permit_maincall(comp2, 1, Umaincall_PRINT);

    /* Private bounds */
    compartment_add_code_bound(comp2, DASICS_LIBCFG_X,
        (uint64_t)&__ULIBTEXT_TEST_RWX2_BEGIN__, (uint64_t)&__ULIBTEXT_TEST_RWX2_END__);
    compartment_add_mem_bound(comp2, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        (uint64_t)&__ULIBDATA_TEST_RWX2_BEGIN__, (uint64_t)&__ULIBDATA_TEST_RWX2_END__);
    compartment_add_mem_bound(comp2, DASICS_LIBCFG_R,
        (uint64_t)&__ULIBRODATA_TEST_RWX2_BEGIN__, (uint64_t)&__ULIBRODATA_TEST_RWX2_END__);
    compartment_add_mem_bound(comp2, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        (uint64_t)&__ULIBBSS_TEST_RWX2_BEGIN__, (uint64_t)&__ULIBBSS_TEST_RWX2_END__);
    /* Shared bounds */
    compartment_add_code_bound(comp2, DASICS_LIBCFG_X,
        (uint64_t)&__ULIBTEXT_SHARE_BEGIN__, (uint64_t)&__ULIBTEXT_SHARE_END__);
    compartment_add_mem_bound(comp2, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        (uint64_t)&__ULIBDATA_SHARE_BEGIN__, (uint64_t)&__ULIBDATA_SHARE_END__);
    compartment_add_mem_bound(comp2, DASICS_LIBCFG_R,
        (uint64_t)&__ULIBRODATA_SHARE_BEGIN__, (uint64_t)&__ULIBRODATA_SHARE_END__);
    compartment_add_mem_bound(comp2, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        (uint64_t)&__ULIBBSS_SHARE_BEGIN__, (uint64_t)&__ULIBBSS_SHARE_END__);

    compartment_set_stack(comp2, 0x60 + 0x50);

    return 0;
}
