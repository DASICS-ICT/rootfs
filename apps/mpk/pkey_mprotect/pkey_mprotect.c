#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mman.h>
#include <assert.h>

int main(void)
{
    char __attribute__((aligned(4096))) array[4096] = {'b'};

    int i;
    for (i = 0; i < 4096; ++i)
    {
        array[i] = 'a';  // write ok
    }
    printf("array = 0x%p, array[256] = %c\n", array, array[256]);  // read ok

    // allocate pkey
    int pkey = pkey_alloc(0, PKEY_DISABLE_WRITE|PKEY_DISABLE_ACCESS);
    printf("pkey = %d\n", pkey);
    assert(pkey > 0);
    int ret = pkey_mprotect(array, sizeof(array), PROT_READ|PROT_WRITE, pkey);
    printf("pkey_mprotect ret = %d\n", ret);
    assert(ret >= 0);

    // trigger faulty access
    char __attribute__((unused)) volatile temp = array[256];  // faulty read
    array[256] = 'n';  // faulty write
    printf("ERROR: Cannot reach here!\n");

    return 0;
}