/* TEMPLATE GENERATED TESTCASE FILE
Filename: CWE416_Use_After_Free__malloc_free_int_01.c
Label Definition File: CWE416_Use_After_Free__malloc_free.label.xml
Template File: sources-sinks-01.tmpl.c
*/
/*
 * @description
 * CWE: 416 Use After Free
 * BadSource:  Allocate data using malloc(), initialize memory block, and Deallocate data using free()
 * GoodSource: Allocate data using malloc() and initialize memory block
 * Sinks:
 *    GoodSink: Do nothing
 *    BadSink : Use data
 * Flow Variant: 01 Baseline
 *
 * */

#include "std_testcase.h"
#include "udasics.h"
#include "umaincall.h"
#include "dasics_stdarg.h"

#include <wchar.h>

// TODO: These macros can be moved into ucsrs.h
#define BOUND_REG_READ(hi,lo,idx)   \
        case idx:  \
            lo = csr_read(0x890 + idx * 2);  \
            hi = csr_read(0x891 + idx * 2);  \
            break;

#define BOUND_REG_WRITE(hi,lo,idx)   \
        case idx:  \
            csr_write(0x890 + idx * 2, lo);  \
            csr_write(0x891 + idx * 2, hi);  \
            break;

#define CONCAT(OP) BOUND_REG_##OP

#define LIBBOUND_LOOKUP(HI,LO,IDX,OP) \
        switch (IDX) \
        {               \
            CONCAT(OP)(HI,LO,0);  \
            CONCAT(OP)(HI,LO,1);  \
            CONCAT(OP)(HI,LO,2);  \
            CONCAT(OP)(HI,LO,3);  \
            CONCAT(OP)(HI,LO,4);  \
            CONCAT(OP)(HI,LO,5);  \
            CONCAT(OP)(HI,LO,6);  \
            CONCAT(OP)(HI,LO,7);  \
            CONCAT(OP)(HI,LO,8);  \
            CONCAT(OP)(HI,LO,9);  \
            CONCAT(OP)(HI,LO,10); \
            CONCAT(OP)(HI,LO,11); \
            CONCAT(OP)(HI,LO,12); \
            CONCAT(OP)(HI,LO,13); \
            CONCAT(OP)(HI,LO,14); \
            CONCAT(OP)(HI,LO,15); \
            default: \
                printf("\x1b[31m%s\x1b[0m","[DASICS]Error: out of libound register range\n"); \
        }

/* Use customized umaincall helper for this testcase */
enum UmaincallTypes
{
    Umaincall_MALLOC,
    Umaincall_FREE
};

static void juliet_umaincall_helper(struct umaincall * regs, ...)
{
    if (dasics_dynamic_call(regs)) { return; }

    uint64_t ret = 0;
    va_list args;
    UmaincallTypes type = (UmaincallTypes)regs->a0;
    va_start(args, regs);

    switch (type)
    {
    case Umaincall_MALLOC: {
        size_t size = va_arg(args, size_t);
        ret = (uint64_t)malloc(size);
        if ((void *)ret != NULL) {
            dasics_libcfg_alloc(DASICS_LIBCFG_R | DASICS_LIBCFG_W, ret, ret + size);
        }
        break;
    }
    case Umaincall_FREE: {
        void *base = va_arg(args, void *);
        uint64_t libcfg = csr_read(0x880);  // DasicsLibCfg
        int32_t max_cfgs = DASICS_LIBCFG_WIDTH;
        int32_t step = 4;
        for (int32_t idx = 0; idx < max_cfgs; ++idx) {
            uint64_t curr_cfg = (libcfg >> (idx * step)) & DASICS_LIBCFG_MASK;

            if ((curr_cfg & DASICS_LIBCFG_V) != 0) {
                uint64_t hi, lo;
                LIBBOUND_LOOKUP(hi, lo, idx, READ);
                if (lo == (uint64_t)base) {
                    dasics_libcfg_free(idx);
                }
            }
        }
        free(base);
        break;
    }
    default:
        printf("\x1b[33m Warning: Invalid umaincall number %d!\n\x1b[0m", type);
        break;
    }

    regs->t1 = regs->ra;  // make LibDASICS happy
    regs->a0 = ret;
    va_end(args);
}

