/*
 * vuldyn.c -- Comprehensive DASICS dynamic library test.
 *
 * This single source file is compiled into libvuldyn.so.  It contains
 * four logical "zones" -- entry, func1, func2, and share -- each placed
 * into dedicated ELF sections via __attribute__((section(...))).
 *
 * The FIT compartment mechanism in the main executable isolates these
 * zones at runtime: each compartment is given only the code and data
 * bounds it needs, so cross-zone accesses trigger DASICS hardware faults.
 *
 * Test coverage:
 *   - [rwx]      Read/write/execute permission enforcement
 *   - [share]    Shared function and shared data access
 *   - [heap]     Self-managed heap isolation via mimalloc (library_id/closure_id)
 *   - [pgtrans]  Permission grant and domain transition (Umaincall_PGRANT + TRANS)
 *   - [argbound] Argument-bound checking across domain switch
 *   - [syscall]  System call whitelist enforcement
 *   - [maincall] Main call whitelist enforcement
 *
 * Compilation: riscv64-unknown-linux-gnu-gcc -O0 -fPIC -shared
 *   (deliberately WITHOUT -fno-plt; external calls go through PLT stubs)
 */

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <asm/unistd.h>

#include "fit.h"
#include "udasics.h"
#include "usyscall.h"

/* ======================================================================
 * Forward declarations for wrapper functions used as domain-switch targets.
 * These are placed in their respective ulibtext sections below.
 * ====================================================================== */
int func1_wrapper(va_list args);
int func2_wrapper(va_list args);

/* ======================================================================
 * Secret data -- deliberately NOT placed in any ulib* section.
 *
 * Because no compartment's mem bounds cover this region, any attempt
 * to read or write `secret` from within a compartment will trigger a
 * DASICS fault (DasicsULoadAccessFault or DasicsUStoreAccessFault).
 * ====================================================================== */
static char secret[100] = "[SECRET]: You should not be able to read this!";

/* ======================================================================
 * func1 private data
 *
 * Only the func1 compartment (and entry with full-library bounds) may
 * access these.  func2 has no mem bounds covering these addresses.
 * ====================================================================== */
static const char __attribute__((section(".ulibrodata.func1")))
    func1_readonly[100] = "[FUNC1]: Read-only buffer for func1";

static char __attribute__((section(".ulibdata.func1")))
    func1_rwbuffer[100] = "[FUNC1]: Read-write buffer for func1";

static char __attribute__((section(".ulibbss.func1")))
    func1_rwbss[10];

/* A short message used for argument-bound testing when calling func2 */
static const char __attribute__((section(".ulibrodata.func1")))
    hello_msg[] = "Hello from func1!";

/* ======================================================================
 * func2 private data
 *
 * Only the func2 compartment (and entry with full-library bounds) may
 * access these.  func1 has no mem bounds covering these addresses.
 * ====================================================================== */
static const char __attribute__((section(".ulibrodata.func2")))
    func2_readonly[100] = "[FUNC2]: Read-only buffer for func2";

static char __attribute__((section(".ulibdata.func2")))
    func2_rwbuffer[100] = "[FUNC2]: Read-write buffer for func2";

static char __attribute__((section(".ulibbss.func2")))
    func2_rwbss[10];

/* ======================================================================
 * Shared data -- accessible by any compartment that includes the
 * "share" section bounds (func1 and func2 both include these).
 * ====================================================================== */
static const char __attribute__((section(".ulibrodata.share")))
    share_readonly[100] = "[SHARE]: Shared read-only buffer";

static char __attribute__((section(".ulibdata.share")))
    share_rwbuffer[100] = "[SHARE]: Shared read-write buffer";

static char __attribute__((section(".ulibbss.share")))
    share_rwbss[10];

/*
 * shared_ptr -- used for cross-compartment heap isolation testing.
 * func1 allocates heap memory and stores the pointer here; func2 then
 * attempts to dereference it.  Because the heap page belongs to func1's
 * closure (library_id=1, closure_id=1), func2 (closure_id=2) has no
 * mem bound covering that region and the access will fault.
 */
static void * __attribute__((section(".ulibdata.share")))
    shared_ptr = NULL;


/* ======================================================================
 *  share() -- Shared function callable from both func1 and func2.
 *
 *  Both compartments include the share text/rodata/data/bss bounds,
 *  so direct calls to share() succeed without a domain switch.
 *  The function exercises shared data access and then attempts to
 *  touch func1/func2 private data -- which may or may not fault
 *  depending on the caller's compartment bounds.
 * ====================================================================== */
