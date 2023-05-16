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

void test_list_suites(YacuTestRun *testRun)
{
    const char *argv[] = {"./tests", "--list", "--no-fork"};
    YacuOptions options = yacu_process_args(3, argv);
    YacuStatus returnCode = yacu_execute(options, suites4Others);
    YACU_ASSERT_EQ_INT(testRun, returnCode, OK);
}

void test_help(YacuTestRun *testRun)
{
    const char *argv[] = {"./tests", "--help"};
    YacuOptions options = yacu_process_args(2, argv);
    YacuStatus returnCode = yacu_execute(options, suites4Others);
    YACU_ASSERT_EQ_INT(testRun, returnCode, OK);
}

void test_run_single_test(YacuTestRun *testRun)
{
    const char *argv[] = {"./tests", "--test", "ForOthers", "cmpIntTest", "--no-fork"};
    YacuOptions options = yacu_process_args(5, argv);
    YacuStatus returnCode = yacu_execute(options, suites4Others);
    YACU_ASSERT_EQ_INT(testRun, returnCode, OK);
}

void test_run_single_suite(YacuTestRun *testRun)
{
    const char *argv[] = {"./tests", "--suite", "ForOthers", "--no-fork"};
    YacuOptions options = yacu_process_args(4, argv);
    YacuStatus returnCode = yacu_execute(options, suites4Others);
    YACU_ASSERT_EQ_INT(testRun, returnCode, OK);
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

void test_wrong_args(YacuTestRun *testRun)
{
    YacuProcessHandle pid = yacu_fork();
    if (is_forked(pid))
    {
        const char *argv[] = {"./tests", "--wrong-args"};
        YacuOptions options = yacu_process_args(2, argv);
        UNUSED(options);
    }
    else
    {
        YacuStatus returnCode = wait_for_forked(pid);
        YACU_ASSERT_EQ_INT(testRun, returnCode, WRONG_ARGS);
    }
}

void test_missing_test_args(YacuTestRun *testRun)
{
    YacuProcessHandle pid = yacu_fork();
    if (is_forked(pid))
    {
        const char *argv[] = {"./tests", "--test"};
        YacuOptions options = yacu_process_args(2, argv);
        UNUSED(options);
    }
    else
    {
        YacuStatus returnCode = wait_for_forked(pid);
        YACU_ASSERT_EQ_INT(testRun, returnCode, WRONG_ARGS);
    }
}

void test_missing_junit_args(YacuTestRun *testRun)
{
    YacuProcessHandle pid = yacu_fork();
    if (is_forked(pid))
    {
        const char *argv[] = {"./tests", "--junit"};
        YacuOptions options = yacu_process_args(2, argv);
        UNUSED(options);
    }
    else
    {
        YacuStatus returnCode = wait_for_forked(pid);
        YACU_ASSERT_EQ_INT(testRun, returnCode, WRONG_ARGS);
    }
}

void test_junit_creation_fail(YacuTestRun *testRun)
{
    YacuProcessHandle pid = yacu_fork();
    if (is_forked(pid))
    {
        const char *argv[] = {"./tests", "--junit", "nonexistingdir/report.xml"};
        YacuOptions options = yacu_process_args(3, argv);
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
    const char *argv[] = {"./tests", "--junit", "success.xml", "--no-fork"};
    YacuOptions options = yacu_process_args(3, argv);
    YacuStatus returnCode = yacu_execute(options, suites4Others);
    YACU_ASSERT_EQ_INT(testRun, returnCode, OK);
}

void test_run_single_suite_with_fork(YacuTestRun *testRun)
{
    const char *argv[] = {"./tests", "--suite", "ForOthers"};
    YacuOptions options = yacu_process_args(3, argv);
    YacuStatus returnCode = yacu_execute(options, suites4Others);
    YACU_ASSERT_EQ_INT(testRun, returnCode, OK);
}

YacuTest otherTests[] = {
    {"ListSuitesTest", &test_list_suites},
    {"HelpTest", &test_help},
    {"SingleTestTest", &test_run_single_test},
    {"SingleSuiteTest", &test_run_single_suite},
    {"SingleSuiteTestWithFork", &test_run_single_suite_with_fork},
    {"ForkTest", &test_fork},
    {"WrongArgsTest", &test_wrong_args},
    {"MissingTestArgs", &test_missing_test_args},
    {"MissingJUnitArgs", &test_missing_junit_args},
    {"JUnitCreationFailTest", &test_junit_creation_fail},
    {"JUnitCreationTest", &test_junit_creation},
    END_OF_TESTS};
