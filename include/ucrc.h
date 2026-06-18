/**
 * SPDX-License-Identifier: MIT
 *
 * @file ucrc.h
 *
 * @brief
 *    Public API for ucrc - a lean cyclic-redundancy-check library for
 *    embedded, RTOS, and Linux targets.
 *
 * @details
 *    ucrc computes CRC-8, CRC-16, and CRC-32 over a stream of 8-bit octets.
 *    The CRC *model* (polynomial, init, reflection, xor-out) is a runtime
 *    descriptor (@c ucrc_model_t), so a single image can use different CRC
 *    variants for different jobs. The computation *strategy* (bitwise,
 *    half-byte table, or byte-wise table) is selected once at compile time
 *    in @c ucrc_conf.h.
 *
 *    Predefined models for the common standards are provided
 *    (@c ucrc_crc16_ccitt_false, @c ucrc_crc32_iso_hdlc, and
 *    @c ucrc_crc8_smbus when CRC-8 is enabled). Their lookup tables, when a
 *    table strategy is selected, live in flash/`.rodata` and cost no RAM.
 *
 *    Two usage modes are offered per the same model descriptor:
 *    @li one-shot: @c ucrc_compute over a whole buffer; and
 *    @li streaming: @c ucrc_begin / @c ucrc_update / @c ucrc_finish for data
 *        that arrives in chunks. The running value is carried by the caller,
 *        so the API is reentrant and serves multiple independent streams.
 *
 *    A single @c uint32_t carries the register for every width; results are
 *    masked to the model's width. Requires C11 (uses @c _Static_assert).
 *
 *    ## 16-bit-MAU note
 *    On a 16-bit-MAU target one octet occupies one @c ucrc_octet_t and only
 *    its low 8 bits are used; the CRC is bit-identical to an 8-bit-MAU peer
 *    and @c len always counts octets. Where two octets are genuinely packed
 *    into one 16-bit word, decompose them first (the sibling @c ppack
 *    primitive is the intended tool); ucrc does not guess.
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
 * @brief Runtime descriptor for a CRC model (the Rocksoft parameters).
 *
 * @details
 *    Fully specifies a CRC. Predefined @c const instances are provided for
 *    the common standards; an application may also define its own. For a
 *    table strategy, @c table points to a lookup table matching @c poly,
 *    @c width, and @c refin; when @c table is @c NULL the bitwise engine is
 *    used regardless of the build strategy (the path for arbitrary
 *    application-defined polynomials).
 *
 * @note  @c refin and @c refout must be equal for table strategies; the
 *        bitwise engine supports them independently. The predefined models
 *        all satisfy @c refin @c == @c refout.
 */
typedef struct {
        uint32_t poly;     /**< Generator polynomial, normal form.        */
        uint32_t init;     /**< Initial register value (catalogue form).  */
        uint32_t xorout;   /**< Final XOR mask.                           */
        const void *table; /**< Strategy lookup table, or NULL = bitwise. */
        uint8_t width;     /**< CRC width in bits: 8, 16, or 32.          */
        bool refin;        /**< Reflect input octets when true.           */
        bool refout;       /**< Reflect output register when true.        */
} ucrc_model_t;

/* ================ PREDEFINED MODELS ======================================= */

#if UCRC_ENABLE_CRC8
/**
 * @brief CRC-8/SMBUS: poly 0x07, init 0x00, no reflection, xorout 0x00.
 * @note  Self-test constant (@c check) is 0xF4.
 */
extern const ucrc_model_t ucrc_crc8_smbus;
#endif

#if UCRC_ENABLE_CRC16
/**
 * @brief CRC-16/CCITT-FALSE: poly 0x1021, init 0xFFFF, no reflection.
 * @note  Self-test constant (@c check) is 0x29B1.
 */
extern const ucrc_model_t ucrc_crc16_ccitt_false;
#endif

#if UCRC_ENABLE_CRC32
/**
 * @brief CRC-32/ISO-HDLC (zlib / Ethernet / gzip / PNG): poly 0x04C11DB7,
 *        init/xorout 0xFFFFFFFF, reflected.
 * @note  Self-test constant (@c check) is 0xCBF43926.
 */
extern const ucrc_model_t ucrc_crc32_iso_hdlc;
#endif

/* ================ GLOBAL PROTOTYPES ======================================= */

/**
 * @brief Compute a complete CRC over a buffer in one call.
 *
 * @pre  @p model is not NULL and is a valid, fully-initialised
 *       @c ucrc_model_t (width 8, 16, or 32; any table matches the build
 *       strategy).
 * @pre  If @p len > 0 then @p data is not NULL.
 * @post The result has @c init and @c xorout applied and is masked to
 *       @p model->width; cast to the matching width type as needed.
 * @post No internal or static state is modified (the call is reentrant).
 *
 * @param[in] model CRC model descriptor.
 * @param[in] data  Octet stream; one logical octet per addressable unit.
 * @param[in] len   Number of octets.
 *
 * @return The CRC value, masked to @p model->width. Returns 0 if @p model
 *         is NULL (a contract violation that also trips @c UCRC_ASSERT).
 *
 * @warning On 16-bit-MAU targets only the low 8 bits of each unit are used.
 */
uint32_t ucrc_compute(const ucrc_model_t *model, const ucrc_octet_t *data,
                      size_t len);

/**
 * @brief Begin a streaming CRC; returns the initial running register.
 *
 * @pre   @p model is not NULL and valid.
 * @post  The returned value is an opaque running register for @p model,
 *        suitable to pass to @c ucrc_update.
 *
 * @param[in] model CRC model descriptor.
 * @return    Initial running register (opaque; do not interpret directly).
 */
uint32_t ucrc_begin(const ucrc_model_t *model);

/**
 * @brief Fold a chunk of octets into a running CRC register.
 *
 * @pre   @p model is not NULL and valid.
 * @pre   @p crc was produced by @c ucrc_begin or a prior @c ucrc_update for
 *        the same @p model.
 * @pre   If @p len > 0 then @p data is not NULL.
 * @post  The returned register reflects all octets folded so far; @p crc is
 *        unchanged (value semantics).
 *
 * @param[in] model CRC model descriptor.
 * @param[in] crc   Running register from @c ucrc_begin / @c ucrc_update.
 * @param[in] data  Octet stream; one logical octet per addressable unit.
 * @param[in] len   Number of octets in this chunk (0 is a valid no-op).
 * @return    Updated running register.
 *
 * @warning On 16-bit-MAU targets only the low 8 bits of each unit are used.
 */
uint32_t ucrc_update(const ucrc_model_t *model, uint32_t crc,
                     const ucrc_octet_t *data, size_t len);

/**
 * @brief Finalise a streaming CRC into the result value.
 *
 * @pre   @p model is not NULL and valid.
 * @pre   @p crc was produced by @c ucrc_begin / @c ucrc_update for the same
 *        @p model.
 * @post  The result has @c refout and @c xorout applied and is masked to
 *        @p model->width.
 *
 * @param[in] model CRC model descriptor.
 * @param[in] crc   Final running register.
 * @return    The CRC value, masked to @p model->width.
 */
uint32_t ucrc_finish(const ucrc_model_t *model, uint32_t crc);

/** @} */ /* end of ucrc_api */

#ifdef __cplusplus
}
#endif

#endif /* UCRC_H_ */
