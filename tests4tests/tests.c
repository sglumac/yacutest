#include <yacu.h>
#include <stddef.h>

void test_assert_cmp_int(YacuTestRun *testRun)
{
    int small = -1;

    YACU_ASSERT_LT_INT(testRun, small, 0);
    YACU_ASSERT_LE_INT(testRun, small, -1);
    YACU_ASSERT_EQ_INT(testRun, small, -1);
    YACU_ASSERT_GE_INT(testRun, small, -1);
    YACU_ASSERT_GT_INT(testRun, small, -2);
}

void test_assert_cmp_uint(YacuTestRun *testRun)
{
    unsigned int small = 1;

    YACU_ASSERT_LT_UINT(testRun, small, 2);
    YACU_ASSERT_LE_UINT(testRun, small, 1);
    YACU_ASSERT_EQ_UINT(testRun, small, 1);
    YACU_ASSERT_GE_UINT(testRun, small, 1);
    YACU_ASSERT_GT_UINT(testRun, small, 0);
}

void test_assert_eq_dbl(YacuTestRun *testRun)
{
    double x = 1.1;

    YACU_ASSERT_APPROX_EQ_DBL(testRun, x, 1.2, 0.2)
}

YacuTest assertionTests[] = {
    {"cmpIntTest", &test_assert_cmp_int},
    {"cmpUIntTest", &test_assert_cmp_uint},
    {"eqDblTest", &test_assert_eq_dbl},
    END_OF_TESTS};

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


extern YacuTest otherTests[];

YacuSuite suites[] = {
    {"Assertions", assertionTests},
    {"AssertionFailures", assertionFailuresTests},
    {"Others", otherTests},
    END_OF_SUITES};

int main(int argc, char const *argv[])
{
    YacuOptions options = yacu_process_args(argc, argv);
    return yacu_execute(options, suites);
}
