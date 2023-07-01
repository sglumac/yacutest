#include <yacu.h>
#include <others.h>
#define UNUSED(x) (void)(x)

void test_simple_eq_int(YacuTestRun *testRun)
{
    int x = 1;
    YACU_ASSERT_EQ_INT(testRun, x, 1);
}

YacuTest forOthers[] = {
    {"simpleEqInt", &test_simple_eq_int},
    END_OF_TESTS};

YacuSuite suites4Others[] = {
    {"ForOthers", forOthers},
    END_OF_SUITES};

void test_run_single_test(YacuTestRun *testRun)
{
    UNUSED(testRun);
    YacuOptions options = yacu_default_options();
    options.suiteName = "ForOthers";
    options.testName = "cmpIntTest";
    yacu_execute(options, suites4Others);
}

void test_run_single_suite(YacuTestRun *testRun)
{
    UNUSED(testRun);
    YacuOptions options = yacu_default_options();
    options.suiteName = "ForOthers";
    yacu_execute(options, suites4Others);
}

void test_fork(YacuTestRun *testRun)
{
    YacuProcessHandle pid = yacu_fork();
    if (is_forked(pid))
    {
        exit(FILE_FAIL);
    }
    else
    {
        YacuStatus returnCode = wait_for_forked(pid);
        YACU_ASSERT_EQ_INT(testRun, returnCode, FILE_FAIL);
    }
}

void test_junit_creation_fail(YacuTestRun *testRun)
{
    YacuProcessHandle pid = yacu_fork();
    if (is_forked(pid))
    {
        YacuOptions options = yacu_default_options();
        options.jUnitPath = "nonexistingdir/report.xml";
        yacu_execute(options, suites4Others);
    }
    else
    {
        YacuStatus returnCode = wait_for_forked(pid);
        YACU_ASSERT_EQ_INT(testRun, returnCode, FILE_FAIL);
    }
}

void test_junit_creation(YacuTestRun *testRun)
{
    UNUSED(testRun);
    YacuOptions options = yacu_default_options();
    options.jUnitPath = "success.xml";
    yacu_execute(options, suites4Others);
}

void test_run_single_suite_with_fork(YacuTestRun *testRun)
{
    UNUSED(testRun);
    YacuOptions options = yacu_default_options();
    options.suiteName = "ForOthers";
    yacu_execute(options, suites4Others);
}

YacuTest otherTests[] = {
    {"SingleTestTest", &test_run_single_test},
    {"SingleSuiteTest", &test_run_single_suite},
    {"SingleSuiteTestWithFork", &test_run_single_suite_with_fork},
    {"ForkTest", &test_fork},
    {"JUnitCreationFailTest", &test_junit_creation_fail},
    {"JUnitCreationTest", &test_junit_creation},
    END_OF_TESTS};
