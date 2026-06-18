/**
 * SPDX-License-Identifier: MIT
 *
 * @file test_harness.h
 *
 * @brief
 *    Minimal test harness shared by the ucrc unit-test modules.
 *
 *    Provides the TEST_ASSERT / TEST_PASS / TEST_CASE macros and the
 *    run_test runner. Each category source file defines its TEST_CASEs with
 *    file-static linkage and exposes a single void run_<category>_tests(void)
 *    entry point that the driver in test_main.c calls.
 */

#ifndef TEST_HARNESS_H_
#define TEST_HARNESS_H_

#include <stdio.h>
#include <stdlib.h>

#define TEST_ASSERT(expr)                                                      \
        do {                                                                   \
                if (!(expr)) {                                                 \
                        fprintf(stderr, "FAIL  %s:%d  %s\n", __FILE__,         \
                                __LINE__, #expr);                              \
                        exit(EXIT_FAILURE);                                    \
                }                                                              \
        } while (0)

#define TEST_PASS(name) fprintf(stdout, "PASS  %s\n", (name))

#define TEST_CASE(name)                                                        \
        static void name(void);                                                \
        static void name(void)

/** @brief Run a single test function and report a PASS line on success. */
void run_test(void (*test_func)(void), const char *name);

#endif /* TEST_HARNESS_H_ */
