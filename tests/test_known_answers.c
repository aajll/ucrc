/**
 * SPDX-License-Identifier: MIT
 *
 * @file test_known_answers.c
 * @brief Known-answer, exhaustive single-octet, and error-sensitivity tests.
 */

#include "test_harness.h"
#include "test_support.h"
#include "ucrc.h"

/* Published Rocksoft `check` constants for the predefined models. */
TEST_CASE(test_check_constants)
{
        TEST_ASSERT(
            ucrc_compute(&ucrc_crc8_smbus, UCRC_CHECK_INPUT, UCRC_CHECK_LEN)
            == 0xF4u);
        TEST_ASSERT(ucrc_compute(&ucrc_crc16_ccitt_false, UCRC_CHECK_INPUT,
                                 UCRC_CHECK_LEN)
                    == 0x29B1u);
        TEST_ASSERT(
            ucrc_compute(&ucrc_crc32_iso_hdlc, UCRC_CHECK_INPUT, UCRC_CHECK_LEN)
            == 0xCBF43926u);
}

/* A custom (untabulated) model must work via the bitwise fallback path. */
TEST_CASE(test_custom_model_bitwise_fallback)
{
        /* CRC-16/MODBUS: 0x8005 reflected, init 0xFFFF; check = 0x4B37. */
        const ucrc_model_t modbus = {
            .poly = 0x8005u,
            .init = 0xFFFFu,
            .xorout = 0x0000u,
            .table = NULL,
            .width = 16u,
            .refin = true,
            .refout = true,
        };

        TEST_ASSERT(ucrc_compute(&modbus, UCRC_CHECK_INPUT, UCRC_CHECK_LEN)
                    == 0x4B37u);
}

/* Results must be masked to the model width (no stray high bits). */
TEST_CASE(test_width_masking)
{
        uint32_t c8 =
            ucrc_compute(&ucrc_crc8_smbus, UCRC_CHECK_INPUT, UCRC_CHECK_LEN);
        uint32_t c16 = ucrc_compute(&ucrc_crc16_ccitt_false, UCRC_CHECK_INPUT,
                                    UCRC_CHECK_LEN);

        TEST_ASSERT((c8 & ~0xFFu) == 0u);
        TEST_ASSERT((c16 & ~0xFFFFu) == 0u);
}

/* Every single-octet value matches the reference, exercising every table
 * entry of the active strategy for all enabled widths. */
TEST_CASE(test_exhaustive_single_octet)
{
        size_t m;

        for (m = 0u; m < UCRC_TEST_NMODELS; m++) {
                const ucrc_model_t *model = UCRC_TEST_MODELS[m];
                unsigned v;

                for (v = 0u; v < 256u; v++) {
                        ucrc_octet_t b = (ucrc_octet_t)v;

                        TEST_ASSERT(ucrc_compute(model, &b, 1u)
                                    == ucrc_test_ref(model, &b, 1u));
                }
        }
}

/* A CRC must detect every single-bit error: flipping any one bit of the
 * message must change the CRC. */
TEST_CASE(test_single_bit_sensitivity)
{
#define SENS_LEN 32u
        ucrc_octet_t buf[SENS_LEN];
        size_t i;
        size_t m;

        for (i = 0u; i < SENS_LEN; i++) {
                buf[i] = (ucrc_octet_t)(0xA5u ^ (i * 7u));
        }

        for (m = 0u; m < UCRC_TEST_NMODELS; m++) {
                const ucrc_model_t *model = UCRC_TEST_MODELS[m];
                uint32_t base = ucrc_compute(model, buf, SENS_LEN);
                size_t bit;

                for (bit = 0u; bit < SENS_LEN * 8u; bit++) {
                        size_t byte = bit / 8u;
                        ucrc_octet_t mask = (ucrc_octet_t)(1u << (bit % 8u));
                        uint32_t flipped;

                        buf[byte] ^= mask;
                        flipped = ucrc_compute(model, buf, SENS_LEN);
                        buf[byte] ^= mask;
                        TEST_ASSERT(flipped != base);
                }
        }
}

void
run_known_answer_tests(void)
{
        run_test(test_check_constants, "check_constants");
        run_test(test_custom_model_bitwise_fallback,
                 "custom_model_bitwise_fallback");
        run_test(test_width_masking, "width_masking");
        run_test(test_exhaustive_single_octet, "exhaustive_single_octet");
        run_test(test_single_bit_sensitivity, "single_bit_sensitivity");
}
