#include "sreg_guard_common.h"

#include <stdio.h>

int main(void) {
    uint64_t before[SREG_COUNT];
    uint64_t after[SREG_COUNT];
    int rc = 0;

    rc = sreg_guard_runtime_init("test_sreg_guard_no_protect");
    if (rc != 0) {
        return 1;
    }

    rc = sreg_run_legal_save_then_use_scene();
    if (rc != 0) {
        sreg_guard_runtime_fini();
        printf("[RESULT] FAIL\n");
        printf("[Finish] test dasics finished\n");
        return rc;
    }

    sreg_set_pattern(0x2000);
    sreg_take_snapshot(before);
    sreg_call_untrusted_overwrite();
    sreg_take_snapshot(after);

    rc = sreg_expect_mismatch("destructive_overwrite_without_guard", before, after);
    sreg_guard_runtime_fini();

    if (rc == 0) {
        printf("[RESULT] PASS\n");
    } else {
        printf("[RESULT] FAIL\n");
    }
    printf("[Finish] test dasics finished\n");
    return rc;
}