void __attribute__((section(".ulibtext.share"))) share(void)
{
    dasics_umaincall(Umaincall_PRINT, "[share] start\n");

    /* --- Legal: read shared read-only data --- */
    volatile char __attribute__((unused)) c;
    c = share_readonly[0];
    dasics_umaincall(Umaincall_PRINT, "[share] read shared rodata: OK\n");

    /* --- Legal: read/write shared rw buffer --- */
    share_rwbuffer[0] = 'S';
    c = share_rwbuffer[0];
    dasics_umaincall(Umaincall_PRINT, "[share] read/write shared rwbuffer: OK\n");

    /* --- Legal: write shared bss --- */
    for (int i = 0; i < 9; i++)
        share_rwbss[i] = 'S';
    share_rwbss[9] = '\0';
    dasics_umaincall(Umaincall_PRINT, "[share] wrote shared bss: %s\n", share_rwbss);

    /*
     * Attempt to access func1 private data.
     * If the calling compartment is func1, the bounds already cover func1
     * data so this will succeed.  If the caller is func2, these accesses
     * are outside its bounds and will trigger DASICS faults.
     */
    dasics_umaincall(Umaincall_PRINT, "[share] attempting access to func1_rwbuffer...\n");
    c = func1_rwbuffer[0];
    dasics_umaincall(Umaincall_PRINT, "[share] read func1_rwbuffer[0]: OK\n");

    /*
     * Attempt to access func2 private data (symmetric to above).
     */
    dasics_umaincall(Umaincall_PRINT, "[share] attempting access to func2_rwbuffer...\n");
    c = func2_rwbuffer[0];
    dasics_umaincall(Umaincall_PRINT, "[share] read func2_rwbuffer[0]: OK\n");

    dasics_umaincall(Umaincall_PRINT, "[share] end\n");
}


/* ======================================================================
 *  func1() -- First untrusted function (compartment closure_id=1).
 *
 *  Test sequence:
 *    1. [rwx]     Legal reads/writes of func1 private data
 *    2. [rwx]     Illegal write to func1_readonly  -> DasicsUStoreAccessFault
 *    3. [rwx]     Illegal read of secret           -> DasicsULoadAccessFault
 *    4. [rwx]     Illegal read of func2 private    -> DasicsULoadAccessFault
 *    5. [share]   Call share() -- exercises shared bounds
 *    6. [heap]    Allocate heap via Umaincall_MALLOC, write data, store in shared_ptr
 *    7. [pgtrans] Grant READ permission on hello_msg to func2_wrapper
 *    8. [pgtrans] Domain-switch to func2_wrapper, passing hello_msg as argument
 *    9. [heap]    Free heap via Umaincall_FREE
 * ====================================================================== */
