/**
 * SPDX-License-Identifier: MIT
 *
 * @file ucrc_platform.h
 *
 * @brief
 *    Platform abstraction for ucrc. Detects whether the target's minimum
 *    addressable unit (MAU) is 8 or 16 bits and exposes the octet storage
 *    type the library consumes.
 *
 *    A CRC is defined over a stream of 8-bit octets. On byte-addressable
 *    targets one octet occupies one @c uint8_t. On word-addressable targets
 *    where @c CHAR_BIT is 16 the smallest storage unit is 16 bits, so one
 *    octet occupies a @c uint16_t; an exact-width @c uint8_t need not exist
 *    at all on such a target, and the library does not rely on one.
 *
 *    ucrc consumes the input as one logical octet per addressable unit and
 *    uses only the low 8 bits of each unit (see @c ucrc_octet_t). The CRC
 *    value is therefore bit-identical across MAU sizes for the same octet
 *    stream, and @c len always counts octets.
 *
 *    Define @c UCRC_SIMULATE_16BIT_MAU at compile time on a byte-addressable
 *    host to exercise the word-addressable path against host unit tests.
 *    This is for library development and test infrastructure; production
 *    builds should leave it undefined and rely on auto-detection.
 */

#ifndef UCRC_PLATFORM_H_
#define UCRC_PLATFORM_H_

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Auto-detection: a target is treated as 16-bit MAU when CHAR_BIT > 8 or
 * when UCHAR_MAX exceeds 255. UCRC_SIMULATE_16BIT_MAU forces this branch on
 * byte-addressable hosts for unit-test simulation.
 */
#if defined(UCRC_SIMULATE_16BIT_MAU) || CHAR_BIT > 8 || UCHAR_MAX > 255u
/** @brief Bits per minimum addressable unit on this target. */
#define UCRC_ADDR_UNIT_BITS 16u
/**
 * @brief Storage type for one logical octet of the CRC input stream.
 *
 * On a 16-bit-MAU target this is @c uint16_t (matching the target's
 * aliasing of @c uint8_t); only the low 8 bits of each element are consumed.
 */
typedef uint16_t ucrc_octet_t;
#else
#define UCRC_ADDR_UNIT_BITS 8u
typedef uint8_t ucrc_octet_t;
#endif

/* Catch unsupported configurations at compile time. */
_Static_assert((UCRC_ADDR_UNIT_BITS == 8u) || (UCRC_ADDR_UNIT_BITS == 16u),
               "ucrc only supports 8-bit or 16-bit addressable units");

#endif /* UCRC_PLATFORM_H_ */
