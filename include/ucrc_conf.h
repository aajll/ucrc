/**
 * SPDX-License-Identifier: MIT
 *
 * @file ucrc_conf.h
 *
 * @brief Configure ucrc at compile time.
 *
 * @details
 *    Select the CRC widths, the computation strategy, and the precondition
 *    trap. @c ucrc.h includes this header automatically. Define an option
 *    before it includes this header. You can also define it in the toolchain.
 *
 * @note The CRC model is a runtime descriptor, not a compile-time option.
 *       This header configures the code and tables in the build.
 */

#ifndef UCRC_CONF_H_
#define UCRC_CONF_H_

#include "ucrc_platform.h"

/* ================ ENABLED WIDTHS ========================================== */
/*
 * A non-zero switch enables its CRC width. CRC-16 and CRC-32 are enabled by
 * default. Enable CRC-8 with -DUCRC_ENABLE_CRC8=1.
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
 * Each build selects one strategy. A table trades ROM for speed. The bitwise
 * engine handles models with no table (table == NULL) in every build.
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
 * UCRC_ASSERT reports contract violations, such as a NULL model. It is an
 * integration trap, not a production error channel. Public functions also
 * prevent undefined behavior when UCRC_ASSERT is disabled. An integrator can
 * redefine this macro to call a supervisor.
 */

#ifndef UCRC_ASSERT
#include <assert.h>
/** @brief Precondition trap. It defaults to the standard assert. */
#define UCRC_ASSERT(expr) assert(expr)
#endif

/* ================ VALIDATION ============================================== */

_Static_assert((UCRC_STRATEGY == UCRC_STRATEGY_BITWISE)
                   || (UCRC_STRATEGY == UCRC_STRATEGY_NIBBLE)
                   || (UCRC_STRATEGY == UCRC_STRATEGY_BYTE),
               "UCRC_STRATEGY must be one of the UCRC_STRATEGY_* values");

_Static_assert(UCRC_ENABLE_CRC8 || UCRC_ENABLE_CRC16 || UCRC_ENABLE_CRC32,
               "at least one CRC width must be enabled");

#endif /* UCRC_CONF_H_ */
