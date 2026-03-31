/*
 * main.c -- Trusted main program for the vuldyn dynamic library test.
 *
 * This program is the trusted entry point that:
 *   1. Initialises the FIT (Function Isolation Table) subsystem.
 *   2. Prints the compartment configuration for debugging.
 *   3. Performs a domain switch into the entry() function of libvuldyn.so,
 *      which then orchestrates the complete test sequence.
 *   4. Tears down the FIT subsystem on return.
 *
 * The entry() function lives in libvuldyn.so and is resolved by the
 * dynamic linker at program startup.  All untrusted code executes within
 * DASICS-protected compartments; this main() runs in the trusted domain.
 */

#include <stdio.h>
#include <stdlib.h>
#include "udasics.h"
#include "fit.h"

extern void entry(void);

/*
 * exit_function() -- atexit handler.
 * Prints a final message to confirm clean shutdown, regardless of whether
 * the test completed normally or was terminated by a fault handler.
 */
void exit_function(void)
{
    printf("[MAIN] vuldyn test finished\n");
}

int main(void)
{
    atexit(exit_function);

    printf("[MAIN] Dynamic library comprehensive test (vuldyn)\n");

    /* Print the compartment table for debugging / verification */
    fit_print();

    /*
     * Domain-switch into the library's entry() function.
     * This activates the entry compartment (auto-assigned closure_id, full bounds)
     * and transfers control to the untrusted code in libvuldyn.so.
     *
     * entry() will subsequently switch into func1 and func2, exercising
     * all DASICS isolation features (rwx, share, heap, pgtrans, argbound,
     * syscall, maincall).
     */
    fit_switchto(entry);

    return 0;
}
