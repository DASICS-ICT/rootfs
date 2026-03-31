/*
 * fit_generated.c -- FIT compartment registration for the vuldyn test.
 *
 * This file implements fit_init_dynamic_libvuldyn(), and exposes
 * fit_init_dynamic() as the aggregation entry called by fit_init()
 * during program startup. It creates three compartments for the three
 * logical zones in libvuldyn.so:
 *
 *   1. entry     (closure_id=auto >= 1) -- Full library bounds.  Can execute all
 *                                  ulibtext code and access all ulibdata.
 *                                  Maincalls: PRINT, TRANS.
 *
 *   2. func1     (closure_id=auto >= 1) -- Restricted to func1 + share sections.
 *                                  Maincalls: PRINT, MALLOC, PGRANT, TRANS, FREE.
 *
 *   3. func2     (closure_id=auto >= 1) -- Restricted to func2 + share sections.
 *                                  Maincalls: PRINT.
 *                                  Syscalls:  getpid.
 *
 * Each compartment also receives:
 *   - A PLT jump bound (the .so's .plt section) so external function
 *     calls through PLT stubs are permitted.
 *   - A GOT mem bound  (the .so's .got section, read-only) so PLT stubs
 *     can load target addresses from the Global Offset Table.
 *   - A stack bound (allocated from the current stack at domain-switch time).
 *
 * IMPORTANT -- Dynamic library boundary symbol access pattern:
 *
 *   Unlike the static-linking tests (rwx, syscall, etc.) where boundary
 *   symbols are in the same executable and accessed via `(uint64_t)&sym`,
 *   the vuldyn test uses a DYNAMIC library (libvuldyn.so).
 *
 *   Linker-script address symbols cannot survive copy relocation across
 *   the .so boundary.  Instead, libvuldyn.so exports 8-byte data objects
 *   (defined in vuldyn_syms.S) whose VALUE is the boundary address.
 *   We read the boundary address by accessing the variable's value:
 *
 *     Static linking (rwx):   (uint64_t)&sym   -- take address of symbol
 *     Dynamic linking (here):  sym              -- read value of data object
 */

#include <stdint.h>
#include <asm/unistd.h>
#include <compartment.h>
#include <dynamic.h>

