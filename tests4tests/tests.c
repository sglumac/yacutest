#include <yacu.h>
#include <stddef.h>

void test_assert_cmp_int(YacuTestRun testRun)
{
    int small = -1;

    yacu_assert_int_lt(testRun, small, 0);
    yacu_assert_int_le(testRun, small, -1);
    yacu_assert_int_eq(testRun, small, -1);
    yacu_assert_int_ge(testRun, small, -1);
    yacu_assert_int_gt(testRun, small, -2);
}

void test_assert_cmp_uint(YacuTestRun testRun)
{
    unsigned int small = 1;

    yacu_assert_int_lt(testRun, small, 2);
    yacu_assert_int_le(testRun, small, 1);
    yacu_assert_int_eq(testRun, small, 1);
    yacu_assert_int_ge(testRun, small, 1);
    yacu_assert_int_gt(testRun, small, 0);
}

YacuTest assertionTests[] = {
    {"cmpIntTest", &test_assert_cmp_int},
    {"cmpUintTest", &test_assert_cmp_uint},
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

void test_wrong_args(YacuTestRun testRun)
{
    YacuProcessHandle pid = yacu_fork();
    if (is_forked(pid))
    {
        const char *argv[] = {"./tests", "--wrong-args"};
        YacuOptions options = yacu_process_args(2, argv);
    }
    else
    {
        YacuExitCode returnCode = wait_for_forked(pid);
        yacu_assert_int_eq(testRun, returnCode, WRONG_ARGS);
    }
}

void test_missing_test_args(YacuTestRun testRun)
{
    YacuProcessHandle pid = yacu_fork();
    if (is_forked(pid))
    {
        const char *argv[] = {"./tests", "--test"};
        YacuOptions options = yacu_process_args(2, argv);
    }
    else
    {
        YacuExitCode returnCode = wait_for_forked(pid);
        yacu_assert_int_eq(testRun, returnCode, WRONG_ARGS);
    }
}

void test_missing_report_args(YacuTestRun testRun)
{
    YacuProcessHandle pid = yacu_fork();
    if (is_forked(pid))
    {
        const char *argv[] = {"./tests", "--report"};
        YacuOptions options = yacu_process_args(2, argv);
    }
    else
    {
        YacuExitCode returnCode = wait_for_forked(pid);
        yacu_assert_int_eq(testRun, returnCode, WRONG_ARGS);
    }
}

void test_report_creation_fail(YacuTestRun testRun)
{
    YacuProcessHandle pid = yacu_fork();
    if (is_forked(pid))
    {
        const char *argv[] = {"./tests", "--report", "nonexistingdir/report.log"};
        YacuOptions options = yacu_process_args(3, argv);
        yacu_execute(options, suites4Others);
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
    {"ListSuitesTest", &test_list_suites},
    {"HelpTest", &test_help},
    {"SingleTestTest", &test_run_single_test},
    {"SingleSuiteTest", &test_run_single_suite},
    {"SingleSuiteTestWithFork", &test_run_single_suite_with_fork},
    {"ForkTest", &test_fork},
    {"WrongArgsTest", &test_wrong_args},
    {"MissingTestArgs", &test_missing_test_args},
    {"MissingReportArgs", &test_missing_report_args},
    {"MissingReportArgs", &test_missing_report_args},
    {"ReportCreationTest", &test_report_creation_fail},
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
