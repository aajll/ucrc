/**
 * SPDX-License-Identifier: MIT
 *
 * @file ucrc.c
 *
 * @brief
 *    Generic CRC engine. One runtime model descriptor drives every width;
 *    the computation strategy is fixed at compile time (see ucrc_conf.h).
 *
 *    The bitwise engine is always compiled and serves any model whose
 *    @c table is NULL (arbitrary application-defined polynomials), so a
 *    table-strategy build still handles untabulated models. Predefined
 *    models carry a table matching the build strategy and use the fast path.
 *
 * @par MISRA C:2023 deviation record
 *    ucrc is written to be MISRA C:2023 aware (fixed-width unsigned types,
 *    explicit @c u suffixes, no heap, no recursion, no @c errno, no
 *    undefined-behaviour shifts, @c static @c const tables). There are zero
 *    required-rule deviations. The advisory-rule deviations below are the
 *    machine-checked record and mirror the misch/cppcheck deviation report
 *    (@c misra-deviations.txt plus the inline suppression comments):
 *    @li Rule 15.5 (single point of exit): the public functions use early
 *        guard-clause returns for the defensive NULL / @c len==0 contract
 *        (@c ucrc_begin, @c ucrc_update, @c ucrc_finish). Justification: the
 *        guards make the no-undefined-behaviour contract explicit and keep the
 *        happy path unnested and auditable. Deviated project-wide in
 *        @c misra-deviations.txt.
 *    @li Rule 8.7 (external linkage referenced in one translation unit):
 *        @c ucrc_compute is a public entry point declared in @c ucrc.h and is
 *        called only by consumer translation units outside this library, so
 *        cppcheck sees a single TU; it cannot be made @c static. Suppressed
 *        inline at its definition.
 *    Directive 4.9 (function-like macro) is a further deliberate deviation the
 *    automated rule set does not check: @c UCRC_ASSERT is a macro so integrators
 *    can redirect it to a supervisor and so it compiles out entirely in a
 *    hardened build. Full tool-driven compliance additionally requires a
 *    certified static analyser, which this repository does not vendor.
 */

/* ================ INCLUDES ================================================ */

#include "ucrc.h"

/* ================ STATIC FUNCTIONS ======================================== */

/**
 * @brief Mask covering @p width low bits (0xFFFFFFFF for width 32).
 */
static uint32_t
ucrc_width_mask(uint_fast8_t width)
{
        return (width >= 32u) ? 0xFFFFFFFFu : (((uint32_t)1u << width) - 1u);
}

/**
 * @brief Reflect the low @p bits bits of @p value (bit 0 <-> bit bits-1).
 */
static uint32_t
ucrc_reflect(uint32_t value, uint_fast8_t bits)
{
        uint32_t result = 0u;
        uint_fast8_t i;

        for (i = 0u; i < bits; i++) {
                if (((value >> i) & 1u) != 0u) {
                        result |= (uint32_t)1u << (uint_fast8_t)(bits - 1u - i);
                }
        }
        return result;
}

/**
 * @brief True when @p model is processed with a reflected running register.
 *
 * Only the table strategies keep the register reflected; the bitwise engine
 * reflects each input octet instead and keeps the register in normal form.
 */
static bool
ucrc_uses_reflected_register(const ucrc_model_t *model)
{
        return (model->table != NULL) && model->refin;
}

/**
 * @brief Bit-by-bit update. Handles @c refin / @c refout independently and
 *        keeps the running register in normal (non-reflected) form.
 */
static uint32_t
ucrc_bitwise_update(const ucrc_model_t *model, uint32_t crc,
                    const ucrc_octet_t *data, size_t len)
{
        uint32_t mask = ucrc_width_mask(model->width);
        uint32_t topbit = (uint32_t)1u << (uint_fast8_t)(model->width - 1u);
        uint32_t reg = crc;
        size_t i;

        for (i = 0u; i < len; i++) {
                uint32_t octet = (uint32_t)(data[i] & 0xFFu);
                uint_fast8_t bit;

                if (model->refin) {
                        octet = ucrc_reflect(octet, 8u);
                }
                for (bit = 0u; bit < 8u; bit++) {
                        uint32_t top = reg & topbit;

                        reg = (reg << 1) & mask;
                        if (((octet >> (uint_fast8_t)(7u - bit)) & 1u) != 0u) {
                                top ^= topbit;
                        }
                        if (top != 0u) {
                                reg ^= model->poly;
                        }
                }
                reg &= mask;
        }
        return reg;
}

#if UCRC_STRATEGY == UCRC_STRATEGY_BYTE

/*
 * Byte-wise (256-entry) update. Tables are uint32_t for every width, so one
 * function serves CRC-8/16/32 with no per-type specialisation and no cast.
 * The table matches model->refin: reflected models keep the register
 * reflected (right-shift), non-reflected models keep it normal (left-shift
 * with masking to the model width).
 */
