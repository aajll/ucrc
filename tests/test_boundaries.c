/**
 * SPDX-License-Identifier: MIT
 *
 * @file test_boundaries.c
 * @brief Empty-input and defensive (bad-argument) boundary behaviour.
 */

#include "test_harness.h"
#include "test_support.h"
#include "ucrc.h"

/* Empty input yields the model's empty-message CRC. */
TEST_CASE(test_empty_input)
{
        /* CRC-32/ISO-HDLC of the empty message is 0x00000000. */
        TEST_ASSERT(ucrc_compute(&ucrc_crc32_iso_hdlc, NULL, 0u) == 0u);
        /* CRC-16/CCITT-FALSE of the empty message is its init, 0xFFFF. */
        TEST_ASSERT(ucrc_compute(&ucrc_crc16_ccitt_false, NULL, 0u) == 0xFFFFu);
}

#ifdef UCRC_TEST_ASSERT_HOOK
unsigned int ucrc_test_assert_failures = 0u;
#endif

/* Defensive behaviour. The NULL-argument cases are contract violations that
 * abort through UCRC_ASSERT in a normal build, so they run only in the
 * variant whose UCRC_ASSERT records the failure and continues
 * (UCRC_TEST_ASSERT_HOOK). Each case checks that the trap fired once per
 * violated precondition and that the defensive return follows. The valid
 * len-0 case is always safe and checked. */
TEST_CASE(test_defensive_boundaries)
{
#ifdef UCRC_TEST_ASSERT_HOOK
        ucrc_octet_t buf[1] = {0u};
        uint32_t crc = ucrc_begin(&ucrc_crc16_ccitt_false);

        ucrc_test_assert_failures = 0u;
        /* begin, update and finish each trap the NULL model. */
        TEST_ASSERT(ucrc_compute(NULL, buf, 1u) == 0u);
        TEST_ASSERT(ucrc_test_assert_failures == 3u);
        TEST_ASSERT(ucrc_begin(NULL) == 0u);
        TEST_ASSERT(ucrc_test_assert_failures == 4u);
        TEST_ASSERT(ucrc_update(NULL, crc, buf, 1u) == crc);
        TEST_ASSERT(ucrc_test_assert_failures == 5u);
        TEST_ASSERT(ucrc_finish(NULL, 0u) == 0u);
        TEST_ASSERT(ucrc_test_assert_failures == 6u);
        /* NULL data with a non-zero length must not dereference. */
        TEST_ASSERT(ucrc_update(&ucrc_crc16_ccitt_false, crc, NULL, 4u) == crc);
        TEST_ASSERT(ucrc_test_assert_failures == 7u);
        /* Valid calls do not trap. */
        (void)ucrc_compute(&ucrc_crc16_ccitt_false, buf, 1u);
        TEST_ASSERT(ucrc_test_assert_failures == 7u);
#endif
        /* NULL data with len 0 is a valid no-op (safe with assertions on). */
        TEST_ASSERT(ucrc_compute(&ucrc_crc16_ccitt_false, NULL, 0u) == 0xFFFFu);
}

/* The version number agrees with its parts. */
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