void __attribute__((section(".ulibtext.func1"))) func1(void)
{
    dasics_umaincall(Umaincall_PRINT, "[func1] start\n");

    /* ------------------------------------------------------------------
     * [rwx] Legal: read the read-only buffer (within func1 rodata bounds)
     * ------------------------------------------------------------------ */
    dasics_umaincall(Umaincall_PRINT, "[func1] reading func1_readonly: %s\n", func1_readonly);

    /* ------------------------------------------------------------------
     * [rwx] Legal: read and write the rw buffer (within func1 data bounds)
     * ------------------------------------------------------------------ */
    func1_rwbuffer[0] = 'F';
    dasics_umaincall(Umaincall_PRINT, "[func1] modified func1_rwbuffer: %s\n", func1_rwbuffer);

    /* ------------------------------------------------------------------
     * [rwx] Legal: write to bss (within func1 bss bounds)
     * ------------------------------------------------------------------ */
    for (int i = 0; i < 9; i++)
        func1_rwbss[i] = '1';
    func1_rwbss[9] = '\0';
    dasics_umaincall(Umaincall_PRINT, "[func1] wrote func1_rwbss: %s\n", func1_rwbss);

    /* ------------------------------------------------------------------
     * [rwx] Illegal: write to read-only buffer -> DasicsUStoreAccessFault
     *   The func1 compartment has R permission on .ulibrodata.func1,
     *   but NOT W permission, so this store instruction will be trapped.
     * ------------------------------------------------------------------ */
    dasics_umaincall(Umaincall_PRINT, "[func1] attempting illegal write to func1_readonly...\n");
    ((char *)func1_readonly)[0] = 'X';

    /* ------------------------------------------------------------------
     * [rwx] Illegal: read secret -> DasicsULoadAccessFault
     *   secret is outside all compartment bounds (no ulib* section attr).
     * ------------------------------------------------------------------ */
    dasics_umaincall(Umaincall_PRINT, "[func1] attempting illegal read of secret...\n");
    /*
     * Read deeper into secret[] to avoid any accidental boundary bleed
     * near adjacent sections caused by coarse bound alignment.
     */
    volatile char __attribute__((unused)) leaked = secret[64];

    /* ------------------------------------------------------------------
     * [rwx] Illegal: read func2 private data -> DasicsULoadAccessFault
     *   func1's mem bounds do not cover func2's rodata/data/bss.
     * ------------------------------------------------------------------ */
    dasics_umaincall(Umaincall_PRINT, "[func1] attempting illegal read of func2_rwbuffer...\n");
    /* Use a non-edge offset to avoid boundary-adjacent false negatives. */
    volatile char __attribute__((unused)) cross = func2_rwbuffer[64];

    /* ------------------------------------------------------------------
     * [share] Call the shared function.  Both func1's and share's text
     * are within the func1 compartment's code bounds, and both share
     * data sections are within its mem bounds, so this direct call works.
     * ------------------------------------------------------------------ */
    dasics_umaincall(Umaincall_PRINT, "[func1] calling share()...\n");
    share();
    dasics_umaincall(Umaincall_PRINT, "[func1] returned from share()\n");

    /* ------------------------------------------------------------------
     * [heap] Allocate a buffer on the self-managed heap via Umaincall_MALLOC.
     * The trusted maincall handler allocates from the mimalloc heap
     * associated with (library_id=1, closure_id=1) and automatically adds
     * a mem bound for the allocated region to this compartment.
     * ------------------------------------------------------------------ */
    size_t alloc_size = 64;
    char *heap_buf = (char *)(uintptr_t)dasics_umaincall(Umaincall_MALLOC, alloc_size);
    if (!heap_buf) {
        dasics_umaincall(Umaincall_PRINT, "[func1] MALLOC failed!\n");
        return;
    }
    dasics_umaincall(Umaincall_PRINT, "[func1] allocated %d bytes on heap\n", (int)alloc_size);

    /* Write a test pattern into the heap buffer */
    for (size_t i = 0; i < alloc_size; i++)
        heap_buf[i] = (char)(0xAB);
    dasics_umaincall(Umaincall_PRINT, "[func1] wrote pattern to heap buffer\n");

    /*
     * Store the heap pointer into shared_ptr so func2 can try to access it.
     * This is within the share data bounds, which both compartments can access.
     */
    shared_ptr = heap_buf;
    dasics_umaincall(Umaincall_PRINT, "[func1] stored heap pointer in shared_ptr\n");

    /* ------------------------------------------------------------------
     * [pgtrans] Grant READ permission on hello_msg to the func2_wrapper
     * compartment.  The permission covers sizeof(hello_msg) bytes.
     *
     * After the grant, func2_wrapper will have a temporary mem bound
     * allowing it to read hello_msg.  The grant is valid for 1 transition.
     * ------------------------------------------------------------------ */
    fit_bounds_t perms[1];
    perms[0].perm   = DASICS_LIBCFG_R;
    perms[0].lo     = (uint64_t)hello_msg;
    perms[0].hi     = (uint64_t)hello_msg + sizeof(hello_msg) - 1;
    perms[0].handle = -1;

    /*
     * valist_size: the total byte size of the va_list arguments that will
     * be passed in the subsequent Umaincall_TRANS call (one char* pointer).
     */
    size_t valist_size = sizeof(char *);
    int grant_ret = (int)dasics_umaincall(Umaincall_PGRANT,
                                          (void *)func2_wrapper, &perms[0],
                                          (size_t)1, valist_size, (unsigned)1);
    if (grant_ret != 0) {
        dasics_umaincall(Umaincall_PRINT, "[func1] PGRANT failed (ret=%d)\n", grant_ret);
    } else {
        dasics_umaincall(Umaincall_PRINT, "[func1] granted READ on hello_msg to func2\n");
    }

    /* ------------------------------------------------------------------
     * [pgtrans] Domain-switch into func2_wrapper, passing hello_msg.
     * The runtime (do_transition) will push func1's state, apply func2's
     * bounds (including the temporary argbound from PGRANT), and invoke
     * func2_wrapper(va_list) in the func2 compartment.
     * ------------------------------------------------------------------ */
    dasics_umaincall(Umaincall_PRINT, "[func1] switching to func2_wrapper...\n");
    dasics_umaincall(Umaincall_TRANS, (void *)func2_wrapper, (char *)hello_msg);
    dasics_umaincall(Umaincall_PRINT, "[func1] returned from func2\n");

    /* ------------------------------------------------------------------
     * [heap] Free the heap buffer via Umaincall_FREE.
     * The trusted maincall handler frees from the same mimalloc heap and
     * removes the corresponding mem bound from this compartment.
     * ------------------------------------------------------------------ */
    dasics_umaincall(Umaincall_FREE, (void *)heap_buf);
    shared_ptr = NULL;
    dasics_umaincall(Umaincall_PRINT, "[func1] freed heap buffer\n");

    dasics_umaincall(Umaincall_PRINT, "[func1] end\n");
}


