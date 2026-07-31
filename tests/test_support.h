/**
 * SPDX-License-Identifier: MIT
 *
 * @file test_support.h
 *
 * @brief Define shared data and functions for ucrc tests.
 *
 * @details
 *    This header defines the model list, the standard check input, a
 *    deterministic PRNG, a reference CRC engine, and a CRC append helper.
 *    These utilities are separate from the harness and test cases.
 */

#ifndef TEST_SUPPORT_H_
#define TEST_SUPPORT_H_

#include "ucrc.h"

#include <stddef.h>
#include <stdint.h>

/** @brief The nine ASCII octets "123456789" for the Rocksoft @c check. */
extern const ucrc_octet_t UCRC_CHECK_INPUT[9];
#define UCRC_CHECK_LEN    ((size_t)9u)

/** @brief All predefined models. The build can enable CRC-8. */
#define UCRC_TEST_NMODELS ((size_t)3u)
extern const ucrc_model_t *const UCRC_TEST_MODELS[UCRC_TEST_NMODELS];

/**
 * @brief Get the next value from a deterministic linear congruential PRNG.
 * @param state A caller-owned 32-bit state. The function updates it.
 * @return The next pseudo-random 32-bit value.
 */
uint32_t ucrc_test_rng(uint32_t *state);

/**
 * @brief Compute a reference CRC with a different algorithm than ucrc.
 *
 * This helps detect a defect shared by the test and library algorithms.
 */
uint32_t ucrc_test_ref(const ucrc_model_t *model, const ucrc_octet_t *data,
                       size_t len);

/**
 * @brief Append @p crc to @p buf in the model wire order.
 *
 * Reflected models use little-endian order. Other models use big-endian order.
 * @return The new length: @p len + model->width / 8.
 */
size_t ucrc_test_append_crc(const ucrc_model_t *model, ucrc_octet_t *buf,
                            size_t len, uint32_t crc);

#endif /* TEST_SUPPORT_H_ */
