#include "sreg_guard_suite_common.h"

#include <stdio.h>
#include <string.h>

static void print_usage(const char *prog) {
    printf("Usage: %s --reg <s0|s1|...|s11> --case <viol_write|viol_read|proto_mismatch|legal_save_restore|auth_tamper_bitflip|auth_tamper_overwrite>\n",
           prog);
}

int main(int argc, char **argv) {
    int rc = 1;
    int i;
    int reg_index = -1;
    const char *reg_name = NULL;
    const char *case_name = NULL;

    if (argc != 5) {
        print_usage(argv[0]);
        return 2;
    }

    for (i = 1; i + 1 < argc; i += 2) {
        if (strcmp(argv[i], "--reg") == 0) {
            reg_name = argv[i + 1];
        } else if (strcmp(argv[i], "--case") == 0) {
            case_name = argv[i + 1];
        } else {
            print_usage(argv[0]);
            return 2;
        }
    }

    reg_index = sreg_guard_parse_reg(reg_name);
    if (reg_index < 0 || case_name == NULL) {
        print_usage(argv[0]);
        return 2;
    }

    if (sreg_guard_runtime_init(reg_name, case_name) != 0) {
        return 1;
    }

    if (strcmp(case_name, "viol_write") == 0) {
        rc = sreg_case_viol_write(reg_index);
    } else if (strcmp(case_name, "viol_read") == 0) {
        rc = sreg_case_viol_read(reg_index);
    } else if (strcmp(case_name, "proto_mismatch") == 0) {
        rc = sreg_case_proto_mismatch(reg_index);
    } else if (strcmp(case_name, "legal_save_restore") == 0) {
        rc = sreg_case_legal_save_restore(reg_index);
    } else if (strcmp(case_name, "auth_tamper_bitflip") == 0) {
        rc = sreg_case_auth_tamper_bitflip(reg_index);
    } else if (strcmp(case_name, "auth_tamper_overwrite") == 0) {
        rc = sreg_case_auth_tamper_overwrite(reg_index);
    } else {
        print_usage(argv[0]);
        rc = 2;
    }

    sreg_guard_runtime_fini();
    if (rc == 0) {
        printf("[SREG-GUARD] RESULT: PASS (%s/%s)\n", reg_name, case_name);
    } else {
        printf("[SREG-GUARD] RESULT: FAIL (%s/%s, rc=%d)\n", reg_name, case_name, rc);
    }
    return rc;
}
