#include "m3_cprog_feas.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <udasics.h>

extern char __ULIBTEXT_BEGIN__, __ULIBTEXT_END__;

static int g_cfg_ids[3] = {-1, -1, -1};
static int g_jumpcfg_id = -1;

static int m3_case20_fail_closed_load(struct ucontext_trap *regs)
{
    printf("[M3-FEASIBILITY] SECONDARY LOAD FAULT: utval=0x%lx pc=0x%lx\n",
           regs->utval, regs->uepc);
    exit(1);
}

static int m3_case20_fail_closed_store(struct ucontext_trap *regs)
{
    printf("[M3-FEASIBILITY] SECONDARY STORE FAULT: utval=0x%lx pc=0x%lx\n",
           regs->utval, regs->uepc);
    exit(1);
}

static int m3_case20_fail_closed_fetch(struct ucontext_trap *regs)
{
    printf("[M3-FEASIBILITY] SECONDARY FETCH FAULT: utval=0x%lx pc=0x%lx\n",
           regs->utval, regs->uepc);
    exit(1);
}

static int m3_case_is_auth_negative(const char *case_name)
{
    return strcmp(case_name, M3_CASE_AUTH_TAMPER_BITFLIP_C) == 0 ||
           strcmp(case_name, M3_CASE_AUTH_TAMPER_OVERWRITE_C) == 0 ||
           strcmp(case_name, M3_CASE_AUTH_TAMPER_OVERFLOW_C) == 0;
}

static void m3_reset_runtime_handles(void)
{
    int i;

    for (i = 0; i < 3; i++) {
        g_cfg_ids[i] = -1;
    }
    g_jumpcfg_id = -1;
}

static int m3_runtime_init(const m3_request_t *req, m3_result_t *res)
{
    register uint64_t sp asm("sp");

    m3_reset_runtime_handles();
    register_udasics(0);

    g_jumpcfg_id = dasics_jumpcfg_alloc((uint64_t)&__ULIBTEXT_BEGIN__,
                                        (uint64_t)&__ULIBTEXT_END__);
    if (g_jumpcfg_id < 0) {
        printf("[M3-FEASIBILITY] INIT FAIL: jumpcfg alloc failed\n");
        unregister_udasics();
        return -1;
    }

    g_cfg_ids[0] = (int)LIBCFG_ALLOC(DASICS_LIBCFG_R | DASICS_LIBCFG_W,
                                     (void *)req, sizeof(*req));
    g_cfg_ids[1] = (int)LIBCFG_ALLOC(DASICS_LIBCFG_R | DASICS_LIBCFG_W,
                                     res, sizeof(*res));
    g_cfg_ids[2] = (int)LIBCFG_ALLOC(DASICS_LIBCFG_R | DASICS_LIBCFG_W,
                                     (void *)(uintptr_t)(sp - 0x2000), 0x2000);

    if (g_cfg_ids[0] < 0 || g_cfg_ids[1] < 0 || g_cfg_ids[2] < 0) {
        printf("[M3-FEASIBILITY] INIT FAIL: libcfg alloc failed\n");
        return -2;
    }

    return 0;
}

static void m3_runtime_fini(void)
{
    int i;

    for (i = 0; i < 3; i++) {
        if (g_cfg_ids[i] >= 0) {
            dasics_libcfg_free(g_cfg_ids[i]);
        }
    }
    if (g_jumpcfg_id >= 0) {
        dasics_jumpcfg_free(g_jumpcfg_id);
    }
    unregister_udasics();
    m3_reset_runtime_handles();
}

static void m3_fill_case01_request(m3_request_t *req)
{
    memset(req, 0, sizeof(*req));
    req->record_count = M3_MAX_RECORDS;

    strcpy(req->records[0].text, "alpha-7");
    req->records[0].weight = 3;

    strcpy(req->records[1].text, "Beta42");
    req->records[1].weight = 5;

    strcpy(req->records[2].text, "delta");
    req->records[2].weight = 2;

    strcpy(req->records[3].text, "kappa9");
    req->records[3].weight = 4;
}

static int m3_verify_case01_result(const m3_result_t *res)
{
    if (res->total_score != 175) {
        return 1;
    }
    if (res->weighted_score != 605) {
        return 2;
    }
    if (res->total_length != 23) {
        return 3;
    }
    if (res->unique_initials != 4) {
        return 4;
    }
    if (strcmp(res->trace, "ABDK") != 0) {
        return 5;
    }
    return 0;
}

static int m3_verify_case00_result(const m3_result_t *res)
{
    if (res->total_score != 18) {
        return 1;
    }
    if (res->weighted_score != 14) {
        return 2;
    }
    if (res->total_length != 5) {
        return 3;
    }
    if (res->unique_initials != 4) {
        return 4;
    }
    if (res->trace[0] != '\0') {
        return 5;
    }
    return 0;
}

static int m3_verify_case00_single_slot_result(const m3_result_t *res)
{
    if (res->total_score != 12) {
        return 1;
    }
    if (res->weighted_score != 14) {
        return 2;
    }
    if (res->total_length != 4) {
        return 3;
    }
    if (res->unique_initials != 1) {
        return 4;
    }
    if (res->trace[0] != '\0') {
        return 5;
    }
    return 0;
}

