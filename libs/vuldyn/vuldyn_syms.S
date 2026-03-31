/*
 * vuldyn_syms.S -- Exported boundary address data objects for libvuldyn.so
 *
 * Problem:
 *   The linker script (ld_vuldyn.lds) defines internal symbols like
 *   _vuldyn_ulibtext_begin whose ELF value IS the section boundary address.
 *   When the main executable links against the .so, copy relocation makes
 *   `&sym` return a BSS address in the main executable -- NOT the boundary
 *   address in the .so.  So the main program cannot obtain boundary addresses
 *   via the traditional `extern uint64_t sym; (uint64_t)&sym` pattern.
 *
 * Solution:
 *   This file creates EXPORTED symbols as 8-byte data objects (.quad).
 *   Each .quad stores the VALUE of the corresponding internal linker-script
 *   symbol.  For a PIC shared library, the assembler emits an R_RISCV_64
 *   relocation for each .quad, which the dynamic linker patches to the
 *   correct runtime address at load time.
 *
 *   When the main executable does copy relocation on these 8-byte objects,
 *   it copies the already-patched address values.  The main program then
 *   reads the boundary address by simply accessing the variable's value
 *   (not its address):
 *
 *     extern uint64_t __ULIBTEXT_VULDYN_BEGIN__;
 *     uint64_t addr = __ULIBTEXT_VULDYN_BEGIN__;   // correct runtime address
 *                  // NOT &__ULIBTEXT_VULDYN_BEGIN__ (that gives BSS addr)
 *
 * Naming convention:
 *   Internal (linker script):  _vuldyn_ulibtext_begin      (value = boundary)
 *   Exported (this file):      __ULIBTEXT_VULDYN_BEGIN__    (data  = boundary)
 */

    .section .rodata

    /*
     * DECLARE_SYM: Create an exported 8-byte data object whose content
     * is the runtime address of an internal linker-script symbol.
     *
     *   \exported  - the global symbol name visible to the main executable
     *   \internal  - the linker-script symbol whose value is the boundary address
     *
     * The .quad directive emits 8 bytes initialised to the linker-script
     * symbol's value.  In a PIC .so, this produces an R_RISCV_64 dynamic
     * relocation that the dynamic linker patches at load time.
     */
    .macro DECLARE_SYM exported, internal
    .global \exported
    .type   \exported, @object
    .size   \exported, 8
    .align  3
