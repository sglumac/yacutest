#include <yacu.h>
#include <failures.h>
#include <common.h>

void forked_assert_failed_cmp_int(YacuTestRun *forkedTestRun)
{
    int small = -1;

    YACU_ASSERT_LT_INT(forkedTestRun, small, -2);
}

void test_assert_failed_cmp_int(YacuTestRun *testRun)
{
    const char reportPath[] = "failedCmpIntTest.log";
    char failureMessage[YACU_TEST_RUN_MESSAGE_MAX_SIZE];
    YacuStatus status = forked_test(
        testRun, reportPath, forked_assert_failed_cmp_int, failureMessage);
    YACU_ASSERT_EQ_INT(testRun, status, TEST_FAILURE);
    YACU_ASSERT_IN_STR(testRun, "failures.c:", failureMessage);
    YACU_ASSERT_IN_STR(testRun, " - Assertion small < -2 (-1 < -2) failed!", failureMessage);
}

YacuTest assertionFailuresTests[] = {
    {"failedCmpIntTest", &test_assert_failed_cmp_int},
    END_OF_TESTS};
