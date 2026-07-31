/**
 * SPDX-License-Identifier: MIT
 *
 * @file test_fuzz.c
 * @brief Test random data, large buffers, and the CRC residue property.
 */

#include "test_harness.h"
#include "test_support.h"
#include "ucrc.h"

#define FUZZ_BUF 256u

/* Test random data against the reference implementation. */
TEST_CASE(test_fuzz_vs_reference)
{
        ucrc_octet_t buf[FUZZ_BUF];
        uint32_t rng = 0x12345678u;
        unsigned trial;

        for (trial = 0u; trial < 4000u; trial++) {
                size_t len = (size_t)(ucrc_test_rng(&rng) % (FUZZ_BUF + 1u));
                size_t i;
                size_t m;

                for (i = 0u; i < len; i++) {
                        buf[i] = (ucrc_octet_t)(ucrc_test_rng(&rng) & 0xFFu);
                }
                for (m = 0u; m < UCRC_TEST_NMODELS; m++) {
                        TEST_ASSERT(
                            ucrc_compute(UCRC_TEST_MODELS[m], buf, len)
                            == ucrc_test_ref(UCRC_TEST_MODELS[m], buf, len));
                }
        }
}

#define BIG_BUF 4096u

/* Test a large buffer against the reference implementation. */
TEST_CASE(test_large_buffer)
{
        static ucrc_octet_t buf[BIG_BUF];
        uint32_t rng = 0xDEADBEEFu;
        unsigned trial;

        for (trial = 0u; trial < 16u; trial++) {
                size_t i;
                size_t m;

                for (i = 0u; i < BIG_BUF; i++) {
                        buf[i] = (ucrc_octet_t)(ucrc_test_rng(&rng) & 0xFFu);
                }
                for (m = 0u; m < UCRC_TEST_NMODELS; m++) {
                        TEST_ASSERT(
                            ucrc_compute(UCRC_TEST_MODELS[m], buf, BIG_BUF)
                            == ucrc_test_ref(UCRC_TEST_MODELS[m], buf,
                                             BIG_BUF));
                }
        }
}

/* A correct codeword has a model-specific residue. The residue does not
 * depend on the message. A one-bit codeword error changes the residue. */
TEST_CASE(test_residue_property)
{
        ucrc_octet_t buf[FUZZ_BUF + 4u];
        uint32_t rng = 0x1BADB002u;
        size_t m;

        for (m = 0u; m < UCRC_TEST_NMODELS; m++) {
                const ucrc_model_t *model = UCRC_TEST_MODELS[m];
                uint32_t residue = 0u;
                int have_residue = 0;
                unsigned trial;

                for (trial = 0u; trial < 500u; trial++) {
                        size_t len =
                            1u + (size_t)(ucrc_test_rng(&rng) % FUZZ_BUF);
                        size_t i;
                        size_t total;
                        uint32_t crc;
                        uint32_t r;
                        size_t pos;
                        ucrc_octet_t bit;

                        for (i = 0u; i < len; i++) {
                                buf[i] =
                                    (ucrc_octet_t)(ucrc_test_rng(&rng) & 0xFFu);
                        }
                        crc = ucrc_compute(model, buf, len);
                        total = ucrc_test_append_crc(model, buf, len, crc);

                        r = ucrc_compute(model, buf, total);
                        if (!have_residue) {
                                residue = r;
                                have_residue = 1;
                        } else {
                                TEST_ASSERT(r == residue);
                        }

                        pos = (size_t)(ucrc_test_rng(&rng) % total);
                        bit = (ucrc_octet_t)(1u << (ucrc_test_rng(&rng) % 8u));
                        buf[pos] ^= bit;
                        TEST_ASSERT(ucrc_compute(model, buf, total) != residue);
                }
        }
}

void
run_fuzz_tests(void)
{
        run_test(test_fuzz_vs_reference, "fuzz_vs_reference");
        run_test(test_large_buffer, "large_buffer");
        run_test(test_residue_property, "residue_property");
}