/* ======================================================================
 *  func1_wrapper() -- va_list wrapper for func1.
 *
 *  Domain switches via Umaincall_TRANS require a function with the
 *  signature `int wrapper(va_list args)`.  This wrapper simply calls
 *  func1() with no arguments.
 * ====================================================================== */
int __attribute__((section(".ulibtext.func1"))) func1_wrapper(va_list args)
{
    (void)args;  /* func1 takes no arguments */
    func1();
    return 0;
}


/* ======================================================================
 *  func2() -- Second untrusted function (compartment closure_id=2).
 *
 *  Test sequence:
 *    1. [argbound] Legal read of str  (within the PGRANT-ed range)
 *    2. [argbound] Illegal write to str -> DasicsUStoreAccessFault (granted R only)
 *    3. [argbound] Illegal out-of-bounds read str[-4096] -> DasicsULoadAccessFault
 *    4. [syscall]  Legal syscall: getpid -> OK
 *    5. [syscall]  Illegal syscall: exit  -> DasicsUSyscallAccessFault
 *    6. [maincall] Illegal maincall: Umaincall_UNKNOWN -> rejected by fit_check_maincall
 *    7. [rwx]      Legal access to func2 private data
 *    8. [rwx]      Illegal access to func1 private data -> fault
 *    9. [heap]     Illegal read via shared_ptr (func1's heap) -> DasicsULoadAccessFault
 * ====================================================================== */
