/**
 * SPDX-License-Identifier: MIT
 *
 * @file test_assert_hook.h
 *
 * @brief
 *    Non-aborting UCRC_ASSERT for the assertion-recording test variant,
 *    force-included ahead of ucrc_conf.h. A failed precondition increments
 *    ucrc_test_assert_failures and execution continues into the defensive
 *    return, so the tests check both that the trap fires and that the
 *    function then returns without undefined behaviour.
 */

#ifndef TEST_ASSERT_HOOK_H_
#define TEST_ASSERT_HOOK_H_

/** @brief Number of failed UCRC_ASSERT checks since the test started. */
extern unsigned int ucrc_test_assert_failures;

#define UCRC_ASSERT(expr)                                                      \
        ((expr) ? (void)0 : (void)(ucrc_test_assert_failures++))

#endif /* TEST_ASSERT_HOOK_H_ */
