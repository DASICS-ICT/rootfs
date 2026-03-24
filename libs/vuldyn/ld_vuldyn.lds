/*
 * ld_vuldyn.lds -- Supplemental linker script for libvuldyn.so
 *
 * This script uses INSERT AFTER directives to augment the default shared
 * library linker script.  It adds custom DASICS sections (.ulibtext,
 * .ulibrodata, .ulibdata, .ulibbss) with per-function boundary symbols.
 *
 * IMPORTANT: The symbols defined here (e.g. _vuldyn_ulibtext_begin) are
 * INTERNAL to the .so.  They are linker-script address symbols whose ELF
 * value equals the section boundary address.  These symbols are NOT
 * directly exported to the main executable because copy relocation would
 * break them -- see vuldyn_syms.S for the exported data wrappers.
 *
 * Section layout within each custom segment:
 *   entry -> func1 -> func2 -> share
 * separated by QUAD(0xdeadbeef) sentinels to ensure non-overlapping
 * address ranges.
 *
 * PLT and GOT boundary symbols are also defined here for compartment
 * configuration.
 */

SECTIONS
{
  /*
   * .ulibtext -- Custom executable code sections.
   * Placed after the standard .text section in the .so.
   */
  .ulibtext ALIGN(8) :
  {
    _vuldyn_ulibtext_begin = .;

    _vuldyn_ulibtext_entry_begin = .;
    *(.ulibtext.entry)
    _vuldyn_ulibtext_entry_end = .;

    /* Force a page break between sub-zones to tolerate coarse HW granularity. */
    . = ALIGN(0x1000);

    _vuldyn_ulibtext_func1_begin = .;
    *(.ulibtext.func1)
    _vuldyn_ulibtext_func1_end = .;

    . = ALIGN(0x1000);

    _vuldyn_ulibtext_func2_begin = .;
    *(.ulibtext.func2)
    _vuldyn_ulibtext_func2_end = .;

    . = ALIGN(0x1000);

    _vuldyn_ulibtext_share_begin = .;
    *(.ulibtext.share)
    _vuldyn_ulibtext_share_end = .;

    _vuldyn_ulibtext_end = .;
  }
}
INSERT AFTER .text;

SECTIONS
{
  /*
   * .ulibrodata -- Custom read-only data sections.
   */
  .ulibrodata ALIGN(8) :
  {
    _vuldyn_ulibrodata_begin = .;

    _vuldyn_ulibrodata_entry_begin = .;
    *(.ulibrodata.entry)
    _vuldyn_ulibrodata_entry_end = .;

    . = ALIGN(0x1000);

    _vuldyn_ulibrodata_func1_begin = .;
    *(.ulibrodata.func1)
    _vuldyn_ulibrodata_func1_end = .;

    . = ALIGN(0x1000);

    _vuldyn_ulibrodata_func2_begin = .;
    *(.ulibrodata.func2)
    _vuldyn_ulibrodata_func2_end = .;

    . = ALIGN(0x1000);

    _vuldyn_ulibrodata_share_begin = .;
    *(.ulibrodata.share)
    _vuldyn_ulibrodata_share_end = .;

    _vuldyn_ulibrodata_end = .;
  }
}
INSERT AFTER .rodata;

SECTIONS
{
  /*
   * .ulibdata -- Custom read-write initialized data sections.
   */
  .ulibdata ALIGN(8) :
  {
    _vuldyn_ulibdata_begin = .;

    _vuldyn_ulibdata_entry_begin = .;
    *(.ulibdata.entry)
    _vuldyn_ulibdata_entry_end = .;

    . = ALIGN(0x1000);

    _vuldyn_ulibdata_func1_begin = .;
    *(.ulibdata.func1)
    _vuldyn_ulibdata_func1_end = .;

    . = ALIGN(0x1000);

    _vuldyn_ulibdata_func2_begin = .;
    *(.ulibdata.func2)
    _vuldyn_ulibdata_func2_end = .;

    . = ALIGN(0x1000);

    _vuldyn_ulibdata_share_begin = .;
    *(.ulibdata.share)
    _vuldyn_ulibdata_share_end = .;

    _vuldyn_ulibdata_end = .;
  }
}
INSERT AFTER .data;

SECTIONS
{
  /*
   * .ulibbss -- Custom read-write zero-initialized data sections.
   */
  .ulibbss ALIGN(8) :
  {
    _vuldyn_ulibbss_begin = .;

    _vuldyn_ulibbss_entry_begin = .;
    *(.ulibbss.entry)
    _vuldyn_ulibbss_entry_end = .;

    . = ALIGN(0x1000);

    _vuldyn_ulibbss_func1_begin = .;
    *(.ulibbss.func1)
    _vuldyn_ulibbss_func1_end = .;

    . = ALIGN(0x1000);

    _vuldyn_ulibbss_func2_begin = .;
    *(.ulibbss.func2)
    _vuldyn_ulibbss_func2_end = .;

    . = ALIGN(0x1000);

    _vuldyn_ulibbss_share_begin = .;
    *(.ulibbss.share)
    _vuldyn_ulibbss_share_end = .;

    _vuldyn_ulibbss_end = .;
  }
}
INSERT AFTER .bss;

SECTIONS
{
  /*
   * PLT and GOT internal boundary symbols.
   */
  _vuldyn_plt_begin = ADDR(.plt);
  _vuldyn_plt_end   = ADDR(.plt) + SIZEOF(.plt);
  _vuldyn_got_begin = ADDR(.got);
  _vuldyn_got_end   = ADDR(.got.plt) + SIZEOF(.got.plt);
}
