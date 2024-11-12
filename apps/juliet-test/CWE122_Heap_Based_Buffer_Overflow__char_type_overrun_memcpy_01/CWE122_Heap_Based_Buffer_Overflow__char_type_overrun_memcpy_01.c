/* TEMPLATE GENERATED TESTCASE FILE
Filename: CWE122_Heap_Based_Buffer_Overflow__char_type_overrun_memcpy_01.c
Label Definition File: CWE122_Heap_Based_Buffer_Overflow.label.xml
Template File: point-flaw-01.tmpl.c
*/
/*
 * @description
 * CWE: 122 Heap Based Buffer Overflow
 * Sinks: type_overrun_memcpy
 *    GoodSink: Perform the memcpy() and prevent overwriting part of the structure
 *    BadSink : Overwrite part of the structure by incorrectly using the sizeof(struct) in memcpy()
 * Flow Variant: 01 Baseline
 *
 * */

#include "std_testcase.h"
#include "udasics.h"
#include "umaincall.h"
#include "dasics_stdarg.h"
#include "udirect.h"
#include "utstack.h"

#ifndef _WIN32
#include <wchar.h>
#endif

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
    Umaincall_FREE,
    Umaincall_SHRINK,
    Umaincall_RECOVER,
};

struct item {
    uint8_t      orig_idx;
    uint8_t      priv;
    uint64_t     lo;
    uint64_t     hi;
    struct item *next;
};