void __attribute__((section(".ulibtext.func2"))) func2(char *str)
{
    dasics_umaincall(Umaincall_PRINT, "[func2] start\n");

    /* ------------------------------------------------------------------
     * [argbound] Legal: read the granted string argument.
     * The PGRANT from func1 gave us READ permission on hello_msg (18 bytes).
     * ------------------------------------------------------------------ */
    dasics_umaincall(Umaincall_PRINT, "[func2] reading granted string: %s\n", str);

    /* ------------------------------------------------------------------
     * [argbound] Illegal: write to str -> DasicsUStoreAccessFault
     *   We only have READ permission on this range, not WRITE.
     * ------------------------------------------------------------------ */
    dasics_umaincall(Umaincall_PRINT, "[func2] attempting illegal write to str...\n");
    str[0] = 'X';

    /* ------------------------------------------------------------------
     * [argbound] Illegal: read before the granted range -> DasicsULoadAccessFault
     *   Use a larger negative offset to avoid boundary-adjacent effects.
     * ------------------------------------------------------------------ */
    dasics_umaincall(Umaincall_PRINT, "[func2] attempting illegal out-of-bounds read str[-4096]...\n");
    volatile char __attribute__((unused)) oob = str[-4096];

    /* ------------------------------------------------------------------
     * [syscall] Legal: getpid is in the syscall whitelist for func2.
     *   ULIB_SYSCALL0 is an inline asm ecall, no PLT/GOT involvement.
     * ------------------------------------------------------------------ */
    uint64_t pid = ULIB_SYSCALL0(__NR_getpid);
    dasics_umaincall(Umaincall_PRINT, "[func2] getpid = %ld (legal syscall OK)\n", pid);

    /* ------------------------------------------------------------------
     * [syscall] Illegal: exit is NOT in the whitelist -> DasicsUSyscallAccessFault
     *   The DASICS hardware intercepts the ecall and checks against the
     *   syscall bitmap; __NR_exit is not permitted for this compartment.
     * ------------------------------------------------------------------ */
    dasics_umaincall(Umaincall_PRINT, "[func2] attempting illegal syscall (exit)...\n");
    ULIB_SYSCALL0(__NR_exit);

    /* ------------------------------------------------------------------
     * [maincall] Illegal: Umaincall_UNKNOWN is not in the maincall whitelist.
     *   fit_check_maincall() in the trusted handler will reject this call,
     *   and the fault handler will skip the offending instruction.
     * ------------------------------------------------------------------ */
    dasics_umaincall(Umaincall_PRINT, "[func2] attempting illegal maincall (UNKNOWN)...\n");
    dasics_umaincall(Umaincall_UNKNOWN, 0xdeadbeef);

    /* ------------------------------------------------------------------
     * [rwx] Legal: access func2's own private data
     * ------------------------------------------------------------------ */
    dasics_umaincall(Umaincall_PRINT, "[func2] reading func2_readonly: %s\n", func2_readonly);
    func2_rwbuffer[0] = '2';
    dasics_umaincall(Umaincall_PRINT, "[func2] modified func2_rwbuffer: %s\n", func2_rwbuffer);
    for (int i = 0; i < 9; i++)
        func2_rwbss[i] = '2';
    func2_rwbss[9] = '\0';
    dasics_umaincall(Umaincall_PRINT, "[func2] wrote func2_rwbss: %s\n", func2_rwbss);

    /* ------------------------------------------------------------------
     * [rwx] Illegal: access func1 private data -> DasicsULoadAccessFault
     *   func2's mem bounds do not cover func1's data sections.
     * ------------------------------------------------------------------ */
    dasics_umaincall(Umaincall_PRINT, "[func2] attempting illegal read of func1_rwbuffer...\n");
    /* Use a non-edge offset to avoid boundary-adjacent false negatives. */
    volatile char __attribute__((unused)) cross = func1_rwbuffer[64];

    /* ------------------------------------------------------------------
     * [heap] Illegal: read func1's heap via shared_ptr -> DasicsULoadAccessFault
     *   shared_ptr itself is in .ulibdata.share (accessible), but the
     *   pointed-to memory belongs to func1's mimalloc heap (closure_id=1),
     *   and func2 (closure_id=2) has no mem bound for that heap page.
     * ------------------------------------------------------------------ */
    if (shared_ptr) {
        dasics_umaincall(Umaincall_PRINT, "[func2] attempting illegal read via shared_ptr (func1 heap)...\n");
        volatile char __attribute__((unused)) heap_leak = *(char *)shared_ptr;
    }

    dasics_umaincall(Umaincall_PRINT, "[func2] end\n");
}


/* ======================================================================
 *  func2_wrapper() -- va_list wrapper for func2.
 *
 *  Extracts the char* argument from the va_list passed by Umaincall_TRANS
 *  and forwards it to func2().
 * ====================================================================== */
int __attribute__((section(".ulibtext.func2"))) func2_wrapper(va_list args)
{
    char *str = va_arg(args, char *);
    func2(str);
    return 0;
}


/* ======================================================================
 *  entry() -- Library entry point (compartment closure_id=0).
 *
 *  This compartment has full-library bounds (all ulibtext, ulibrodata,
 *  ulibdata, ulibbss sections).  It acts as the orchestrator: prints a
 *  banner, domain-switches into func1_wrapper, then prints completion.
 *
 *  Allowed maincalls: PRINT, TRANS (just enough to print and switch).
 * ====================================================================== */
void __attribute__((section(".ulibtext.entry"))) entry(void)
{
    dasics_umaincall(Umaincall_PRINT, "========== VULDYN TEST START ==========\n");
    dasics_umaincall(Umaincall_PRINT, "[entry] switching to func1_wrapper...\n");

    /*
     * Domain-switch into func1.  The runtime will push entry's context,
     * apply func1's narrower bounds, and call func1_wrapper(va_list).
     */
    dasics_umaincall(Umaincall_TRANS, (void *)func1_wrapper);

    dasics_umaincall(Umaincall_PRINT, "[entry] returned from func1\n");
    dasics_umaincall(Umaincall_PRINT, "========== VULDYN TEST END   ==========\n");
}