#ifndef OMITBAD

void ATTR_ULIB_TEXT CWE416_Use_After_Free__malloc_free_int_01_bad()
{
    int * data;
    /* Initialize data */
    data = NULL;
    data = (int *)dasics_umaincall(Umaincall_MALLOC, 100*sizeof(int));
    if (data == NULL) { exit(-1); }
    {
        size_t i;
        for(i = 0; i < 100; i++)
        {
            data[i] = 5;
        }
    }
    /* POTENTIAL FLAW: Free data in the source - the bad sink attempts to use data */
    dasics_umaincall(Umaincall_FREE, data);
    /* POTENTIAL FLAW: Use of data that may have been freed */
    printf("%d\n", data[0]);
    /* POTENTIAL INCIDENTAL - Possible memory leak here if data was not freed */
}

#endif /* OMITBAD */

#ifndef OMITGOOD

/* goodG2B uses the GoodSource with the BadSink */
static void goodG2B()
{
    int * data;
    /* Initialize data */
    data = NULL;
    data = (int *)malloc(100*sizeof(int));
    if (data == NULL) {exit(-1);}
    {
        size_t i;
        for(i = 0; i < 100; i++)
        {
            data[i] = 5;
        }
    }
    /* FIX: Do not free data in the source */
    /* POTENTIAL FLAW: Use of data that may have been freed */
    printIntLine(data[0]);
    /* POTENTIAL INCIDENTAL - Possible memory leak here if data was not freed */
}

/* goodB2G uses the BadSource with the GoodSink */
static void goodB2G()
{
    int * data;
    /* Initialize data */
    data = NULL;
    data = (int *)malloc(100*sizeof(int));
    if (data == NULL) {exit(-1);}
    {
        size_t i;
        for(i = 0; i < 100; i++)
        {
            data[i] = 5;
        }
    }
    /* POTENTIAL FLAW: Free data in the source - the bad sink attempts to use data */
    free(data);
    /* FIX: Don't use data that may have been freed already */
    /* POTENTIAL INCIDENTAL - Possible memory leak here if data was not freed */
    /* do nothing */
    ; /* empty statement needed for some flow variants */
}

void CWE416_Use_After_Free__malloc_free_int_01_good()
{
    goodG2B();
    goodB2G();
}

#endif /* OMITGOOD */

/* Below is the main(). It is only used when building this testcase on
   its own for testing or for building a binary to use in testing binary
   analysis tools. It is not used when compiling all the testcases as one
   application, which is how source code analysis tools are tested. */

#ifdef INCLUDEMAIN

int main(int argc, char * argv[])
{
    /* set customized umaincall helper */
    register_udasics((uint64_t)juliet_umaincall_helper);

    /* seed randomness */
    srand( (unsigned)time(NULL) );
#ifndef OMITGOOD
    printLine("Calling good()...");
    CWE416_Use_After_Free__malloc_free_int_01_good();
    printLine("Finished good()");
#endif /* OMITGOOD */
#ifndef OMITBAD
    printLine("Calling bad()...");

    // Allocate jump bound for .ulibtext section
    extern char __ULIBTEXT_BEGIN__, __ULIBTEXT_END__;
    int idx_ulibtext = dasics_jumpcfg_alloc((uint64_t)&__ULIBTEXT_BEGIN__, (uint64_t)&__ULIBTEXT_END__);

    // Allocate stack permission for this bad function
    uint64_t frame_addr, badfunc_stack_top;
    asm volatile("mv %0, sp" : "=r"(frame_addr));
    badfunc_stack_top = frame_addr - 0x8;  // 0x8 is the stack size of lib_call
    int idx_stack = dasics_libcfg_alloc(DASICS_LIBCFG_R | DASICS_LIBCFG_W, \
        badfunc_stack_top - 32, badfunc_stack_top);

    // Call the vulnerable bad function
    lib_call(&CWE416_Use_After_Free__malloc_free_int_01_bad);

    // Release allocated permissions
    dasics_libcfg_free(idx_stack);
    dasics_jumpcfg_free(idx_ulibtext);

    printLine("Finished bad()");
#endif /* OMITBAD */

    unregister_udasics();
    return 0;
}

#endif
