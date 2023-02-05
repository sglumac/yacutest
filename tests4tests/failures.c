#include <yacu.h>
#include <failures.h>

typedef void ForkedAction(char *forkMessage);
typedef void MainAction(YacuTestRun *testRun, YacuStatus forkReturnCode, const char *forkMessage);

void test_using_fork(YacuTestRun *testRun, MainAction mainAction, ForkedAction forkedAction)
{
    YacuProcessHandle pid = yacu_fork();
    char forkMessage[YACU_TEST_RUN_MESSAGE_MAX_SIZE] = "";
    if (is_forked(pid))
    {
        forkedAction(forkMessage);
    }
    YacuStatus forkReturnCode = wait_for_forked(pid);
    mainAction(testRun, forkReturnCode, forkMessage);
}

void forked_assert_failed_cmp_int(char *forkMessage)
{
    YacuReportPtr reports[] = {&END_OF_REPORTS};
    YacuTestRun forkedTestRun = {.result = OK, .message = "", .forked = false, .reports = reports};

    int small = -1;

    YACU_ASSERT_LT_INT(&forkedTestRun, small, -2);
}

void main_assert_failed_cmp_int(YacuTestRun *testRun, YacuStatus forkReturnCode, const char *forkMessage)
{
    YACU_ASSERT_EQ_INT(testRun, forkReturnCode, TEST_FAILURE);
}

void test_assert_failed_cmp_int(YacuTestRun *testRun)
{
    test_using_fork(testRun, main_assert_failed_cmp_int, forked_assert_failed_cmp_int);
}

YacuTest assertionFailuresTests[] = {
    {"failedCmpIntTest", &test_assert_failed_cmp_int},
};
