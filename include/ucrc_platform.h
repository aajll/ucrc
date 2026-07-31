/**
 * SPDX-License-Identifier: MIT
 *
 * @file ucrc_platform.h
 *
 * @brief Define platform types for ucrc.
 *
 * @details
 *    This header detects an 8-bit or 16-bit minimum addressable unit (MAU).
 *    It then defines the input-octet storage type.
 *
 *    A CRC uses a stream of 8-bit octets. On byte-addressable targets, an
 *    octet uses one @c uint8_t. If @c CHAR_BIT is 16, an octet uses one
 *    @c uint16_t. An exact-width @c uint8_t might not exist on that target.
 *
 *    ucrc reads one logical octet from each addressable unit. It uses only
 *    the low 8 bits. The CRC and @c len are the same for each MAU size.
 *
 *    Define @c UCRC_SIMULATE_16BIT_MAU on an 8-bit-MAU host to test the
 *    16-bit-MAU path. Do not define it in a production build.
 */

#ifndef UCRC_PLATFORM_H_
#define UCRC_PLATFORM_H_

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

/*
 * The target uses a 16-bit MAU when CHAR_BIT is greater than 8 or UCHAR_MAX
 * is greater than 255. UCRC_SIMULATE_16BIT_MAU selects this path on a host.
 */
#if defined(UCRC_SIMULATE_16BIT_MAU) || CHAR_BIT > 8 || UCHAR_MAX > 255u
/** @brief Bits per minimum addressable unit on this target. */
#define UCRC_ADDR_UNIT_BITS 16u
/**
 * @brief Storage type for one logical octet of the CRC input stream.
 *
 * On a 16-bit-MAU target, this type is @c uint16_t. ucrc uses only the low
 * 8 bits of each element.
 */
typedef uint16_t ucrc_octet_t;
#else
#define UCRC_ADDR_UNIT_BITS 8u
typedef uint8_t ucrc_octet_t;
#endif

/* Report unsupported configurations at compile time. */
_Static_assert((UCRC_ADDR_UNIT_BITS == 8u) || (UCRC_ADDR_UNIT_BITS == 16u),
               "ucrc only supports 8-bit or 16-bit addressable units");

#endif /* UCRC_PLATFORM_H_ */
