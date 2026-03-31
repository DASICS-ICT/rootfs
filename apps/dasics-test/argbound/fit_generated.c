#include <stdarg.h>
#include <compartment.h>

int fit_init_static(void) {
    extern uint64_t __ULIBTEXT_TEST_ARGBOUND_BEGIN__, __ULIBTEXT_TEST_ARGBOUND_END__;
    extern uint64_t __ULIBDATA_TEST_ARGBOUND_BEGIN__, __ULIBDATA_TEST_ARGBOUND_END__;
    extern uint64_t __ULIBRODATA_TEST_ARGBOUND_BEGIN__, __ULIBRODATA_TEST_ARGBOUND_END__;
    extern uint64_t __ULIBBSS_TEST_ARGBOUND_BEGIN__, __ULIBBSS_TEST_ARGBOUND_END__;

    extern int test_argbound_wrapper(va_list);
    compartment_t *comp = compartment_create(test_argbound_wrapper);
    if (!comp) return -1;

    compartment_permit_maincall(comp, 1, Umaincall_PRINT);

    compartment_add_code_bound(comp, DASICS_LIBCFG_X,
        (uint64_t)&__ULIBTEXT_TEST_ARGBOUND_BEGIN__, (uint64_t)&__ULIBTEXT_TEST_ARGBOUND_END__);
    compartment_add_mem_bound(comp, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        (uint64_t)&__ULIBDATA_TEST_ARGBOUND_BEGIN__, (uint64_t)&__ULIBDATA_TEST_ARGBOUND_END__);
    compartment_add_mem_bound(comp, DASICS_LIBCFG_R,
        (uint64_t)&__ULIBRODATA_TEST_ARGBOUND_BEGIN__, (uint64_t)&__ULIBRODATA_TEST_ARGBOUND_END__);
    compartment_add_mem_bound(comp, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        (uint64_t)&__ULIBBSS_TEST_ARGBOUND_BEGIN__, (uint64_t)&__ULIBBSS_TEST_ARGBOUND_END__);

    compartment_set_stack(comp, 0x70);

    return 0;
}
