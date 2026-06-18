/**
 * SPDX-License-Identifier: MIT
 *
 * @file test_support.c
 * @brief Shared fixtures and helpers for the ucrc test modules.
 */

#include "test_support.h"

const ucrc_octet_t UCRC_CHECK_INPUT[9] = {'1', '2', '3', '4', '5',
                                          '6', '7', '8', '9'};

const ucrc_model_t *const UCRC_TEST_MODELS[UCRC_TEST_NMODELS] = {
    &ucrc_crc8_smbus,
    &ucrc_crc16_ccitt_false,
    &ucrc_crc32_iso_hdlc,
};

uint32_t
ucrc_test_rng(uint32_t *state)
{
        *state = (*state * 1664525u) + 1013904223u;
        return *state;
}

static uint32_t
ref_reflect(uint32_t value, uint8_t bits)
{
        uint32_t result = 0u;
        uint8_t i;

        for (i = 0u; i < bits; i++) {
                if ((value >> i) & 1u) {
                        result |= (uint32_t)1u << (uint8_t)(bits - 1u - i);
                }
        }
        return result;
}

uint32_t
ucrc_test_ref(const ucrc_model_t *model, const ucrc_octet_t *data, size_t len)
{
        uint32_t mask = (model->width >= 32u)
                            ? 0xFFFFFFFFu
                            : (((uint32_t)1u << model->width) - 1u);
        uint32_t topbit = (uint32_t)1u << (uint8_t)(model->width - 1u);
        uint32_t reg = model->init & mask;
        size_t i;

        for (i = 0u; i < len; i++) {
                uint32_t byte = (uint32_t)(data[i] & 0xFFu);
                uint8_t b;

                if (model->refin) {
                        byte = ref_reflect(byte, 8u);
                }
                reg ^= (byte << (uint8_t)(model->width - 8u)) & mask;
                for (b = 0u; b < 8u; b++) {
                        if (reg & topbit) {
                                reg = ((reg << 1) ^ model->poly) & mask;
                        } else {
                                reg = (reg << 1) & mask;
                        }
                }
        }
        if (model->refout) {
                reg = ref_reflect(reg, model->width);
        }
        return (reg ^ model->xorout) & mask;
}

size_t
ucrc_test_append_crc(const ucrc_model_t *model, ucrc_octet_t *buf, size_t len,
                     uint32_t crc)
{
        size_t nbytes = (size_t)(model->width / 8u);
        size_t i;

        for (i = 0u; i < nbytes; i++) {
                uint8_t shift = model->refout
                                    ? (uint8_t)(8u * i)
                                    : (uint8_t)(8u * (nbytes - 1u - i));

                buf[len + i] = (ucrc_octet_t)((crc >> shift) & 0xFFu);
        }
        return len + nbytes;
}
