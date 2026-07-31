/**
 * SPDX-License-Identifier: MIT
 *
 * @file test_harness.h
 *
 * @brief Define the shared unit-test harness.
 *
 * @details
 *    This header defines TEST_ASSERT, TEST_PASS, TEST_CASE, and run_test.
 *    Each category source file defines file-static test cases. It exposes one
 *    run_<category>_tests(void) function for test_main.c.
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

/** @brief Run one test function. Report PASS if it succeeds. */
void run_test(void (*test_func)(void), const char *name);

#endif /* TEST_HARNESS_H_ */
