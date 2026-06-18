/**
 * SPDX-License-Identifier: MIT
 *
 * @file test_main.c
 *
 * @brief
 *    Driver for the ucrc unit-test suite. Each category module exposes a
 *    run_<category>_tests entry point; this file calls them in turn. The
 *    suite is built once per strategy and once with the 16-bit-MAU
 *    simulation (see tests/meson.build), so every code path runs against the
 *    published CRC check constants and the independent reference engine.
 */

#include "test_harness.h"

#include <stdio.h>

void run_known_answer_tests(void);
void run_streaming_tests(void);
void run_boundary_tests(void);
void run_fuzz_tests(void);

int
main(void)
{
        fprintf(stdout, "\n=== Running ucrc unit tests ===\n\n");

        run_known_answer_tests();
        run_streaming_tests();
        run_boundary_tests();
        run_fuzz_tests();

        fprintf(stdout, "\n=== All tests passed ===\n\n");
        return EXIT_SUCCESS;
}