static uint32_t
ucrc_table_update(const ucrc_model_t *model, uint32_t crc,
                  const ucrc_octet_t *data, size_t len)
{
        const uint32_t *table = model->table;
        uint32_t reg = crc;
        size_t i;

        if (model->refin) {
                for (i = 0u; i < len; i++) {
                        uint32_t o = (uint32_t)(data[i] & 0xFFu);

                        reg = (reg >> 8) ^ table[(reg ^ o) & 0xFFu];
                }
        } else {
                uint32_t mask = ucrc_width_mask(model->width);
                uint_fast8_t sh = (uint_fast8_t)(model->width - 8u);

                for (i = 0u; i < len; i++) {
                        uint32_t o = (uint32_t)(data[i] & 0xFFu);

                        reg = ((reg << 8) ^ table[((reg >> sh) ^ o) & 0xFFu])
                              & mask;
                }
        }
        return reg;
}

#elif UCRC_STRATEGY == UCRC_STRATEGY_NIBBLE

/*
 * Half-byte (16-entry) update, two lookups per octet. Tables are uint32_t for
 * every width, so one function serves all widths with no cast. Reflected
 * processes the low nibble first; non-reflected processes the high nibble
 * first.
 */
static uint32_t
ucrc_table_update(const ucrc_model_t *model, uint32_t crc,
                  const ucrc_octet_t *data, size_t len)
{
        const uint32_t *table = model->table;
        uint32_t reg = crc;
        size_t i;

        if (model->refin) {
                for (i = 0u; i < len; i++) {
                        uint32_t o = (uint32_t)(data[i] & 0xFFu);

                        reg = (reg >> 4) ^ table[(reg ^ o) & 0xFu];
                        reg = (reg >> 4) ^ table[(reg ^ (o >> 4)) & 0xFu];
                }
        } else {
                uint32_t mask = ucrc_width_mask(model->width);
                uint_fast8_t sh = (uint_fast8_t)(model->width - 4u);

                for (i = 0u; i < len; i++) {
                        uint32_t o = (uint32_t)(data[i] & 0xFFu);

                        reg = ((reg << 4)
                               ^ table[((reg >> sh) ^ (o >> 4)) & 0xFu])
                              & mask;
                        reg = ((reg << 4) ^ table[((reg >> sh) ^ o) & 0xFu])
                              & mask;
                }
        }
        return reg;
}

#endif /* UCRC_STRATEGY */

/* ================ GLOBAL FUNCTIONS ======================================== */

uint32_t
ucrc_begin(const ucrc_model_t *model)
{
        uint32_t init;

        UCRC_ASSERT(model != NULL);
        if (model == NULL) {
                return 0u;
        }

        init = model->init & ucrc_width_mask(model->width);
        if (ucrc_uses_reflected_register(model)) {
                return ucrc_reflect(init, model->width);
        }
        return init;
}

uint32_t
ucrc_update(const ucrc_model_t *model, uint32_t crc, const ucrc_octet_t *data,
            size_t len)
{
        UCRC_ASSERT(model != NULL);
        if (model == NULL) {
                return crc;
        }
        if (len == 0u) {
                return crc;
        }
        UCRC_ASSERT(data != NULL);
        if (data == NULL) {
                return crc;
        }

#if UCRC_STRATEGY == UCRC_STRATEGY_BITWISE
        /* Bitwise build: the table engine is not compiled; every model,
         * tabulated or not, is served by the bitwise engine. */
        return ucrc_bitwise_update(model, crc, data, len);
#else
        /* Table build: untabulated (custom) models fall back to bitwise. */
        if (model->table == NULL) {
                return ucrc_bitwise_update(model, crc, data, len);
        }
        return ucrc_table_update(model, crc, data, len);
#endif
}

uint32_t
ucrc_finish(const ucrc_model_t *model, uint32_t crc)
{
        uint32_t out;

        UCRC_ASSERT(model != NULL);
        if (model == NULL) {
                return 0u;
        }

        if (ucrc_uses_reflected_register(model)) {
                out = model->refout ? crc : ucrc_reflect(crc, model->width);
        } else {
                out = model->refout ? ucrc_reflect(crc, model->width) : crc;
        }
        out ^= model->xorout;
        return out & ucrc_width_mask(model->width);
}

uint32_t
/* cppcheck-suppress[misra-c2012-8.7] ; @deviation public API entry point
 * declared in ucrc.h; referenced only by consumer TUs outside this library,
 * so cppcheck sees a single translation unit */
ucrc_compute(const ucrc_model_t *model, const ucrc_octet_t *data, size_t len)
{
        uint32_t crc = ucrc_begin(model);

        crc = ucrc_update(model, crc, data, len);
        return ucrc_finish(model, crc);
}
