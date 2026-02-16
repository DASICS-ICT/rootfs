#include "sreg_guard_common.h"

#include <stdio.h>

int main(void) {
    uint64_t saved_on_first_touch[SREG_COUNT];
    uint64_t after[SREG_COUNT];
    int rc = 0;

    rc = sreg_guard_runtime_init("test_sreg_guard_lazy");
    if (rc != 0) {
        return 1;
    }

    sreg_set_pattern(0x4000);
    sreg_call_untrusted_save_then_use();

    /* Software proxy for first-touch trigger: snapshot right before first destructive overwrite. */
    sreg_take_snapshot(saved_on_first_touch);
    sreg_call_untrusted_overwrite();
    sreg_restore_snapshot(saved_on_first_touch);
    sreg_take_snapshot(after);

    rc = sreg_expect_match("lazy_restore_after_first_touch", saved_on_first_touch, after);
    sreg_guard_runtime_fini();

    if (rc == 0) {
        printf("[RESULT] PASS\n");
    } else {
        printf("[RESULT] FAIL\n");
    }
    printf("[Finish] test dasics finished\n");
    return rc;
}
