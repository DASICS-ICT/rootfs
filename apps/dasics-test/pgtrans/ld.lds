/* pgtrans: func1, func2 */
OUTPUT_FORMAT("elf64-littleriscv", "elf64-littleriscv",
	      "elf64-littleriscv")
OUTPUT_ARCH(riscv)
ENTRY(_check_dasics)
SEARCH_DIR("=/usr/local/lib64/lp64d"); SEARCH_DIR("=/usr/local/lib64"); SEARCH_DIR("=/lib64/lp64d"); SEARCH_DIR("=/lib64"); SEARCH_DIR("=/usr/lib64/lp64d"); SEARCH_DIR("=/usr/lib64"); SEARCH_DIR("=/usr/local/lib"); SEARCH_DIR("=/lib"); SEARCH_DIR("=/usr/lib");
SECTIONS
{
  PROVIDE (__executable_start = SEGMENT_START("text-segment", 0x1000000)); . = SEGMENT_START("text-segment", 0x1000000) + SIZEOF_HEADERS;
  .plt            : { _plt_start = . ; *(.plt) *(.iplt) _plt_end = . ; }
  _text_start = . ;
  .text           :
  {
    *(.text.unlikely .text.*_unlikely .text.unlikely.*)
    *(.text.exit .text.exit.*)
    *(.text.startup .text.startup.*)
    *(.text.hot .text.hot.*)
    *(SORT(.text.sorted.*))
    *(.text .stub .text.* .gnu.linkonce.t.*)
    *(.gnu.warning)
  }
  _text_end = . ;
  . = ALIGN(64 / 8);
  . = .;
  . = ALIGN(64 / 8);
  PROVIDE (__ULIBTEXT_BEGIN__ = .);
  .ulibtext :
  {
    __ULIBTEXT_FUNC1_BEGIN__ = .;
    *(.ulibtext.func1)
    __ULIBTEXT_FUNC1_END__ = .;
    QUAD(0xdeadbeef)
    __ULIBTEXT_FUNC2_BEGIN__ = .;
    *(.ulibtext.func2)
    __ULIBTEXT_FUNC2_END__ = .;
  }
  PROVIDE (__ULIBTEXT_END__ = .);
  . = ALIGN(64 / 8);
  .ufreezonetext : { __ULIBFREEZONETEXT_BEGIN__ = .; *(.ufreezonetext) __ULIBFREEZONETEXT_END__ = .; }
  .interp : { _interp_start = . ; *(.interp) }
  .note.gnu.build-id  : { *(.note.gnu.build-id) }
  .hash : { *(.hash) }
  .gnu.hash : { *(.gnu.hash) }
  .dynsym : { *(.dynsym) }
  .dynstr : { *(.dynstr) }
  .gnu.version : { *(.gnu.version) }
  .gnu.version_d : { *(.gnu.version_d) }
  .gnu.version_r : { *(.gnu.version_r) }
  .rela.dyn : { *(.rela.init) *(.rela.text .rela.text.*) *(.rela.fini) *(.rela.rodata .rela.rodata.*) *(.rela.data .rela.data.*) *(.rela.tdata .rela.tdata.*) *(.rela.tbss .rela.tbss.*) *(.rela.ctors) *(.rela.dtors) *(.rela.got) *(.rela.sdata .rela.sdata.*) *(.rela.sbss .rela.sbss.*) *(.rela.sdata2 .rela.sdata2.*) *(.rela.sbss2 .rela.sbss2.*) *(.rela.bss .rela.bss.*) *(.rela.ifunc) }
  .rela.plt : { *(.rela.plt) PROVIDE_HIDDEN (__rela_iplt_start = .); *(.rela.iplt) PROVIDE_HIDDEN (__rela_iplt_end = .); }
  .init : { KEEP (*(SORT_NONE(.init))) }
  .fini : { KEEP (*(SORT_NONE(.fini))) }
  PROVIDE (__etext = .);
  PROVIDE (_etext = .);
  PROVIDE (etext = .);
  PROVIDE (__RODATA_BEGIN__ = .);
  .rodata : { *(.rodata .rodata.* .gnu.linkonce.r.*) }
  .rodata1 : { *(.rodata1) }
  PROVIDE (__RODATA_END__ = .);
  .sdata2 : { *(.sdata2 .sdata2.* .gnu.linkonce.s2.*) }
  .sbss2 : { *(.sbss2 .sbss2.* .gnu.linkonce.sb2.*) }
  .eh_frame_hdr : { *(.eh_frame_hdr) *(.eh_frame_entry .eh_frame_entry.*) }
  .eh_frame : ONLY_IF_RO { KEEP (*(.eh_frame)) *(.eh_frame.*) }
  .gcc_except_table : ONLY_IF_RO { *(.gcc_except_table .gcc_except_table.*) }
  .gnu_extab : ONLY_IF_RO { *(.gnu_extab*) }
  .exception_ranges : ONLY_IF_RO { *(.exception_ranges*) }
  . = DATA_SEGMENT_ALIGN (CONSTANT (MAXPAGESIZE), CONSTANT (COMMONPAGESIZE));
  .eh_frame : ONLY_IF_RW { KEEP (*(.eh_frame)) *(.eh_frame.*) }
  .gnu_extab : ONLY_IF_RW { *(.gnu_extab) }
  .gcc_except_table : ONLY_IF_RW { *(.gcc_except_table .gcc_except_table.*) }
  .exception_ranges : ONLY_IF_RW { *(.exception_ranges*) }
  .tdata : { PROVIDE_HIDDEN (__tdata_start = .); *(.tdata .tdata.* .gnu.linkonce.td.*) }
  .tbss : { *(.tbss .tbss.* .gnu.linkonce.tb.*) *(.tcommon) }
  .preinit_array : { PROVIDE_HIDDEN (__preinit_array_start = .); KEEP (*(.preinit_array)) PROVIDE_HIDDEN (__preinit_array_end = .); }
  .init_array : { PROVIDE_HIDDEN (__init_array_start = .); KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*))) KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors)) PROVIDE_HIDDEN (__init_array_end = .); }
  .fini_array : { PROVIDE_HIDDEN (__fini_array_start = .); KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*))) KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors)) PROVIDE_HIDDEN (__fini_array_end = .); }
  .ctors : { KEEP (*crtbegin.o(.ctors)) KEEP (*crtbegin?.o(.ctors)) KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors)) KEEP (*(SORT(.ctors.*))) KEEP (*(.ctors)) }
  .dtors : { KEEP (*crtbegin.o(.dtors)) KEEP (*crtbegin?.o(.dtors)) KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors)) KEEP (*(SORT(.dtors.*))) KEEP (*(.dtors)) }
  .jcr : { KEEP (*(.jcr)) }
  .data.rel.ro : { *(.data.rel.ro.local* .gnu.linkonce.d.rel.ro.local.*) *(.data.rel.ro .data.rel.ro.* .gnu.linkonce.d.rel.ro.*) }
  .dynamic : { *(.dynamic) }
  . = DATA_SEGMENT_RELRO_END (0, .);
  .data : { __DATA_BEGIN__ = .; *(.data .data.* .gnu.linkonce.d.*) SORT(CONSTRUCTORS) }
  .data1 : { *(.data1) }
  . = ALIGN(64 / 8);
  . = .;
  .got : { _got_start = . ; *(.got.plt) *(.igot.plt) *(.got) *(.igot) _got_end = . ; }
  .sdata : { __SDATA_BEGIN__ = .; *(.srodata.cst16) *(.srodata.cst8) *(.srodata.cst4) *(.srodata.cst2) *(.srodata .srodata.*) *(.sdata .sdata.* .gnu.linkonce.s.*) }
  .ulibrodata :
  {
    __ULIBRODATA_FUNC1_BEGIN__ = .;
    *(.ulibrodata.func1)
    __ULIBRODATA_FUNC1_END__ = .;
    QUAD(0xdeadbeef)
    __ULIBRODATA_FUNC2_BEGIN__ = .;
    *(.ulibrodata.func2)
    __ULIBRODATA_FUNC2_END__ = .;
  }
  .ulibdata :
  {
    __ULIBDATA_FUNC1_BEGIN__ = .;
    *(.ulibdata.func1)
    __ULIBDATA_FUNC1_END__ = .;
    QUAD(0xdeadbeef)
    __ULIBDATA_FUNC2_BEGIN__ = .;
    *(.ulibdata.func2)
    __ULIBDATA_FUNC2_END__ = .;
  }
  _edata = .; PROVIDE (edata = .);
  . = .;
  __bss_start = .;
  .sbss : { *(.dynsbss) *(.sbss .sbss.* .gnu.linkonce.sb.*) *(.scommon) }
  .ulibbss :
  {
    __ULIBBSS_FUNC1_BEGIN__ = .;
    *(.ulibbss.func1)
    __ULIBBSS_FUNC1_END__ = .;
    QUAD(0xdeadbeef)
    __ULIBBSS_FUNC2_BEGIN__ = .;
    *(.ulibbss.func2)
    __ULIBBSS_FUNC2_END__ = .;
  }
  .bss : { *(.dynbss) *(.bss .bss.* .gnu.linkonce.b.*) *(COMMON) . = ALIGN(. != 0 ? 64 / 8 : 1); }
  . = ALIGN(64 / 8);
  . = SEGMENT_START("ldata-segment", .);
  . = ALIGN(64 / 8);
  __BSS_END__ = .;
  __global_pointer$ = MIN(__SDATA_BEGIN__ + 0x800, MAX(__DATA_BEGIN__ + 0x800, __BSS_END__ - 0x800));
  _end = .; PROVIDE (end = .);
  . = DATA_SEGMENT_END (.);
  .stab 0 : { *(.stab) }
  .stabstr 0 : { *(.stabstr) }
  .comment 0 : { *(.comment) }
  .gnu.attributes 0 : { KEEP (*(.gnu.attributes)) }
  /DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*) }
}