static struct item *priv_stack = NULL;

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
        int *idx = va_arg(args, int *);
        ret = (uint64_t)malloc(size);
        if ((void *)ret != NULL) {
            *idx = dasics_libcfg_alloc(DASICS_LIBCFG_R | DASICS_LIBCFG_W, ret, ret + size);
        }
        break;
    }
    case Umaincall_FREE: {
        void *base = va_arg(args, void *);
        int idx = va_arg(args, int);
        dasics_libcfg_free(idx);
        free(base);
        break;
    }
    case Umaincall_SHRINK: {
        uint64_t newlo = va_arg(args, uint64_t);
        uint64_t newhi = va_arg(args, uint64_t);
        int *idx = va_arg(args, int *);

        uint64_t priv = dasics_libcfg_get(*idx);
        uint64_t lo, hi;
        LIBBOUND_LOOKUP(hi, lo, *idx, READ);
        if ((priv & DASICS_LIBCFG_V) && lo <= newlo && newhi <= hi) {
            dasics_libcfg_free(*idx);
            *idx = dasics_libcfg_alloc(priv, newlo, newhi);
            struct item *new_item = (struct item *)malloc(sizeof(struct item));
            new_item->orig_idx = *idx;
            new_item->priv = priv;
            new_item->lo = lo;
            new_item->hi = hi;
            STACK_PUSH(priv_stack, new_item);
        }
        else {
            printf("ERROR: Shrink failed\n");
        }
        break;
    }
    case Umaincall_RECOVER: {
        int *idx = va_arg(args, int *);

        if (STACK_EMPTY(priv_stack)) {
            printf("ERROR: Recover failed due to empty priv stack\n");
        }
        else if (STACK_TOP(priv_stack)->orig_idx != *idx) {
            printf("ERROR: Recover failed due to unmatched priv stack\n");
        }
        else {
            dasics_libcfg_free(*idx);
            struct item *top = NULL;
            STACK_POP(priv_stack, top);
            *idx = dasics_libcfg_alloc(top->priv, top->lo, top->hi);
            free(top);
        }
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

#define SRC_STR "0123456789abcdef0123456789abcde"

typedef struct _charVoid
{
    char charFirst[16];
    void * voidSecond;
    void * voidThird;
} charVoid;

#ifndef OMITBAD

void ATTR_ULIB_TEXT CWE122_Heap_Based_Buffer_Overflow__char_type_overrun_memcpy_01_bad()
{
    {
        int idx_structCharVoid;
        charVoid * structCharVoid = (charVoid *)dasics_umaincall(Umaincall_MALLOC, sizeof(charVoid), &idx_structCharVoid);
        if (structCharVoid == NULL) {exit(-1);}
        structCharVoid->voidSecond = (void *)SRC_STR;
        /* Print the initial block pointed to by structCharVoid->voidSecond */
        printf("%s\n", (char *)structCharVoid->voidSecond);
        /* FLAW: Use the sizeof(*structCharVoid) which will overwrite the pointer y */
        dasics_umaincall(Umaincall_SHRINK, structCharVoid, &structCharVoid->voidSecond, &idx_structCharVoid);
        memcpy(structCharVoid->charFirst, SRC_STR, sizeof(*structCharVoid));
        dasics_umaincall(Umaincall_RECOVER, &idx_structCharVoid);
        structCharVoid->charFirst[(sizeof(structCharVoid->charFirst)/sizeof(char))-1] = '\0'; /* null terminate the string */
        printf("%s\n", (char *)structCharVoid->charFirst);
        printf("%s\n", (char *)structCharVoid->voidSecond);
        dasics_umaincall(Umaincall_FREE, structCharVoid, idx_structCharVoid);
    }
}

#endif /* OMITBAD */

#ifndef OMITGOOD

static void good1()
{
    {
        charVoid * structCharVoid = (charVoid *)malloc(sizeof(charVoid));
        if (structCharVoid == NULL) {exit(-1);}
        structCharVoid->voidSecond = (void *)SRC_STR;
        /* Print the initial block pointed to by structCharVoid->voidSecond */
        printLine((char *)structCharVoid->voidSecond);
        /* FIX: Use the sizeof(structCharVoid->charFirst) to avoid overwriting the pointer y */
        memcpy(structCharVoid->charFirst, SRC_STR, sizeof(structCharVoid->charFirst));
        structCharVoid->charFirst[(sizeof(structCharVoid->charFirst)/sizeof(char))-1] = '\0'; /* null terminate the string */
        printLine((char *)structCharVoid->charFirst);
        printLine((char *)structCharVoid->voidSecond);
        free(structCharVoid);
    }
}

void CWE122_Heap_Based_Buffer_Overflow__char_type_overrun_memcpy_01_good()
{
    good1();
}

#endif /* OMITGOOD */

/* Below is the main(). It is only used when building this testcase on
   its own for testing or for building a binary to use in testing binary
   analysis tools. It is not used when compiling all the testcases as one
   application, which is how source code analysis tools are tested. */

#ifdef INCLUDEMAIN

int main(int argc, char * argv[])
{
    register_udasics((uint64_t)juliet_umaincall_helper);

    /* seed randomness */
    srand( (unsigned)time(NULL) );
#ifndef OMITGOOD
    printLine("Calling good()...");
    CWE122_Heap_Based_Buffer_Overflow__char_type_overrun_memcpy_01_good();
    printLine("Finished good()");
#endif /* OMITGOOD */
#ifndef OMITBAD
    printLine("Calling bad()...");

    // Allocate jump bound for .ulibtext section
    extern char __ULIBTEXT_BEGIN__, __ULIBTEXT_END__;
    int idx_ulibtext = dasics_jumpcfg_alloc((uint64_t)&__ULIBTEXT_BEGIN__, (uint64_t)&__ULIBTEXT_END__);

    // Allocate .rodata permission for this bad function
    extern char __RODATA_BEGIN__, __RODATA_END__;
    int idx_rodata = dasics_libcfg_alloc(DASICS_LIBCFG_R, (uint64_t)&__RODATA_BEGIN__, (uint64_t)&__RODATA_END__);

    // Allocate stack permission for this bad function
    uint64_t frame_addr, badfunc_stack_top;
    asm volatile("mv %0, sp" : "=r"(frame_addr));
    badfunc_stack_top = frame_addr - 0x8;  // 0x8 is the stack size of lib_call
    int idx_stack = dasics_libcfg_alloc(DASICS_LIBCFG_R | DASICS_LIBCFG_W, \
        badfunc_stack_top - 32, badfunc_stack_top);

    // Redirect memcpy to untrusted zone
    add_redirect_item("memcpy"); // memcpy
    open_redirect();

    // Call the vulnerable bad function
    lib_call(&CWE122_Heap_Based_Buffer_Overflow__char_type_overrun_memcpy_01_bad);

    // Delete memcpy redirect
    close_redirect();
    delete_redirect_item("memcpy");

    // Release allocated permissions
    dasics_libcfg_free(idx_stack);
    dasics_libcfg_free(idx_rodata);
    dasics_jumpcfg_free(idx_ulibtext);

    printLine("Finished bad()");
#endif /* OMITBAD */

    unregister_udasics();
    return 0;
}

#endif
