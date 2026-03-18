#include "m4_sreg_alpha_suite.h"

#include <stdio.h>
#include <string.h>

static void print_usage(const char *prog)
{
    printf("Usage: %s --case <%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s>\n",
           prog,
           M4_CASE00_NO_TOUCH_FAST_RETURN,
           M4_CASE01_FIRST_READ_TOKENIZE_THEN_OPEN,
           M4_CASE02_TOKEN_STACK_ROUNDTRIP,
           M4_CASE03_TOKEN_ARITH_CORRUPT_FAULT,
           M4_CASE04_FIRST_DEST_WRITE_UNUSED,
           M4_CASE05_FIRST_DEST_WRITE_TRUST_OVERWRITE,
           M4_CASE06_FIRST_DEST_WRITE_TRUST_USE,
           M4_CASE07_SWAP_S1_TO_S2_FAULT,
           M4_CASE07_SWAP_S1_TO_S11_FAULT,
           M4_CASE08_MULTI_SLOT_INDEPENDENCE,
           M4_CASE09_NATURAL_PROLOGUE_ONLY,
           M4_CASE10_NATURAL_SINGLE_LAYER,
           M4_CASE11_NATURAL_SIMPLE_CHAIN,
           M4_CASE12_STACK_TOKEN_BITFLIP_FAULT,
           M4_CASE13_STACK_TOKEN_OVERFLOW_FAULT);
}

int main(int argc, char **argv)
{
    const char *case_name = NULL;
    int case_id;
    int rc = 1;
    int i;

    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--case") == 0) {
            if (i + 1 >= argc) {
                print_usage(argv[0]);
                return 2;
            }
            case_name = argv[++i];
        } else if (strcmp(argv[i], "-dasics") == 0 ||
                   strcmp(argv[i], "-sreg-open") == 0) {
            continue;
        } else {
            print_usage(argv[0]);
            return 2;
        }
    }

    case_id = m4_sreg_alpha_parse_case(case_name);
    if (case_id < 0) {
        print_usage(argv[0]);
        return 2;
    }

    if (m4_sreg_alpha_runtime_init(case_name) != 0) {
        return 1;
    }

    switch (case_id) {
    case 0:
        rc = m4_case00_no_touch_fast_return();
        break;
    case 1:
        rc = m4_case01_first_read_tokenize_then_open();
        break;
    case 2:
        rc = m4_case02_token_passthrough_stack_roundtrip();
        break;
    case 3:
        rc = m4_case03_token_arith_corrupt_fault();
        break;
    case 4:
        rc = m4_case04_first_dest_write_then_unused();
        break;
    case 5:
        rc = m4_case05_first_dest_write_then_trusted_overwrite();
        break;
    case 6:
        rc = m4_case06_first_dest_write_then_trusted_use_fault();
        break;
    case 7:
        rc = m4_case07_swap_s1_to_s2_fault();
        break;
    case 8:
        rc = m4_case07_swap_s1_to_s11_fault();
        break;
    case 9:
        rc = m4_case08_multi_slot_independence();
        break;
    case 10:
        rc = m4_case09_natural_prologue_only();
        break;
    case 11:
        rc = m4_case10_natural_single_layer();
        break;
    case 12:
        rc = m4_case11_natural_simple_chain();
        break;
    case 13:
        rc = m4_case12_stack_token_bitflip_fault();
        break;
    case 14:
        rc = m4_case13_stack_token_overflow_fault();
        break;
    default:
        rc = 2;
        break;
    }

    m4_sreg_alpha_runtime_fini();
    if (rc == 0) {
        printf("[M4-ALPHA] RESULT: PASS (%s)\n", case_name);
    } else {
        printf("[M4-ALPHA] RESULT: FAIL (%s, rc=%d)\n", case_name, rc);
    }
    printf("[Finish] test dasics finished\n");
    return rc;
}
