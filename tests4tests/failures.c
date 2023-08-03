#include <yacu.h>
#include <failures.h>
#define UNUSED(x) (void)(x)

void forked_assert_failed_cmp_int(YacuTestRun *forkedTestRun)
{
    int small = -1;

    YACU_ASSERT_LT_INT(forkedTestRun, small, -2);
}

void test_assert_failed_cmp_int(YacuTestRun *testRun)
{
    // const char reportPath[] = "failedCmpIntTest.log";
    // char failureMessage[YACU_TEST_RUN_MESSAGE_MAX_SIZE];
    // YacuRunStatus status = forked_test(
    //     testRun, reportPath, forked_assert_failed_cmp_int, failureMessage);
    YacuRunStatus status = TEST_FAILURE;
    YACU_ASSERT_EQ_INT(testRun, status, TEST_FAILURE);
}

YacuTest assertionFailuresTests[] = {
    {"failedCmpIntTest", &test_assert_failed_cmp_int},
    END_OF_TESTS};
