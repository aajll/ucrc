/**
 * SPDX-License-Identifier: MIT
 *
 * @file test_support.h
 *
 * @brief
 *    Shared fixtures and helpers for the ucrc test modules: the predefined
 *    model list, the standard check input, a deterministic PRNG, an
 *    independent reference CRC engine, and a CRC-append helper.
 *
 *    These are test utilities, kept separate from both the harness
 *    (test_harness.h) and the individual test cases.
 */

#ifndef TEST_SUPPORT_H_
#define TEST_SUPPORT_H_

#include "ucrc.h"

#include <stddef.h>
#include <stdint.h>

/** @brief The nine ASCII bytes "123456789" used by the Rocksoft `check`. */
extern const ucrc_octet_t UCRC_CHECK_INPUT[9];
#define UCRC_CHECK_LEN    ((size_t)9u)

/** @brief All predefined models (CRC-8 is present when enabled in the build).
 */
#define UCRC_TEST_NMODELS ((size_t)3u)
extern const ucrc_model_t *const UCRC_TEST_MODELS[UCRC_TEST_NMODELS];

/**
 * @brief Deterministic linear-congruential PRNG.
 * @param state Caller-owned 32-bit state, advanced in place.
 * @return Next pseudo-random 32-bit value.
 */
uint32_t ucrc_test_rng(uint32_t *state);

/**
 * @brief Independent reference CRC, deliberately a different formulation from
 *        the library so a shared bug is unlikely to hide in both.
 */
uint32_t ucrc_test_ref(const ucrc_model_t *model, const ucrc_octet_t *data,
                       size_t len);

/**
 * @brief Append @p crc to @p buf after @p len octets in the model's wire
 *        order (little-endian for reflected models, big-endian otherwise).
 * @return The new length, @p len + model->width / 8.
 */
size_t ucrc_test_append_crc(const ucrc_model_t *model, ucrc_octet_t *buf,
                            size_t len, uint32_t crc);

#endif /* TEST_SUPPORT_H_ */
