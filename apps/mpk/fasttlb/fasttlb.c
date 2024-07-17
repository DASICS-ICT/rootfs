#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mman.h>
#include <assert.h>

int main(void)
{
    char __attribute__((aligned(4096))) array_ro[4096] = {'b'};
    char __attribute__((aligned(4096))) array_na[4096] = {'c'};

    int i;
    for (i = 0; i < 4096; ++i)
    {
        array_ro[i] = 'a';  // write ok
    }
    printf("array_ro = 0x%p, array_ro[256] = %c\n", array_ro, array_ro[256]);  // read ok

    // allocate pkeys
    int pkey_ro = pkey_alloc(0, PKEY_DISABLE_WRITE);
    printf("pkey_ro = %d\n", pkey_ro);
    assert(pkey_ro > 0);
    int ret = pkey_mprotect(array_ro, sizeof(array_ro), PROT_READ|PROT_WRITE, pkey_ro);
    printf("pkey_mprotect ret = %d\n", ret);
    assert(ret >= 0);

    int pkey_na = pkey_alloc(0, PKEY_DISABLE_ACCESS);
    printf("pkey_na = %d\n", pkey_na);
    assert(pkey_na > 0);
    ret = pkey_mprotect(array_na, sizeof(array_na), PROT_READ|PROT_WRITE, pkey_na);
    printf("pkey_mprotect ret = %d\n", ret);
    assert(ret >= 0);

    // trigger faulty access
    char __attribute__((unused)) volatile temp1 = array_ro[256];  // trigger tlb entry load
    char __attribute__((unused)) volatile temp2 = array_na[256];  // faulty read
    printf("ERROR: Cannot reach here!\n");

    pkey_free(pkey_na);
    pkey_free(pkey_ro);

    return 0;
}