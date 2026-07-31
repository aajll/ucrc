/**
 * SPDX-License-Identifier: MIT
 *
 * @file ucrc.c
 *
 * @brief Implement the generic CRC engine.
 *
 * @details
 *    One runtime model descriptor supports each CRC width. The build selects
 *    the computation strategy in @c ucrc_conf.h.
 *
 *    The bitwise engine is always available for a model with a NULL @c table.
 *    A table-strategy build can therefore process custom models. Predefined
 *    models use a table that matches the selected strategy.
 *
 * @par MISRA C:2023 deviation record
 *    ucrc uses fixed-width unsigned types and explicit @c u suffixes. It
 *    does not use heap storage, recursion, or @c errno. It avoids undefined
 *    shift behavior and uses @c static @c const tables. It has no required
 *    rule deviations. @c misra-deviations.txt records advisory deviations.
 *
 *    @li Rule 15.5 requires one function exit. Public functions use early
 *        returns to handle NULL pointers and zero lengths. These guards make
 *        the no-undefined-behavior contract clear and keep the main path flat.
 *    @li Rule 8.7 concerns external linkage used in one translation unit.
 *        @c ucrc_compute is a public API function. Consumer translation units
 *        call it, but cppcheck cannot see them. The inline suppression is at
 *        the function definition.
 *
 *    Directive 4.9 concerns function-like macros. @c UCRC_ASSERT is a macro
 *    so an integrator can call a supervisor or remove the trap in a hardened
 *    build. Full tool-based compliance also needs a certified static analyzer.
 */

/* ================ INCLUDES ================================================ */

#include "ucrc.h"

/* ================ STATIC FUNCTIONS ======================================== */

/**
 * @brief Get a mask for the low @p width bits. Width 32 returns 0xFFFFFFFF.
 */
static uint32_t
ucrc_width_mask(uint_fast8_t width)
{
        return (width >= 32u) ? 0xFFFFFFFFu : (((uint32_t)1u << width) - 1u);
}

/**
 * @brief Reflect the low @p bits of @p value.
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
 * Table strategies keep the register reflected. The bitwise engine reflects
 * each input octet and keeps the register in normal form.
 */
static bool
ucrc_uses_reflected_register(const ucrc_model_t *model)
{
        return (model->table != NULL) && model->refin;
}

/**
 * @brief Update one CRC bit at a time.
 *
 * This function handles @c refin and @c refout independently. It keeps the
 * running register in normal form.
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
 * Update one octet with a 256-entry table. Each table uses @c uint32_t, so
 * this function supports each CRC width without a type cast. Reflected models
 * use a reflected register and right shifts. Other models use normal form and
 * left shifts. The function masks the register to the model width.
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
 * Update one octet with a 16-entry table. This function performs two lookups
 * per octet. Reflected models process the low nibble first. Other models
 * process the high nibble first.
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
        /* This build has no table engine. The bitwise engine handles every
         * model. */
        return ucrc_bitwise_update(model, crc, data, len);
#else
        /* Models with no table use the bitwise engine. */
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
/* cppcheck-suppress[misra-c2012-8.7] @deviation Public API function declared
 * in ucrc.h. Consumer translation units call it, but cppcheck sees one
 * translation unit. */
ucrc_compute(const ucrc_model_t *model, const ucrc_octet_t *data, size_t len)
{
        uint32_t crc = ucrc_begin(model);

        crc = ucrc_update(model, crc, data, len);
        return ucrc_finish(model, crc);
}
