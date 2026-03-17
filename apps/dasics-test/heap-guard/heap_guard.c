/*
 * Heap protection test: func1 allocates on its self-managed heap and stores
 * the pointer in shared_ptr (in .ulibdata.share). func2 tries to access
 * that heap via shared_ptr and triggers DASICS fault. main frees shared_ptr.
 */
#include <stdio.h>
#include <stdint.h>

#include "udasics.h"
#include "fit.h"

#include <mimalloc.h>

const char *test_info = "[MAIN] Heap guard test: self-managed heap isolation\n";

/* Shared pointer: in .ulibdata.share so main/func1/func2 can all access the variable */
static void *shared_ptr __attribute__((section(".ulibdata.share")));

void __attribute__((section(".ulibtext.func1"))) func1(void)
{
    dasics_umaincall(Umaincall_PRINT, "[func1] start\n");
    /* Allocate on this closure's self-managed heap via trusted maincall */
    size_t size = 64;
    void *p = (void *)(uintptr_t)dasics_umaincall(Umaincall_MALLOC, size);
    if (!p) {
        dasics_umaincall(Umaincall_PRINT, "[func1] MALLOC failed\n");
        return;
    }
    shared_ptr = p;
    /* Normal access to own heap */
    for (size_t i = 0; i < size; i++) {
        ((char *)p)[i] = 0xab;
    }
    dasics_umaincall(Umaincall_PRINT, "[func1] allocated and wrote to own heap\n");
    /* Intentionally do not free; main will free later */
    dasics_umaincall(Umaincall_PRINT, "[func1] end\n");
}

void __attribute__((section(".ulibtext.func2"))) func2(void)
{
    dasics_umaincall(Umaincall_PRINT, "[func2] start\n");
    if (!shared_ptr) {
        dasics_umaincall(Umaincall_PRINT, "[func2] shared_ptr is NULL\n");
        return;
    }
    dasics_umaincall(Umaincall_PRINT, "[func2] will access func1's heap via shared_ptr (expect DASICS fault)\n");
    /* This access is to func1's self-managed heap; func2 has no permission -> DASICS fault */
    volatile char __attribute__((unused)) c = *(char *)shared_ptr;  // raise DasicsULoadAccessFault
    *(char *)shared_ptr = 'B';  // raise DasicsUStoreAccessFault
    dasics_umaincall(Umaincall_PRINT, "[func2] end\n");
}

int main(void)
{
    printf("%s", test_info);
    fit_init(0);
    fit_print();

    fit_switchto(func1);
    fit_switchto(func2);

    if (shared_ptr) {
        free(shared_ptr);
        printf("[MAIN] freed shared_ptr\n");
    }

    fit_destroy();
    printf("[MAIN] done\n");
    return 0;
}
