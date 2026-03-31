/*
 * Permission grant test: func1 allocates a buffer via mimalloc, writes
 * "Hello Func2!", grants READ permission (16 bytes) to func2, then
 * domain-switches into func2. func2 reads and prints the string (legal),
 * then tries to read beyond the granted range (illegal -> DASICS fault).
 * After func2 returns, func1 frees the buffer and returns to main.
 */
#include <stdio.h>
#include <stdint.h>

#include "udasics.h"
#include "fit.h"

const char *test_info = "[MAIN] Permission grant test: func1 grants read permission to func2\n";

static const char hello_msg[] __attribute__((section(".ulibrodata.func1"))) = "Hello Func2!";

/* ---- func2 (untrusted, .ulibtext.func2) ---- */

void __attribute__((section(".ulibtext.func2"))) func2(char *buffer)
{
    dasics_umaincall(Umaincall_PRINT, "[func2] start\n");

    /* Legal: read the granted string and print it */
    // FIXME: fine-grained syscall argument check is not implemented yet!
    dasics_umaincall(Umaincall_PRINT, "[func2] read: %s\n", buffer);

    /* Legal(1st transition) / Illegal(2nd transition) access to the buffer */
    dasics_umaincall(Umaincall_PRINT, "[func2] attempting access to the buffer...\n");
    buffer[0] = 'B';
    volatile char __attribute__((unused)) c1 = buffer[0];

    /* Illegal: read beyond the 16-byte granted range -> DasicsULoadAccessFault */
    dasics_umaincall(Umaincall_PRINT, "[func2] attempting illegal access beyond granted range...\n");
    volatile char __attribute__((unused)) c2 = buffer[100];

    dasics_umaincall(Umaincall_PRINT, "[func2] end\n");
}

/* ---- func1 (untrusted, .ulibtext.func1) ---- */

void __attribute__((section(".ulibtext.func1"))) func1(void)
{
    dasics_umaincall(Umaincall_PRINT, "[func1] start\n");

    /* Allocate buffer on self-managed heap via trusted maincall */
    size_t alloc_size = 64;
    char *buffer = (char *)(uintptr_t)dasics_umaincall(Umaincall_MALLOC, alloc_size);
    if (!buffer) {
        dasics_umaincall(Umaincall_PRINT, "[func1] MALLOC failed\n");
        return;
    }
    dasics_umaincall(Umaincall_PRINT, "[func1] allocated buffer\n");

    /* Copy hello_msg into the buffer (hello_msg is in .ulibrodata.func1) */
    int i = 0;
    while (hello_msg[i] != '\0') {
        buffer[i] = hello_msg[i];
        i++;
    }
    buffer[i] = '\0';
    dasics_umaincall(Umaincall_PRINT, "[func1] wrote: %s\n", buffer);

    /* Grant READ permission on the first 16 bytes of buffer to func2 */
    fit_bounds_t perms[1];
    perms[0].perm = DASICS_LIBCFG_R;
    perms[0].lo = (uint64_t)buffer;
    perms[0].hi = (uint64_t)buffer + 15;
    perms[0].handle = -1;

    int ret = (int)dasics_umaincall(Umaincall_PGRANT,
                                    (void *)func2, &perms[0],
                                    (size_t)1, (unsigned)1);
    if (ret != 0) {
        dasics_umaincall(Umaincall_PRINT, "[func1] PGRANT failed\n");
        dasics_umaincall(Umaincall_FREE, (void *)buffer);
        return;
    }
    dasics_umaincall(Umaincall_PRINT, "[func1] granted READ permission to func2\n");

    /* Domain switch into func2, passing buffer pointer */
    dasics_umaincall(Umaincall_TRANS, (void *)func2, buffer);
    dasics_umaincall(Umaincall_PRINT, "[func1] returned from func2\n");

    /* Domain switch into func2, passing buffer pointer, expecting func2 fails to access the buffer */
    dasics_umaincall(Umaincall_TRANS, (void *)func2, buffer);
    dasics_umaincall(Umaincall_PRINT, "[func1] returned from func2\n");

    /* Free the buffer via trusted maincall */
    dasics_umaincall(Umaincall_FREE, (void *)buffer);
    dasics_umaincall(Umaincall_PRINT, "[func1] freed buffer\n");

    dasics_umaincall(Umaincall_PRINT, "[func1] end\n");
}

/* ---- main (trusted) ---- */

int main(void)
{
    printf("%s", test_info);
    fit_print();

    fit_switchto(func1);

    printf("[MAIN] done\n");
    return 0;
}
