/**
 * SPDX-License-Identifier: MIT
 *
 * @file ucrc_conf.h
 *
 * @brief
 *    Compile-time configuration for ucrc.
 *
 * @details
 *    Selects which CRC widths are compiled in, which computation strategy
 *    is used, and the precondition-trap hook. It is included automatically
 *    by @c ucrc.h. Override any option by defining it before this header is
 *    first reached, or at the toolchain level (e.g. @c -DUCRC_STRATEGY=...).
 *
 * @note
 *    The CRC *model* (polynomial, init, reflection, xor-out) is a runtime
 *    descriptor (@c ucrc_model_t in @c ucrc.h), not a compile-time option.
 *    This header only configures which code and tables are built.
 */

#ifndef UCRC_CONF_H_
#define UCRC_CONF_H_

#include "ucrc_platform.h"

/* ================ ENABLED WIDTHS ========================================== */
/*
 * Each CRC width is compiled in only when its switch is non-zero. CRC-16 and
 * CRC-32 are on by default (the IPC and Flash/EEPROM consumers); CRC-8 is
 * included but off by default - enable it with -DUCRC_ENABLE_CRC8=1.
 */

#ifndef UCRC_ENABLE_CRC8
/** @brief Compile the CRC-8 model and tables when non-zero (default off). */
#define UCRC_ENABLE_CRC8 0
#endif

#ifndef UCRC_ENABLE_CRC16
/** @brief Compile the CRC-16 model and tables when non-zero (default on). */
#define UCRC_ENABLE_CRC16 1
#endif

#ifndef UCRC_ENABLE_CRC32
/** @brief Compile the CRC-32 model and tables when non-zero (default on). */
#define UCRC_ENABLE_CRC32 1
#endif

/* ================ COMPUTATION STRATEGY ==================================== */
/*
 * One strategy is selected per build, trading table ROM for speed. The
 * bitwise engine is always present as the fallback for models that carry no
 * table (table == NULL), so application-defined models work in any build.
 */

/** @brief Bit-by-bit strategy: no tables, smallest image, slowest. */
#define UCRC_STRATEGY_BITWISE 0
/** @brief Half-byte table strategy: 16-entry table, middle ground. */
#define UCRC_STRATEGY_NIBBLE  1
/** @brief Byte-wise table strategy: 256-entry table, fastest (default). */
#define UCRC_STRATEGY_BYTE    2

#ifndef UCRC_STRATEGY
/** @brief Selected computation strategy (default: byte-wise table). */
#define UCRC_STRATEGY UCRC_STRATEGY_BYTE
#endif

/* ================ PRECONDITION TRAP ======================================= */
/*
 * UCRC_ASSERT reports contract violations (e.g. a NULL model). It is a
 * development/integration trap, not a production error channel: every public
 * function also defends itself so that a build with UCRC_ASSERT disabled
 * never executes undefined behaviour. Safety-critical integrators may
 * redefine this to route into their supervisor (log + safe state).
 */

#ifndef UCRC_ASSERT
#include <assert.h>
/** @brief Overridable precondition trap. Defaults to the standard assert. */
#define UCRC_ASSERT(expr) assert(expr)
#endif

/* ================ VALIDATION ============================================== */

_Static_assert(UCRC_STRATEGY == UCRC_STRATEGY_BITWISE
                   || UCRC_STRATEGY == UCRC_STRATEGY_NIBBLE
                   || UCRC_STRATEGY == UCRC_STRATEGY_BYTE,
               "UCRC_STRATEGY must be one of the UCRC_STRATEGY_* values");

_Static_assert(UCRC_ENABLE_CRC8 || UCRC_ENABLE_CRC16 || UCRC_ENABLE_CRC32,
               "at least one CRC width must be enabled");

#endif /* UCRC_CONF_H_ */
