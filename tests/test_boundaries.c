/**
 * SPDX-License-Identifier: MIT
 *
 * @file test_boundaries.c
 * @brief Empty-input and defensive (bad-argument) boundary behaviour.
 */

#include "test_harness.h"
#include "test_support.h"
#include "ucrc.h"
#include "ucrc_version.h"

/* Empty input yields the model's empty-message CRC. */
TEST_CASE(test_empty_input)
{
        /* CRC-32/ISO-HDLC of the empty message is 0x00000000. */
        TEST_ASSERT(ucrc_compute(&ucrc_crc32_iso_hdlc, NULL, 0u) == 0u);
        /* CRC-16/CCITT-FALSE of the empty message is its init, 0xFFFF. */
        TEST_ASSERT(ucrc_compute(&ucrc_crc16_ccitt_false, NULL, 0u) == 0xFFFFu);
}

/* Defensive behaviour. The NULL-argument cases are contract violations that
 * trip UCRC_ASSERT in a normal build, so the production no-UB return path is
 * only exercised in the variant compiled with assertions disabled
 * (UCRC_TEST_NO_ASSERT). The valid len-0 case is always safe and checked. */
TEST_CASE(test_defensive_boundaries)
{
#ifdef UCRC_TEST_NO_ASSERT
        ucrc_octet_t buf[1] = {0u};

        TEST_ASSERT(ucrc_compute(NULL, buf, 1u) == 0u);
        TEST_ASSERT(ucrc_begin(NULL) == 0u);
        TEST_ASSERT(ucrc_finish(NULL, 0u) == 0u);
        /* NULL data with a non-zero length must not dereference. */
        TEST_ASSERT(ucrc_update(&ucrc_crc16_ccitt_false,
                                ucrc_begin(&ucrc_crc16_ccitt_false), NULL, 4u)
                    == ucrc_begin(&ucrc_crc16_ccitt_false));
#endif
        /* NULL data with len 0 is a valid no-op (safe with assertions on). */
        TEST_ASSERT(ucrc_compute(&ucrc_crc16_ccitt_false, NULL, 0u) == 0xFFFFu);
}

/* The deprecated version header still exposes the macros from ucrc.h. */
TEST_CASE(test_version_macros)
{
        TEST_ASSERT(UCRC_VERSION_NUMBER
                    == ((UCRC_VERSION_MAJOR * 10000L)
                        + (UCRC_VERSION_MINOR * 100L) + UCRC_VERSION_PATCH));
        TEST_ASSERT(UCRC_VERSION_STRING[0] != '\0');
}

void
run_boundary_tests(void)
{
        run_test(test_empty_input, "empty_input");
        run_test(test_defensive_boundaries, "defensive_boundaries");
        run_test(test_version_macros, "version_macros");
}
