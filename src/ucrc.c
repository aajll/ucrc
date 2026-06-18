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
 */

/* ================ INCLUDES ================================================ */

#include "ucrc.h"

/* ================ STATIC FUNCTIONS ======================================== */

/**
 * @brief Mask covering @p width low bits (0xFFFFFFFF for width 32).
 */
static uint32_t
ucrc_width_mask(uint8_t width)
{
        return (width >= 32u) ? 0xFFFFFFFFu : (((uint32_t)1u << width) - 1u);
}

/**
 * @brief Reflect the low @p bits bits of @p value (bit 0 <-> bit bits-1).
 */
static uint32_t
ucrc_reflect(uint32_t value, uint8_t bits)
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
        uint32_t topbit = (uint32_t)1u << (uint8_t)(model->width - 1u);
        size_t i;

        for (i = 0u; i < len; i++) {
                uint32_t octet = (uint32_t)(data[i] & 0xFFu);
                uint8_t bit;

                if (model->refin) {
                        octet = ucrc_reflect(octet, 8u);
                }
                for (bit = 0u; bit < 8u; bit++) {
                        uint32_t top = crc & topbit;

                        crc = (crc << 1) & mask;
                        if ((octet >> (uint8_t)(7u - bit)) & 1u) {
                                top ^= topbit;
                        }
                        if (top != 0u) {
                                crc ^= model->poly;
                        }
                }
                crc &= mask;
        }
        return crc;
}

#if UCRC_STRATEGY == UCRC_STRATEGY_BYTE

/*
 * Byte-wise (256-entry) update, instantiated per table element width. The
 * table matches model->refin: reflected models keep the register reflected
 * (right-shift), non-reflected models keep it normal (left-shift + mask).
 */
#define UCRC_GEN_BYTE_UPDATE(SUFFIX, TYPE)                                     \
        static uint32_t ucrc_byte_update_##SUFFIX(                             \
            const ucrc_model_t *model, uint32_t crc, const ucrc_octet_t *data, \
            size_t len)                                                        \
        {                                                                      \
                const TYPE *table = (const TYPE *)model->table;                \
                size_t i;                                                      \
                if (model->refin) {                                            \
                        for (i = 0u; i < len; i++) {                           \
                                uint32_t o = (uint32_t)(data[i] & 0xFFu);      \
                                crc = (crc >> 8) ^ table[(crc ^ o) & 0xFFu];   \
                        }                                                      \
                } else {                                                       \
                        uint32_t mask = ucrc_width_mask(model->width);         \
                        uint8_t sh = (uint8_t)(model->width - 8u);             \
                        for (i = 0u; i < len; i++) {                           \
                                uint32_t o = (uint32_t)(data[i] & 0xFFu);      \
                                crc = ((crc << 8)                              \
                                       ^ table[((crc >> sh) ^ o) & 0xFFu])     \
                                      & mask;                                  \
                        }                                                      \
                }                                                              \
                return crc;                                                    \
        }

#if UCRC_ENABLE_CRC8
UCRC_GEN_BYTE_UPDATE(u8, uint8_t)
#endif
#if UCRC_ENABLE_CRC16
UCRC_GEN_BYTE_UPDATE(u16, uint16_t)
#endif
#if UCRC_ENABLE_CRC32
UCRC_GEN_BYTE_UPDATE(u32, uint32_t)
#endif

static uint32_t
ucrc_table_update(const ucrc_model_t *model, uint32_t crc,
                  const ucrc_octet_t *data, size_t len)
{
        switch (model->width) {
#if UCRC_ENABLE_CRC8
        case 8u: return ucrc_byte_update_u8(model, crc, data, len);
#endif
#if UCRC_ENABLE_CRC16
        case 16u: return ucrc_byte_update_u16(model, crc, data, len);
#endif
#if UCRC_ENABLE_CRC32
        case 32u: return ucrc_byte_update_u32(model, crc, data, len);
#endif
        default: UCRC_ASSERT(false); return crc;
        }
}

#elif UCRC_STRATEGY == UCRC_STRATEGY_NIBBLE

/*
 * Half-byte (16-entry) update, two lookups per octet. Reflected processes
 * the low nibble first; non-reflected processes the high nibble first.
 */
#define UCRC_GEN_NIBBLE_UPDATE(SUFFIX, TYPE)                                   \
        static uint32_t ucrc_nibble_update_##SUFFIX(                           \
            const ucrc_model_t *model, uint32_t crc, const ucrc_octet_t *data, \
            size_t len)                                                        \
        {                                                                      \
                const TYPE *table = (const TYPE *)model->table;                \
                size_t i;                                                      \
                if (model->refin) {                                            \
                        for (i = 0u; i < len; i++) {                           \
                                uint32_t o = (uint32_t)(data[i] & 0xFFu);      \
                                crc = (crc >> 4) ^ table[(crc ^ o) & 0xFu];    \
                                crc = (crc >> 4)                               \
                                      ^ table[(crc ^ (o >> 4)) & 0xFu];        \
                        }                                                      \
                } else {                                                       \
                        uint32_t mask = ucrc_width_mask(model->width);         \
                        uint8_t sh = (uint8_t)(model->width - 4u);             \
                        for (i = 0u; i < len; i++) {                           \
                                uint32_t o = (uint32_t)(data[i] & 0xFFu);      \
                                crc =                                          \
                                    ((crc << 4)                                \
                                     ^ table[((crc >> sh) ^ (o >> 4)) & 0xFu]) \
                                    & mask;                                    \
                                crc = ((crc << 4)                              \
                                       ^ table[((crc >> sh) ^ o) & 0xFu])      \
                                      & mask;                                  \
                        }                                                      \
                }                                                              \
                return crc;                                                    \
        }

#if UCRC_ENABLE_CRC8
UCRC_GEN_NIBBLE_UPDATE(u8, uint8_t)
#endif
#if UCRC_ENABLE_CRC16
UCRC_GEN_NIBBLE_UPDATE(u16, uint16_t)
#endif
#if UCRC_ENABLE_CRC32
UCRC_GEN_NIBBLE_UPDATE(u32, uint32_t)
#endif

static uint32_t
ucrc_table_update(const ucrc_model_t *model, uint32_t crc,
                  const ucrc_octet_t *data, size_t len)
{
        switch (model->width) {
#if UCRC_ENABLE_CRC8
        case 8u: return ucrc_nibble_update_u8(model, crc, data, len);
#endif
#if UCRC_ENABLE_CRC16
        case 16u: return ucrc_nibble_update_u16(model, crc, data, len);
#endif
#if UCRC_ENABLE_CRC32
        case 32u: return ucrc_nibble_update_u32(model, crc, data, len);
#endif
        default: UCRC_ASSERT(false); return crc;
        }
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

        if (model->table == NULL) {
                return ucrc_bitwise_update(model, crc, data, len);
        }
#if UCRC_STRATEGY == UCRC_STRATEGY_BITWISE
        return ucrc_bitwise_update(model, crc, data, len);
#else
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
ucrc_compute(const ucrc_model_t *model, const ucrc_octet_t *data, size_t len)
{
        uint32_t crc = ucrc_begin(model);

        crc = ucrc_update(model, crc, data, len);
        return ucrc_finish(model, crc);
}
