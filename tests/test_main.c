/**
 * SPDX-License-Identifier: MIT
 *
 * @file test_main.c
 *
 * @brief Run the ucrc unit-test suite.
 *
 * @details
 *    Each category module provides a run_<category>_tests function. This file
 *    calls each function. The build tests every strategy and the 16-bit-MAU
 *    simulation against published CRC check values and a reference engine.
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
