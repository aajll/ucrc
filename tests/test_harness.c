/**
 * SPDX-License-Identifier: MIT
 *
 * @file test_harness.c
 * @brief Implement the ucrc unit-test runner.
 */

#include "test_harness.h"

void
run_test(void (*test_func)(void), const char *name)
{
        test_func();
        TEST_PASS(name);
}
