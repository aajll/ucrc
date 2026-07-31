/**
 * SPDX-License-Identifier: MIT
 *
 * @file test_boundaries.c
 * @brief Test empty input and invalid-argument boundaries.
 */

#include "test_harness.h"
#include "test_support.h"
#include "ucrc.h"

/* Test the CRC for an empty message. */
TEST_CASE(test_empty_input)
{
        /* CRC-32/ISO-HDLC of an empty message is 0x00000000. */
        TEST_ASSERT(ucrc_compute(&ucrc_crc32_iso_hdlc, NULL, 0u) == 0u);
        /* CRC-16/CCITT-FALSE of an empty message is 0xFFFF. */
        TEST_ASSERT(ucrc_compute(&ucrc_crc16_ccitt_false, NULL, 0u) == 0xFFFFu);
}

/* Test defensive behavior. NULL arguments violate the contract and invoke
 * UCRC_ASSERT. UCRC_TEST_NO_ASSERT tests the safe return path. A zero length
 * with NULL data is always valid. */
TEST_CASE(test_defensive_boundaries)
{
#ifdef UCRC_TEST_NO_ASSERT
        ucrc_octet_t buf[1] = {0u};

        TEST_ASSERT(ucrc_compute(NULL, buf, 1u) == 0u);
        TEST_ASSERT(ucrc_begin(NULL) == 0u);
        TEST_ASSERT(ucrc_finish(NULL, 0u) == 0u);
        /* NULL data with a non-zero length must not cause a dereference. */
        TEST_ASSERT(ucrc_update(&ucrc_crc16_ccitt_false,
                                ucrc_begin(&ucrc_crc16_ccitt_false), NULL, 4u)
                    == ucrc_begin(&ucrc_crc16_ccitt_false));
#endif
        /* NULL data with zero length is a valid no-op. */
        TEST_ASSERT(ucrc_compute(&ucrc_crc16_ccitt_false, NULL, 0u) == 0xFFFFu);
}

void
run_boundary_tests(void)
{
        run_test(test_empty_input, "empty_input");
        run_test(test_defensive_boundaries, "defensive_boundaries");
}
