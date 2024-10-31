/* TEMPLATE GENERATED TESTCASE FILE
Filename: CWE126_Buffer_Overread__CWE129_fgets_01.c
Label Definition File: CWE126_Buffer_Overread__CWE129.label.xml
Template File: sources-sinks-01.tmpl.c
*/
/*
 * @description
 * CWE: 126 Buffer Overread
 * BadSource: fgets Read data from the console using fgets()
 * GoodSource: Larger than zero but less than 10
 * Sinks:
 *    GoodSink: Ensure the array index is valid
 *    BadSink : Improperly check the array index by not checking the upper bound
 * Flow Variant: 01 Baseline
 *
 * */

#include "std_testcase.h"
#include "udasics.h"
#include "umaincall.h"
#include "dasics_stdarg.h"

#define CHAR_ARRAY_SIZE (3 * sizeof(data) + 2)

/* Use customized umaincall helper for this testcase */
enum UmaincallTypes
{
    Umaincall_PRINTF,
    Umaincall_FGETS,
    Umaincall_ATOI,
    Umaincall_PRINTLINE,
    Umaincall_PRINTINTLINE
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
    case Umaincall_PRINTF:
        ret = vprintf(va_arg(args, const char *), args);
        break;
    case Umaincall_FGETS:
        ret = (uint64_t)fgets(va_arg(args, char *), va_arg(args, int), va_arg(args, FILE *));
        break;
    case Umaincall_ATOI:
        ret = (uint64_t)atoi(va_arg(args, char *));
        break;
    case Umaincall_PRINTLINE:
        printLine(va_arg(args, const char *));
        break;
    case Umaincall_PRINTINTLINE:
        printIntLine(va_arg(args, int));
        break;
    default:
        printf("\x1b[33m Warning: Invalid umaincall number %d!\n\x1b[0m", type);
        break;
    }

    regs->t1 = regs->ra;  // make LibDASICS happy
    regs->a0 = ret;
    va_end(args);
}

#ifndef OMITBAD

void ATTR_ULIB_TEXT CWE126_Buffer_Overread__CWE129_fgets_01_bad()
{
    int data;
    /* Initialize data */
    data = -1;
    {
        char inputBuffer[CHAR_ARRAY_SIZE] = "";
        /* POTENTIAL FLAW: Read data from the console using fgets() */
        if ((char *)dasics_umaincall(Umaincall_FGETS, inputBuffer, CHAR_ARRAY_SIZE, stdin) != NULL)
        {
            /* Convert to int */
            data = (int)dasics_umaincall(Umaincall_ATOI, inputBuffer);
        }
        else
        {
            dasics_umaincall(Umaincall_PRINTLINE, "fgets() failed.");
        }
    }
    {
        int buffer[10] = { 0 };
        /* POTENTIAL FLAW: Attempt to access an index of the array that is above the upper bound
         * This check does not check the upper bounds of the array index */
        if (data >= 0)
        {
            dasics_umaincall(Umaincall_PRINTINTLINE, buffer[data]);
        }
        else
        {
            dasics_umaincall(Umaincall_PRINTLINE, "ERROR: Array index is negative");
        }
    }
}

#endif /* OMITBAD */

#ifndef OMITGOOD

/* goodG2B uses the GoodSource with the BadSink */
static void goodG2B()
{
    int data;
    /* Initialize data */
    data = -1;
    /* FIX: Use a value greater than 0, but less than 10 to avoid attempting to
     * access an index of the array in the sink that is out-of-bounds */
    data = 7;
    {
        int buffer[10] = { 0 };
        /* POTENTIAL FLAW: Attempt to access an index of the array that is above the upper bound
         * This check does not check the upper bounds of the array index */
        if (data >= 0)
        {
            printIntLine(buffer[data]);
        }
        else
        {
            printLine("ERROR: Array index is negative");
        }
    }
}

/* goodB2G uses the BadSource with the GoodSink */
static void goodB2G()
{
    int data;
    /* Initialize data */
    data = -1;
    {
        char inputBuffer[CHAR_ARRAY_SIZE] = "";
        /* POTENTIAL FLAW: Read data from the console using fgets() */
        if (fgets(inputBuffer, CHAR_ARRAY_SIZE, stdin) != NULL)
        {
            /* Convert to int */
            data = atoi(inputBuffer);
        }
        else
        {
            printLine("fgets() failed.");
        }
    }
    {
        int buffer[10] = { 0 };
        /* FIX: Properly validate the array index and prevent a buffer overread */
        if (data >= 0 && data < (10))
        {
            printIntLine(buffer[data]);
        }
        else
        {
            printLine("ERROR: Array index is out-of-bounds");
        }
    }
}

void CWE126_Buffer_Overread__CWE129_fgets_01_good()
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
    CWE126_Buffer_Overread__CWE129_fgets_01_good();
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
        badfunc_stack_top - 0x50, badfunc_stack_top);

    // Allocate global variable permission (stdin)
    uint64_t gp_val;
    asm volatile("mv %0, gp" : "=r"(gp_val));
    int idx_stdin = dasics_libcfg_alloc(DASICS_LIBCFG_R, \
        gp_val - 1816, gp_val - 1816 + sizeof(FILE *));

    // Call the vulnerable bad function
    lib_call(&CWE126_Buffer_Overread__CWE129_fgets_01_bad);

    // Release allocated permissions
    dasics_libcfg_free(idx_stdin);
    dasics_libcfg_free(idx_stack);
    dasics_jumpcfg_free(idx_ulibtext);

    printLine("Finished bad()");
#endif /* OMITBAD */

    unregister_udasics();
    return 0;
}

#endif
