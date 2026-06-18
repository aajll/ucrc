/**
 * SPDX-License-Identifier: MIT
 *
 * @file test_streaming.c
 * @brief One-shot vs streaming equivalence, fixed boundaries and fuzzed.
 */

#include "test_harness.h"
#include "test_support.h"
#include "ucrc.h"

/* One-shot must equal streaming for every chunk boundary, including 0-len. */
TEST_CASE(test_streaming_boundaries)
{
        size_t m;

        for (m = 0u; m < UCRC_TEST_NMODELS; m++) {
                const ucrc_model_t *model = UCRC_TEST_MODELS[m];
                uint32_t oneshot =
                    ucrc_compute(model, UCRC_CHECK_INPUT, UCRC_CHECK_LEN);
                size_t split;

                for (split = 0u; split <= UCRC_CHECK_LEN; split++) {
                        uint32_t crc = ucrc_begin(model);

                        crc = ucrc_update(model, crc, UCRC_CHECK_INPUT, split);
                        /* A zero-length update in the middle must be a no-op.
                         */
                        crc = ucrc_update(model, crc, UCRC_CHECK_INPUT, 0u);
                        crc = ucrc_update(model, crc, &UCRC_CHECK_INPUT[split],
                                          UCRC_CHECK_LEN - split);
                        TEST_ASSERT(ucrc_finish(model, crc) == oneshot);
                }
        }
}

#define FUZZ_BUF 256u

/* One-shot must equal streaming over random chunk boundaries. */
TEST_CASE(test_streaming_fuzz)
{
        ucrc_octet_t buf[FUZZ_BUF];
        uint32_t rng = 0x9E3779B9u;
        unsigned trial;

        for (trial = 0u; trial < 2000u; trial++) {
                size_t len = (size_t)(ucrc_test_rng(&rng) % (FUZZ_BUF + 1u));
                size_t i;
                size_t m;

                for (i = 0u; i < len; i++) {
                        buf[i] = (ucrc_octet_t)(ucrc_test_rng(&rng) & 0xFFu);
                }
                for (m = 0u; m < UCRC_TEST_NMODELS; m++) {
                        const ucrc_model_t *model = UCRC_TEST_MODELS[m];
                        uint32_t oneshot = ucrc_compute(model, buf, len);
                        uint32_t crc = ucrc_begin(model);
                        size_t pos = 0u;

                        while (pos < len) {
                                size_t chunk = (size_t)(ucrc_test_rng(&rng)
                                                        % (len - pos + 1u));

                                crc = ucrc_update(model, crc, &buf[pos], chunk);
                                pos += chunk;
                        }
                        TEST_ASSERT(ucrc_finish(model, crc) == oneshot);
                }
        }
}

void
run_streaming_tests(void)
{
        run_test(test_streaming_boundaries, "streaming_boundaries");
        run_test(test_streaming_fuzz, "streaming_fuzz");
}
