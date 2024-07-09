#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mman.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

#define PAGE_SIZE 4096
#define ATTR_PAGE_ALIGNED __attribute__((aligned(PAGE_SIZE)))

#define read_csr(reg) ({ unsigned long __tmp; \
  asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })

#define write_csr(reg, val) ({ \
  asm volatile ("csrw " #reg ", %0" :: "rK"(val)); })

#define rdcycle() read_csr(cycle)
#define rdtime() read_csr(time)

#define BUF_SIZE (PAGE_SIZE * 4)

static char ATTR_PAGE_ALIGNED ulib1_readonly[BUF_SIZE] = "This is ro to ULIB1 and invisible to ULIB2!\n";
static char ATTR_PAGE_ALIGNED ulib1_rwbuffer[BUF_SIZE] = "This is rw to ULIB1 and ro to ULIB2!\n";
static char ATTR_PAGE_ALIGNED ulib2_rwbuffer[BUF_SIZE] = "This is rw to ULIB1 and ULIB2!\n";
static char ATTR_PAGE_ALIGNED ulib2_sharebuf[BUF_SIZE];

void test_ulib2(void)
{
#ifdef PRINT_DEBUG
    printf("[ULIB2] ulib1_rwbuffer: %s", ulib1_rwbuffer);  // ok
    printf("[ULIB2] ulib2_rwbuffer: %s", ulib2_rwbuffer);  // ok
    printf("[ULIB2] ulib1_robuffer: %s", ulib1_readonly);  // raise exception
#endif  // PRINT_DEBUG
}

void test_ulib1(int pkey1_ulib1_ro, int pkey1_ulib1_rw, int pkey1_ulib2_rw)
{
    // Start timing
    uint64_t start, end;
    start = rdcycle();

    // Start iteration
    int i = 0;
    const int iterations = 1;
    while (i < iterations) {
        // Set privilege of ulib2 before calling it
        uint64_t pkru_old = read_csr(0x800);  // CSR_UPKRU
        uint64_t pkru_new = pkru_old | (PKEY_DISABLE_ACCESS << (pkey1_ulib1_ro * 2)) |
                                       (PKEY_DISABLE_WRITE  << (pkey1_ulib1_rw * 2));
        write_csr(0x800, pkru_new);  // CSR_UPKRU

        // Call ulib2
        test_ulib2();

        // Restore ulib1's privilege
        write_csr(0x800, pkru_old);  // CSR_UPKRU

        i++;
    }

    // End timing
    end = rdcycle();
    printf("[ULIB1] INFO: total time_elapsed: %lu cycles\n", end - start);
}

// void test_ulib1(int pkey1_ulib1_ro, int pkey1_ulib1_rw, int pkey1_ulib2_rw)
// {
//     // Start timing
//     uint64_t start, end;
//     start = rdcycle();

//     // Allocate pkeys for ulib2
//     int pkey2_ulib1_ro = pkey_alloc(0, PKEY_DISABLE_ACCESS);
//     int pkey2_ulib1_rw = pkey_alloc(0, PKEY_DISABLE_WRITE);
//     int pkey2_ulib2_rw = pkey_alloc(0, 0);

//     // Start iteration
//     // int i = 0;
//     // const int iteration = 100;
//     // while (i < iteration)
//     // {
//         // Bind allocated pkeys with arrays
//         pkey_mprotect(ulib1_readonly, BUF_SIZE, PROT_READ|PROT_WRITE, pkey2_ulib1_ro);
//         pkey_mprotect(ulib1_rwbuffer, BUF_SIZE, PROT_READ|PROT_WRITE, pkey2_ulib1_rw);
//         pkey_mprotect(ulib2_rwbuffer, BUF_SIZE, PROT_READ|PROT_WRITE, pkey2_ulib2_rw);

//         // Call ulib2
//         test_ulib2();

//         // Restore ulib1's privilege
//         pkey_mprotect(ulib1_readonly, BUF_SIZE, PROT_READ|PROT_WRITE, pkey1_ulib1_ro);
//         pkey_mprotect(ulib1_rwbuffer, BUF_SIZE, PROT_READ|PROT_WRITE, pkey1_ulib1_rw);
//         pkey_mprotect(ulib2_rwbuffer, BUF_SIZE, PROT_READ|PROT_WRITE, pkey1_ulib2_rw);

//     //     i++;
//     // }

//     // Release allocated pkeys
//     pkey_free(pkey2_ulib1_ro);
//     pkey_free(pkey2_ulib1_rw);
//     pkey_free(pkey2_ulib2_rw);

//     // End timing
//     end = rdcycle();
//     printf("[ULIB1] INFO: total time_elapsed: %lu cycles\n", end - start);
// }

// void test_ulib1(int pkey1_ulib1_ro, int pkey1_ulib1_rw, int pkey1_ulib2_rw)
// {
//     // Start timing
//     uint64_t start, end;
//     start = rdcycle();

//     // Allocate and bind one pkey for ulib2 shared buffer
//     int pkey2_shrbuf_rw = pkey_alloc(0, 0);
//     pkey_mprotect(ulib2_sharebuf, BUF_SIZE, PROT_READ|PROT_WRITE, pkey2_shrbuf_rw);

//     // Start iteration
//     // int i = 0;
//     // const int iteration = 10000;
//     // while (i < iteration)
//     // {
//         // Copy shared data to ulib2 shared buffer
//         memcpy(ulib2_sharebuf, ulib1_readonly, BUF_SIZE);

//         // Set privilege of ulib2 before calling it
//         uint64_t pkru_old = read_csr(0x800);  // CSR_UPKRU
//         uint64_t pkru_new = pkru_old | (PKEY_DISABLE_ACCESS << (pkey1_ulib1_ro * 2)) |
//                                        (PKEY_DISABLE_WRITE  << (pkey1_ulib1_rw * 2));
//         write_csr(0x800, pkru_new);  // CSR_UPKRU

//         // Call ulib2
//         test_ulib2();

//         // Restore ulib1's privilege
//         write_csr(0x800, pkru_old);  // CSR_UPKRU

//         // Copy shared data to ulib1 rw buffer
//         memcpy(ulib1_rwbuffer, ulib2_sharebuf, BUF_SIZE);

//     //     i++;
//     // }

//     // Release allocated pkeys
//     pkey_free(pkey2_shrbuf_rw);

//     // End timing
//     end = rdcycle();
//     printf("[ULIB1] INFO: total time_elapsed: %lu cycles\n", end - start);
// }

int main(void)
{
    printf("[MAIN] INFO: Enter main ... CLOCKS_PER_SEC = %ld\n", CLOCKS_PER_SEC);

    uint64_t start = rdcycle();

    for (int i = 0; i < 50; ++i) {
        rdcycle();
    }

    uint64_t end = rdcycle();

    printf("[ULIB1] INFO: total time_elapsed: %lu cycles\n", end - start);

    // Allocate pkeys for ulib1
    int pkey1_ulib1_ro = pkey_alloc(0, PKEY_DISABLE_WRITE);
    int pkey1_ulib1_rw = pkey_alloc(0, 0);
    int pkey1_ulib2_rw = pkey_alloc(0, 0);

    // Bind allocated pkeys with arrays
    pkey_mprotect(ulib1_readonly, BUF_SIZE, PROT_READ|PROT_WRITE, pkey1_ulib1_ro);
    pkey_mprotect(ulib1_rwbuffer, BUF_SIZE, PROT_READ|PROT_WRITE, pkey1_ulib1_rw);
    pkey_mprotect(ulib2_rwbuffer, BUF_SIZE, PROT_READ|PROT_WRITE, pkey1_ulib2_rw);

    // Call ulib1
    for (int i = 0; i < 100; ++i) {
        test_ulib1(pkey1_ulib1_ro, pkey1_ulib1_rw, pkey1_ulib2_rw);
    }

    // Release allocated pkeys
    pkey_free(pkey1_ulib1_ro);
    pkey_free(pkey1_ulib1_rw);
    pkey_free(pkey1_ulib2_rw);

    return 0;
}