int fit_init_dynamic_libvuldyn(void)
{
    /* ==================================================================
     * Boundary data objects exported from libvuldyn.so (vuldyn_syms.S).
     *
     * Each is an 8-byte uint64_t whose VALUE is the runtime address of
     * the corresponding section boundary.  Access them directly (sym),
     * NOT via &sym -- see file header comment for explanation.
     * ================================================================== */

    /* -- ulibtext (executable code) boundaries -- */
    extern uint64_t __ULIBTEXT_VULDYN_BEGIN__,   __ULIBTEXT_VULDYN_END__;
    extern uint64_t __ULIBTEXT_VULDYN_ENTRY_BEGIN__, __ULIBTEXT_VULDYN_ENTRY_END__;
    extern uint64_t __ULIBTEXT_VULDYN_FUNC1_BEGIN__, __ULIBTEXT_VULDYN_FUNC1_END__;
    extern uint64_t __ULIBTEXT_VULDYN_FUNC2_BEGIN__, __ULIBTEXT_VULDYN_FUNC2_END__;
    extern uint64_t __ULIBTEXT_VULDYN_SHARE_BEGIN__, __ULIBTEXT_VULDYN_SHARE_END__;

    /* -- ulibrodata (read-only data) boundaries -- */
    extern uint64_t __ULIBRODATA_VULDYN_BEGIN__,   __ULIBRODATA_VULDYN_END__;
    extern uint64_t __ULIBRODATA_VULDYN_ENTRY_BEGIN__, __ULIBRODATA_VULDYN_ENTRY_END__;
    extern uint64_t __ULIBRODATA_VULDYN_FUNC1_BEGIN__, __ULIBRODATA_VULDYN_FUNC1_END__;
    extern uint64_t __ULIBRODATA_VULDYN_FUNC2_BEGIN__, __ULIBRODATA_VULDYN_FUNC2_END__;
    extern uint64_t __ULIBRODATA_VULDYN_SHARE_BEGIN__, __ULIBRODATA_VULDYN_SHARE_END__;

    /* -- ulibdata (read-write initialized data) boundaries -- */
    extern uint64_t __ULIBDATA_VULDYN_BEGIN__,   __ULIBDATA_VULDYN_END__;
    extern uint64_t __ULIBDATA_VULDYN_ENTRY_BEGIN__, __ULIBDATA_VULDYN_ENTRY_END__;
    extern uint64_t __ULIBDATA_VULDYN_FUNC1_BEGIN__, __ULIBDATA_VULDYN_FUNC1_END__;
    extern uint64_t __ULIBDATA_VULDYN_FUNC2_BEGIN__, __ULIBDATA_VULDYN_FUNC2_END__;
    extern uint64_t __ULIBDATA_VULDYN_SHARE_BEGIN__, __ULIBDATA_VULDYN_SHARE_END__;

    /* -- ulibbss (read-write zero-initialized data) boundaries -- */
    extern uint64_t __ULIBBSS_VULDYN_BEGIN__,   __ULIBBSS_VULDYN_END__;
    extern uint64_t __ULIBBSS_VULDYN_ENTRY_BEGIN__, __ULIBBSS_VULDYN_ENTRY_END__;
    extern uint64_t __ULIBBSS_VULDYN_FUNC1_BEGIN__, __ULIBBSS_VULDYN_FUNC1_END__;
    extern uint64_t __ULIBBSS_VULDYN_FUNC2_BEGIN__, __ULIBBSS_VULDYN_FUNC2_END__;
    extern uint64_t __ULIBBSS_VULDYN_SHARE_BEGIN__, __ULIBBSS_VULDYN_SHARE_END__;

    /* -- PLT and GOT boundaries (for PIC external calls without -fno-plt) -- */
    extern uint64_t __VULDYN_PLT_BEGIN__, __VULDYN_PLT_END__;
    extern uint64_t __VULDYN_GOT_BEGIN__, __VULDYN_GOT_END__;
    /*
     * Real in-library function entry addresses exported as .quad objects.
     * Use VALUE (sym), not &sym, so FIT key matches do_transition_dynamic(func).
     *
     * These values are the real in-library entry addresses.
     */
    extern uint64_t __VULDYN_ENTRY_ADDR__;
    extern uint64_t __VULDYN_FUNC1_ADDR__;
    extern uint64_t __VULDYN_FUNC2_ADDR__;


    /* ==================================================================
     * Compartment 1: entry (library_id=runtime, closure_id=auto)
     *
     * This is the outermost untrusted compartment.  It has full access to
     * the entire library's custom sections (all of ulibtext, ulibrodata,
     * ulibdata, ulibbss).  Its only purpose is to domain-switch into func1.
     *
     * Maincalls: PRINT (for logging) and TRANS (for domain switching).
     *
     * Bounds:
     *   Jump: 1 (whole ULIBTEXT) + 1 (PLT) = 2 / 4 max
     *   Mem:  3 (whole rodata+data+bss) + 1 (GOT R) + 1 (stack) = 5 / 16 max
     * ================================================================== */
    extern void entry(void);
    compartment_t *comp_entry = compartment_create(
        (void *)(uintptr_t)__VULDYN_ENTRY_ADDR__);
    if (!comp_entry) return -1;
    if (!compartment_duplicate(comp_entry, entry)) return -1;

    /* Permitted maincalls: PRINT for output, TRANS for domain switching */
    compartment_permit_maincall(comp_entry, 2, Umaincall_PRINT, Umaincall_TRANS);

    /* Code bound: entire .ulibtext range (entry can execute any lib code) */
    compartment_add_code_bound(comp_entry, DASICS_LIBCFG_X,
        __ULIBTEXT_VULDYN_BEGIN__, __ULIBTEXT_VULDYN_END__);

    /* Code bound: PLT section (required for external calls via PLT stubs) */
    compartment_add_code_bound(comp_entry, DASICS_LIBCFG_X,
        __VULDYN_PLT_BEGIN__, __VULDYN_PLT_END__);

    /* Mem bounds: entire library rodata (R), data (RW), bss (RW) */
    compartment_add_mem_bound(comp_entry, DASICS_LIBCFG_R,
        __ULIBRODATA_VULDYN_BEGIN__, __ULIBRODATA_VULDYN_END__);
    compartment_add_mem_bound(comp_entry, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        __ULIBDATA_VULDYN_BEGIN__, __ULIBDATA_VULDYN_END__);
    compartment_add_mem_bound(comp_entry, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        __ULIBBSS_VULDYN_BEGIN__, __ULIBBSS_VULDYN_END__);

    /* Mem bound: GOT section (R) -- PLT stubs load function pointers from here */
    compartment_add_mem_bound(comp_entry, DASICS_LIBCFG_R,
        __VULDYN_GOT_BEGIN__, __VULDYN_GOT_END__);

    /*
     * Stack: entry must have enough headroom for printf/maincall path
     * and normal function prologue/epilogue. 16 bytes is too small and
     * can corrupt return context near the final return-to-main path.
     */
    compartment_set_stack(comp_entry, 0x20);


    /* ==================================================================
     * Compartment 2: func1 (library_id=runtime, closure_id=auto)
     *
     * Restricted compartment for func1.  Has access to:
     *   - func1's private code and data sections
     *   - share's code and data sections (for calling share())
     *   - PLT and GOT (for external calls)
     *
     * Maincalls: PRINT, MALLOC (heap alloc), PGRANT (permission grant),
     *            TRANS (domain switch to func2), FREE (heap dealloc).
     *
     * Bounds:
     *   Jump: 1 (func1 text) + 1 (share text) + 1 (PLT) = 3 / 4 max
     *   Mem:  3 (func1 rodata/data/bss) + 3 (share rodata/data/bss)
     *         + 1 (GOT R) + 1 (stack) = 8 / 16 max
     *         (+ heap bound added dynamically by Umaincall_MALLOC)
     * ================================================================== */
    extern void func1(void);
    compartment_t *comp_func1 = compartment_create(
        (void *)(uintptr_t)__VULDYN_FUNC1_ADDR__);
    if (!comp_func1) return -1;
    if (!compartment_duplicate(comp_func1, func1)) return -1;

    /* Permitted maincalls for the full test sequence */
    compartment_permit_maincall(comp_func1, 5,
        Umaincall_PRINT, Umaincall_MALLOC, Umaincall_PGRANT,
        Umaincall_TRANS, Umaincall_FREE);

    /* Code bounds: func1 code + shared code + PLT */
    compartment_add_code_bound(comp_func1, DASICS_LIBCFG_X,
        __ULIBTEXT_VULDYN_FUNC1_BEGIN__, __ULIBTEXT_VULDYN_FUNC1_END__);
    compartment_add_code_bound(comp_func1, DASICS_LIBCFG_X,
        __ULIBTEXT_VULDYN_SHARE_BEGIN__, __ULIBTEXT_VULDYN_SHARE_END__);
    compartment_add_code_bound(comp_func1, DASICS_LIBCFG_X,
        __VULDYN_PLT_BEGIN__, __VULDYN_PLT_END__);

    /* Mem bounds: func1 private data sections */
    compartment_add_mem_bound(comp_func1, DASICS_LIBCFG_R,
        __ULIBRODATA_VULDYN_FUNC1_BEGIN__, __ULIBRODATA_VULDYN_FUNC1_END__);
    compartment_add_mem_bound(comp_func1, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        __ULIBDATA_VULDYN_FUNC1_BEGIN__, __ULIBDATA_VULDYN_FUNC1_END__);
    compartment_add_mem_bound(comp_func1, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        __ULIBBSS_VULDYN_FUNC1_BEGIN__, __ULIBBSS_VULDYN_FUNC1_END__);

    /* Mem bounds: shared data sections (func1 calls share()) */
    compartment_add_mem_bound(comp_func1, DASICS_LIBCFG_R,
        __ULIBRODATA_VULDYN_SHARE_BEGIN__, __ULIBRODATA_VULDYN_SHARE_END__);
    compartment_add_mem_bound(comp_func1, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        __ULIBDATA_VULDYN_SHARE_BEGIN__, __ULIBDATA_VULDYN_SHARE_END__);
    compartment_add_mem_bound(comp_func1, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        __ULIBBSS_VULDYN_SHARE_BEGIN__, __ULIBBSS_VULDYN_SHARE_END__);

    /* Mem bound: GOT (R) for PLT function pointer resolution */
    compartment_add_mem_bound(comp_func1, DASICS_LIBCFG_R,
        __VULDYN_GOT_BEGIN__, __VULDYN_GOT_END__);

    /* Stack: func1 + share (no wrapper overhead) */
    compartment_set_stack(comp_func1, 0x80 + 0x30);


    /* ==================================================================
     * Compartment 3: func2 (library_id=runtime, closure_id=auto)
     *
     * Restricted compartment for func2.  Has access to:
     *   - func2's private code and data sections
     *   - share's code and data sections (for calling share() if needed)
     *   - PLT and GOT (for external calls)
     *
     * Maincalls: PRINT only (func2 should not be able to allocate/free/switch).
     * Syscalls:  getpid only (__NR_exit is NOT allowed -> will fault).
     *
     * Bounds:
     *   Jump: 1 (func2 text) + 1 (share text) + 1 (PLT) = 3 / 4 max
     *   Mem:  3 (func2 rodata/data/bss) + 3 (share rodata/data/bss)
     *         + 1 (GOT R) + 1 (stack) = 8 / 16 max
     *         (+ temp argbound added dynamically)
     * ================================================================== */
    extern void func2(char *str);
    compartment_t *comp_func2 = compartment_create(
        (void *)(uintptr_t)__VULDYN_FUNC2_ADDR__);
    if (!comp_func2) return -1;
    if (!compartment_duplicate(comp_func2, func2)) return -1;

    /* Permitted maincall: PRINT only (attempting UNKNOWN will be rejected) */
    compartment_permit_maincall(comp_func2, 1, Umaincall_PRINT);

    /* Permitted syscall: getpid only (attempting exit will fault) */
    compartment_permit_syscall(comp_func2, 1, __NR_getpid);

    /* Code bounds: func2 code + shared code + PLT */
    compartment_add_code_bound(comp_func2, DASICS_LIBCFG_X,
        __ULIBTEXT_VULDYN_FUNC2_BEGIN__, __ULIBTEXT_VULDYN_FUNC2_END__);
    compartment_add_code_bound(comp_func2, DASICS_LIBCFG_X,
        __ULIBTEXT_VULDYN_SHARE_BEGIN__, __ULIBTEXT_VULDYN_SHARE_END__);
    compartment_add_code_bound(comp_func2, DASICS_LIBCFG_X,
        __VULDYN_PLT_BEGIN__, __VULDYN_PLT_END__);

    /* Mem bounds: func2 private data sections */
    compartment_add_mem_bound(comp_func2, DASICS_LIBCFG_R,
        __ULIBRODATA_VULDYN_FUNC2_BEGIN__, __ULIBRODATA_VULDYN_FUNC2_END__);
    compartment_add_mem_bound(comp_func2, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        __ULIBDATA_VULDYN_FUNC2_BEGIN__, __ULIBDATA_VULDYN_FUNC2_END__);
    compartment_add_mem_bound(comp_func2, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        __ULIBBSS_VULDYN_FUNC2_BEGIN__, __ULIBBSS_VULDYN_FUNC2_END__);

    /* Mem bounds: shared data sections */
    compartment_add_mem_bound(comp_func2, DASICS_LIBCFG_R,
        __ULIBRODATA_VULDYN_SHARE_BEGIN__, __ULIBRODATA_VULDYN_SHARE_END__);
    compartment_add_mem_bound(comp_func2, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        __ULIBDATA_VULDYN_SHARE_BEGIN__, __ULIBDATA_VULDYN_SHARE_END__);
    compartment_add_mem_bound(comp_func2, DASICS_LIBCFG_R | DASICS_LIBCFG_W,
        __ULIBBSS_VULDYN_SHARE_BEGIN__, __ULIBBSS_VULDYN_SHARE_END__);

    /* Mem bound: GOT (R) for PLT function pointer resolution */
    compartment_add_mem_bound(comp_func2, DASICS_LIBCFG_R,
        __VULDYN_GOT_BEGIN__, __VULDYN_GOT_END__);

    /* Stack: func2 + share (no wrapper overhead) */
    compartment_set_stack(comp_func2, 0x120 + 0x30);

    return 0;
}

int fit_init_dynamic(void)
{
#ifdef DASICS_DEBUG
    printf("[LOG] fit_init_dynamic: register compartments for libvuldyn.so\n");
#endif
    return fit_init_dynamic_libvuldyn();
}