\exported:
    .quad   \internal
    .endm


    /* ================================================================
     * .ulibtext (executable code) boundary addresses
     * ================================================================ */
    DECLARE_SYM __ULIBTEXT_VULDYN_BEGIN__,       _vuldyn_ulibtext_begin
    DECLARE_SYM __ULIBTEXT_VULDYN_END__,         _vuldyn_ulibtext_end
    DECLARE_SYM __ULIBTEXT_VULDYN_ENTRY_BEGIN__, _vuldyn_ulibtext_entry_begin
    DECLARE_SYM __ULIBTEXT_VULDYN_ENTRY_END__,   _vuldyn_ulibtext_entry_end
    DECLARE_SYM __ULIBTEXT_VULDYN_FUNC1_BEGIN__, _vuldyn_ulibtext_func1_begin
    DECLARE_SYM __ULIBTEXT_VULDYN_FUNC1_END__,   _vuldyn_ulibtext_func1_end
    DECLARE_SYM __ULIBTEXT_VULDYN_FUNC2_BEGIN__, _vuldyn_ulibtext_func2_begin
    DECLARE_SYM __ULIBTEXT_VULDYN_FUNC2_END__,   _vuldyn_ulibtext_func2_end
    DECLARE_SYM __ULIBTEXT_VULDYN_SHARE_BEGIN__, _vuldyn_ulibtext_share_begin
    DECLARE_SYM __ULIBTEXT_VULDYN_SHARE_END__,   _vuldyn_ulibtext_share_end

    /* ================================================================
     * .ulibrodata (read-only data) boundary addresses
     * ================================================================ */
    DECLARE_SYM __ULIBRODATA_VULDYN_BEGIN__,       _vuldyn_ulibrodata_begin
    DECLARE_SYM __ULIBRODATA_VULDYN_END__,         _vuldyn_ulibrodata_end
    DECLARE_SYM __ULIBRODATA_VULDYN_ENTRY_BEGIN__, _vuldyn_ulibrodata_entry_begin
    DECLARE_SYM __ULIBRODATA_VULDYN_ENTRY_END__,   _vuldyn_ulibrodata_entry_end
    DECLARE_SYM __ULIBRODATA_VULDYN_FUNC1_BEGIN__, _vuldyn_ulibrodata_func1_begin
    DECLARE_SYM __ULIBRODATA_VULDYN_FUNC1_END__,   _vuldyn_ulibrodata_func1_end
    DECLARE_SYM __ULIBRODATA_VULDYN_FUNC2_BEGIN__, _vuldyn_ulibrodata_func2_begin
    DECLARE_SYM __ULIBRODATA_VULDYN_FUNC2_END__,   _vuldyn_ulibrodata_func2_end
    DECLARE_SYM __ULIBRODATA_VULDYN_SHARE_BEGIN__, _vuldyn_ulibrodata_share_begin
    DECLARE_SYM __ULIBRODATA_VULDYN_SHARE_END__,   _vuldyn_ulibrodata_share_end

    /* ================================================================
     * .ulibdata (read-write initialized data) boundary addresses
     * ================================================================ */
    DECLARE_SYM __ULIBDATA_VULDYN_BEGIN__,       _vuldyn_ulibdata_begin
    DECLARE_SYM __ULIBDATA_VULDYN_END__,         _vuldyn_ulibdata_end
    DECLARE_SYM __ULIBDATA_VULDYN_ENTRY_BEGIN__, _vuldyn_ulibdata_entry_begin
    DECLARE_SYM __ULIBDATA_VULDYN_ENTRY_END__,   _vuldyn_ulibdata_entry_end
    DECLARE_SYM __ULIBDATA_VULDYN_FUNC1_BEGIN__, _vuldyn_ulibdata_func1_begin
    DECLARE_SYM __ULIBDATA_VULDYN_FUNC1_END__,   _vuldyn_ulibdata_func1_end
    DECLARE_SYM __ULIBDATA_VULDYN_FUNC2_BEGIN__, _vuldyn_ulibdata_func2_begin
    DECLARE_SYM __ULIBDATA_VULDYN_FUNC2_END__,   _vuldyn_ulibdata_func2_end
    DECLARE_SYM __ULIBDATA_VULDYN_SHARE_BEGIN__, _vuldyn_ulibdata_share_begin
    DECLARE_SYM __ULIBDATA_VULDYN_SHARE_END__,   _vuldyn_ulibdata_share_end

    /* ================================================================
     * .ulibbss (read-write zero-initialized data) boundary addresses
     * ================================================================ */
    DECLARE_SYM __ULIBBSS_VULDYN_BEGIN__,       _vuldyn_ulibbss_begin
    DECLARE_SYM __ULIBBSS_VULDYN_END__,         _vuldyn_ulibbss_end
    DECLARE_SYM __ULIBBSS_VULDYN_ENTRY_BEGIN__, _vuldyn_ulibbss_entry_begin
    DECLARE_SYM __ULIBBSS_VULDYN_ENTRY_END__,   _vuldyn_ulibbss_entry_end
    DECLARE_SYM __ULIBBSS_VULDYN_FUNC1_BEGIN__, _vuldyn_ulibbss_func1_begin
    DECLARE_SYM __ULIBBSS_VULDYN_FUNC1_END__,   _vuldyn_ulibbss_func1_end
    DECLARE_SYM __ULIBBSS_VULDYN_FUNC2_BEGIN__, _vuldyn_ulibbss_func2_begin
    DECLARE_SYM __ULIBBSS_VULDYN_FUNC2_END__,   _vuldyn_ulibbss_func2_end
    DECLARE_SYM __ULIBBSS_VULDYN_SHARE_BEGIN__, _vuldyn_ulibbss_share_begin
    DECLARE_SYM __ULIBBSS_VULDYN_SHARE_END__,   _vuldyn_ulibbss_share_end

    /* ================================================================
     * PLT and GOT boundary addresses
     * ================================================================ */
    DECLARE_SYM __VULDYN_PLT_BEGIN__, _vuldyn_plt_begin
    DECLARE_SYM __VULDYN_PLT_END__,   _vuldyn_plt_end
    DECLARE_SYM __VULDYN_GOT_BEGIN__, _vuldyn_got_begin
    DECLARE_SYM __VULDYN_GOT_END__,   _vuldyn_got_end

    /* ================================================================
     * Real function entry addresses used as FIT lookup keys
     * ================================================================ */
    DECLARE_SYM __VULDYN_ENTRY_ADDR__, entry
    DECLARE_SYM __VULDYN_FUNC1_ADDR__, func1
    DECLARE_SYM __VULDYN_FUNC2_ADDR__, func2
