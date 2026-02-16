#include "s0_guard_common.h"

#include <stdio.h>
#include <string.h>

static void print_usage(const char *prog) {
    printf("Usage: %s --case <viol_write|viol_read|proto_mismatch|legal_save_restore>\n", prog);
}

int main(int argc, char **argv) {
    int rc = 1;
    const char *case_name = NULL;

    if (argc == 3 && strcmp(argv[1], "--case") == 0) {
        case_name = argv[2];
    } else {
        print_usage(argv[0]);
        return 2;
    }

    if (s0_guard_runtime_init(case_name) != 0) {
        return 1;
    }

    if (strcmp(case_name, "viol_write") == 0) {
        rc = s0_case_viol_write();
    } else if (strcmp(case_name, "viol_read") == 0) {
        rc = s0_case_viol_read();
    } else if (strcmp(case_name, "proto_mismatch") == 0) {
        rc = s0_case_proto_mismatch();
    } else if (strcmp(case_name, "legal_save_restore") == 0) {
        rc = s0_case_legal_save_restore();
    } else {
        print_usage(argv[0]);
        rc = 2;
    }

    s0_guard_runtime_fini();
    if (rc == 0) {
        printf("[S0-GUARD] RESULT: PASS (%s)\n", case_name);
    } else {
        printf("[S0-GUARD] RESULT: FAIL (%s, rc=%d)\n", case_name, rc);
    }
    return rc;
}
