/**
 * SPDX-License-Identifier: MIT
 *
 * @file ucrc.h
 *
 * @brief Define the public ucrc API.
 *
 * @details
 *    ucrc computes CRC-8, CRC-16, and CRC-32 values from 8-bit octets. A
 *    runtime @c ucrc_model_t descriptor defines the polynomial, initial
 *    value, reflection, and final XOR. One image can use different models.
 *    @c ucrc_conf.h selects the bitwise, nibble-table, or byte-table strategy
 *    at compile time.
 *
 *    ucrc provides common standard models. A table strategy stores their
 *    lookup tables in flash or @c .rodata. The tables use no RAM.
 *
 *    Use @c ucrc_compute for a complete buffer. Use @c ucrc_begin,
 *    @c ucrc_update, and @c ucrc_finish for data that arrives in chunks. The
 *    caller stores the running value. The API supports independent streams.
 *
 *    A @c uint32_t stores the register for each CRC width. ucrc masks results
 *    to the model width. The library requires C11 and uses @c _Static_assert.
 *
 *    @par 16-bit-MAU targets
 *    Each @c ucrc_octet_t stores one octet, but ucrc uses only its low 8 bits.
 *    @c len always counts octets. Split packed 16-bit words before you call
 *    ucrc. The library does not select octets from a packed word.
 */

#ifndef UCRC_H_
#define UCRC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ================ INCLUDES ================================================ */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ucrc_conf.h"
#include "ucrc_platform.h"

/**
 * @defgroup ucrc_api ucrc Library
 * @brief Cyclic redundancy check (CRC-8 / CRC-16 / CRC-32).
 * @{
 */

/* ================ STRUCTURES ============================================== */

/**
 * @brief Define a CRC model with Rocksoft parameters.
 *
 * @details
 *    This structure completely defines a CRC. ucrc provides @c const models
 *    for common standards. An application can define its own model. For a
 *    table strategy, @c table points to a @c uint32_t lookup table that
 *    matches @c poly, @c width, and @c refin. A NULL @c table selects the
 *    bitwise engine in every build.
 *
 * @note Table strategies require @c refin and @c refout to be equal. The
 *       bitwise engine supports each value independently.
 */
typedef struct {
        uint32_t poly;         /**< Generator polynomial in normal form. */
        uint32_t init;         /**< Initial register value.              */
        uint32_t xorout;       /**< Final XOR mask.                      */
        const uint32_t *table; /**< Strategy table. NULL selects bitwise.*/
        uint_fast8_t width;    /**< CRC width: 8, 16, or 32 bits.        */
        bool refin;            /**< Reflect input octets when true.      */
        bool refout;           /**< Reflect the output register if true. */
} ucrc_model_t;

/* ================ PREDEFINED MODELS ======================================= */

#if UCRC_ENABLE_CRC8
/**
 * @brief Define CRC-8/SMBUS.
 *
 * The polynomial is 0x07. The initial value and XOR mask are 0x00.
 * @note The @c check value is 0xF4.
 */
extern const ucrc_model_t ucrc_crc8_smbus;
#endif

#if UCRC_ENABLE_CRC16
/**
 * @brief Define CRC-16/CCITT-FALSE.
 *
 * The polynomial is 0x1021. The initial value is 0xFFFF.
 * @note The @c check value is 0x29B1.
 */
extern const ucrc_model_t ucrc_crc16_ccitt_false;
#endif

#if UCRC_ENABLE_CRC32
/**
 * @brief Define CRC-32/ISO-HDLC for zlib, Ethernet, gzip, and PNG.
 *
 * The polynomial is 0x04C11DB7. The initial value and XOR mask are
 * 0xFFFFFFFF. This model reflects input and output.
 * @note The @c check value is 0xCBF43926.
 */
extern const ucrc_model_t ucrc_crc32_iso_hdlc;
#endif

/* ================ GLOBAL PROTOTYPES ======================================= */

/**
 * @brief Compute a complete CRC over a buffer in one call.
 *
 * @pre @p model is a valid, initialized @c ucrc_model_t.
 * @pre If @p len is greater than zero, @p data is not NULL.
 * @post The result applies @c init and @c xorout. ucrc masks it to the model
 *       width.
 * @post This function does not modify internal or static state.
 *
 * @param[in] model The CRC model descriptor.
 * @param[in] data The octet stream. Each addressable unit stores one octet.
 * @param[in] len The number of octets.
 *
 * @return The CRC value, masked to @p model->width. Returns zero if @p model
 *         is NULL. UCRC_ASSERT also reports this contract violation.
 *
 * @warning On 16-bit-MAU targets only the low 8 bits of each unit are used.
 */
uint32_t ucrc_compute(const ucrc_model_t *model, const ucrc_octet_t *data,
                      size_t len);

/**
 * @brief Start a streaming CRC and return its initial register.
 *
 * @pre @p model is valid and not NULL.
 * @post Pass the opaque returned register to @c ucrc_update.
 *
 * @param[in] model The CRC model descriptor.
 * @return The initial running register. Do not interpret it directly.
 */
uint32_t ucrc_begin(const ucrc_model_t *model);

/**
 * @brief Add an octet chunk to a running CRC register.
 *
 * @pre @p model is valid and not NULL.
 * @pre @p crc came from @c ucrc_begin or @c ucrc_update for @p model.
 * @pre If @p len is greater than zero, @p data is not NULL.
 * @post The returned register includes all octets so far. @p crc is unchanged.
 *
 * @param[in] model The CRC model descriptor.
 * @param[in] crc The register from @c ucrc_begin or @c ucrc_update.
 * @param[in] data The octet stream. Each addressable unit stores one octet.
 * @param[in] len The octet count in this chunk. Zero performs no operation.
 * @return The updated running register.
 *
 * @warning On 16-bit-MAU targets only the low 8 bits of each unit are used.
 */
uint32_t ucrc_update(const ucrc_model_t *model, uint32_t crc,
                     const ucrc_octet_t *data, size_t len);

/**
 * @brief Finish a streaming CRC and return the result.
 *
 * @pre @p model is valid and not NULL.
 * @pre @p crc came from @c ucrc_begin or @c ucrc_update for @p model.
 * @post The result applies @c refout and @c xorout. ucrc masks it to the
 *       model width.
 *
 * @param[in] model The CRC model descriptor.
 * @param[in] crc The final running register.
 * @return The CRC value, masked to @p model->width.
 */
uint32_t ucrc_finish(const ucrc_model_t *model, uint32_t crc);

/** @} */ /* End of ucrc_api. */

#ifdef __cplusplus
}
#endif

#endif /* UCRC_H_ */
