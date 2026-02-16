#include "sreg_guard_common.h"

#include <stdio.h>

int main(void) {
    uint64_t saved_on_entry[SREG_COUNT];
    uint64_t after[SREG_COUNT];
    int rc = 0;

    rc = sreg_guard_runtime_init("test_sreg_guard_eager");
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

    sreg_set_pattern(0x3000);
    sreg_take_snapshot(saved_on_entry);
    sreg_call_untrusted_overwrite();
    sreg_restore_snapshot(saved_on_entry);
    sreg_take_snapshot(after);

    rc = sreg_expect_match("eager_restore_after_boundary_return", saved_on_entry, after);
    sreg_guard_runtime_fini();

    if (rc == 0) {
        printf("[RESULT] PASS\n");
    } else {
        printf("[RESULT] FAIL\n");
    }
    printf("[Finish] test dasics finished\n");
    return rc;
}