int main(int argc, char **argv)
{
    m3_request_t req;
    m3_result_t res;
    const char *case_name = M3_CASE_SINGLE_SLOT;
    int rc;
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--case") == 0) {
            if (i + 1 >= argc) {
                printf("Usage: %s [--case %s]\n",
                       argv[0], M3_CASE_SINGLE_SLOT);
                return 2;
            }
            case_name = argv[++i];
        } else if (strcmp(argv[i], "-dasics") == 0 ||
                   strcmp(argv[i], "-sreg-open") == 0) {
            /* Linux may consume these DASICS control suffixes before main(). */
        } else if (strcmp(argv[i], "-sreg-close") == 0) {
            printf("[M3-FEASIBILITY] UNSUPPORTED MODE: %s requires -sreg-open\n",
                   case_name);
            return 2;
        } else {
            printf("Usage: %s [--case %s]\n",
                   argv[0], M3_CASE_SINGLE_SLOT);
            return 2;
        }
    }

    if (strcmp(case_name, M3_CASE_SINGLE_SLOT) != 0 &&
        strcmp(case_name, M3_CASE_PROLOGUE_ONLY) != 0 &&
        strcmp(case_name, M3_CASE_SIMPLE_CHAIN) != 0 &&
        strcmp(case_name, M3_CASE_NATURAL_SINGLE_LAYER) != 0 &&
        strcmp(case_name, M3_CASE_NATURAL_SIMPLE_CHAIN) != 0 &&
        strcmp(case_name, M3_CASE_AUTH_TAMPER_BITFLIP_C) != 0 &&
        strcmp(case_name, M3_CASE_AUTH_TAMPER_OVERWRITE_C) != 0 &&
        strcmp(case_name, M3_CASE_AUTH_TAMPER_OVERFLOW_C) != 0) {
        printf("[M3-FEASIBILITY] UNSUPPORTED CASE: %s\n", case_name);
        return 2;
    }

    memset(&res, 0, sizeof(res));
    m3_fill_case01_request(&req);

    printf("[M3-FEASIBILITY] CASE START: %s\n", case_name);

    rc = m3_runtime_init(&req, &res);
    if (rc != 0) {
        m3_runtime_fini();
        return 1;
    }

    if (m3_case_is_auth_negative(case_name)) {
        register_uload_fault_handler(m3_case20_fail_closed_load);
        register_ustore_fault_handler(m3_case20_fail_closed_store);
        register_ufetch_fault_handler(m3_case20_fail_closed_fetch);
    }

    if (strcmp(case_name, M3_CASE_SINGLE_SLOT) == 0) {
        rc = (int)lib_call((void *)m3_untrusted_process_case00_single_slot, &req, &res);
    } else if (strcmp(case_name, M3_CASE_PROLOGUE_ONLY) == 0) {
        rc = (int)lib_call((void *)m3_untrusted_process_case00, &req, &res);
    } else if (strcmp(case_name, M3_CASE_NATURAL_SINGLE_LAYER) == 0) {
        rc = (int)lib_call((void *)m3_untrusted_process_case10_natural_single_layer, &req, &res);
    } else if (strcmp(case_name, M3_CASE_NATURAL_SIMPLE_CHAIN) == 0) {
        rc = (int)lib_call((void *)m3_untrusted_process_case11_natural_simple_chain, &req, &res);
    } else if (strcmp(case_name, M3_CASE_AUTH_TAMPER_BITFLIP_C) == 0) {
        rc = (int)lib_call((void *)m3_untrusted_process_case20_auth_tamper_bitflip_c, &req, &res);
        printf("[M3-FEASIBILITY] CASE FAIL: %s unexpected return rc=%d\n",
               case_name, rc);
        m3_runtime_fini();
        return 1;
    } else if (strcmp(case_name, M3_CASE_AUTH_TAMPER_OVERWRITE_C) == 0) {
        rc = (int)lib_call((void *)m3_untrusted_process_case21_auth_tamper_overwrite_c, &req, &res);
        printf("[M3-FEASIBILITY] CASE FAIL: %s unexpected return rc=%d\n",
               case_name, rc);
        m3_runtime_fini();
        return 1;
    } else if (strcmp(case_name, M3_CASE_AUTH_TAMPER_OVERFLOW_C) == 0) {
        rc = (int)lib_call((void *)m3_untrusted_process_case22_auth_tamper_overflow_c, &req, &res);
        printf("[M3-FEASIBILITY] CASE FAIL: %s unexpected return rc=%d\n",
               case_name, rc);
        m3_runtime_fini();
        return 1;
    } else {
        rc = (int)lib_call((void *)m3_untrusted_process_case01, &req, &res);
    }

    if (m3_case_is_auth_negative(case_name)) {
        register_uload_fault_handler(handle_DasicsULoadFault);
        register_ustore_fault_handler(handle_DasicsUStoreFault);
        register_ufetch_fault_handler(handle_DasicsUFetchFault);
    }

    if (rc != 0) {
        printf("[M3-FEASIBILITY] CASE FAIL: %s rc=%d\n", case_name, rc);
        m3_runtime_fini();
        return 1;
    }

    if (strcmp(case_name, M3_CASE_SINGLE_SLOT) == 0) {
        rc = m3_verify_case00_single_slot_result(&res);
    } else if (strcmp(case_name, M3_CASE_PROLOGUE_ONLY) == 0) {
        rc = m3_verify_case00_result(&res);
    } else if (strcmp(case_name, M3_CASE_NATURAL_SINGLE_LAYER) == 0) {
        rc = m3_verify_case01_result(&res);
    } else if (strcmp(case_name, M3_CASE_NATURAL_SIMPLE_CHAIN) == 0) {
        rc = m3_verify_case01_result(&res);
    } else {
        rc = m3_verify_case01_result(&res);
    }
    if (rc != 0) {
        printf("[M3-FEASIBILITY] CASE FAIL: %s verify=%d score=%u weighted=%u len=%u initials=%u trace=%s\n",
               case_name, rc, res.total_score, res.weighted_score,
               res.total_length, res.unique_initials, res.trace);
        m3_runtime_fini();
        return 1;
    }

    printf("[M3-FEASIBILITY] CASE PASS: %s\n", case_name);
    m3_runtime_fini();
    return 0;
}
