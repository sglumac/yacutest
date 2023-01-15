#include <yacu.h>
#include <stddef.h>

void test_assert_cmp_int(YacuTestRun testRun)
{
    int small = 1;

    yacu_assert_int_lt(testRun, small, 2);
    yacu_assert_int_le(testRun, small, 1);
    yacu_assert_int_eq(testRun, small, 1);
    yacu_assert_int_ge(testRun, small, 1);
    yacu_assert_int_gt(testRun, small, 0);
}

YacuTest assertionTests[] = {
    {"cmpIntTest", &test_assert_cmp_int},
    END_OF_TESTS};

YacuSuite suites4Others[] = {
    {"Assertions", assertionTests},
    END_OF_SUITES};

void test_list_suites(YacuTestRun testRun)
{
    const char *argv[] = {"./tests", "--list", "--no-fork", "--report", "test_list_suites.log"};
    YacuOptions options = yacu_process_args(5, argv);
    int returnCode = yacu_execute(options, suites4Others);
    yacu_assert_int_eq(testRun, returnCode, 0);
}

void test_help(YacuTestRun testRun)
{
    const char *argv[] = {"./tests", "--help", "--report", "test_help.log"};
    YacuOptions options = yacu_process_args(4, argv);
    int returnCode = yacu_execute(options, suites4Others);
    yacu_assert_int_eq(testRun, returnCode, 0);
}

void test_run_single_test(YacuTestRun testRun)
{
    const char *argv[] = {"./tests", "--test", "Assertions", "cmpIntTest", "--no-fork", "--report", "test_run_single_test.log"};
    YacuOptions options = yacu_process_args(7, argv);
    int returnCode = yacu_execute(options, suites4Others);
    yacu_assert_int_eq(testRun, returnCode, 0);
}

void test_run_single_suite(YacuTestRun testRun)
{
    const char *argv[] = {"./tests", "--suite", "Assertions", "--no-fork", "--report", "test_run_single_suite.log"};
    YacuOptions options = yacu_process_args(6, argv);
    int returnCode = yacu_execute(options, suites4Others);
    yacu_assert_int_eq(testRun, returnCode, OK);
}

void test_fork(YacuTestRun testRun)
{
    YacuProcessHandle pid = yacu_fork();
    yacu_report(testRun, "yacu_fork() = %d\n", pid);
    if (is_forked(pid))
    {
        exit(FILE_FAIL);
    }
    else
    {
        YacuExitCode returnCode = wait_for_forked(pid);
        yacu_assert_int_eq(testRun, returnCode, FILE_FAIL);
    }
}

void test_run_single_suite_with_fork(YacuTestRun testRun)
{
    const char *argv[] = {"./tests", "--suite", "Assertions", "--report", "test_run_single_suite_with_fork.log"};
    YacuOptions options = yacu_process_args(5, argv);
    int returnCode = yacu_execute(options, suites4Others);
    yacu_assert_int_eq(testRun, returnCode, OK);
}

YacuTest otherTests[] = {
    {"listSuitesTest", &test_list_suites},
    {"helpTest", &test_help},
    {"singleTestTest", &test_run_single_test},
    {"singleSuiteTest", &test_run_single_suite},
    {"singleSuiteTestWithFork", &test_run_single_suite_with_fork},
    {"forkTest", &test_fork},
    END_OF_TESTS};

YacuSuite suites[] = {
    {"Assertions", assertionTests},
    {"Others", otherTests},
    END_OF_SUITES};

int main(int argc, char const *argv[])
{
    YacuOptions options = yacu_process_args(argc, argv);
    return yacu_execute(options, suites);
}
