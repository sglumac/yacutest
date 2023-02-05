#include <yacu.h>
#include <failures.h>
#include <stdlib.h>

typedef struct FileReport
{
    const char *filePath;
} FileReport;

static void file_report_on_test_finished(YacuReportState state, YacuStatus result, const char *message)
{
    FileReport *fileReport = state;

    FILE *reportFile = fopen(fileReport->filePath, "w");
    fputs(message, reportFile);
    fflush(reportFile);
    fclose(reportFile);
}

static void file_report_on_suite_finished(YacuReportState state) {}

static void file_report_on_suites_finished(YacuReportState state) {}

typedef void ForkedAction(YacuTestRun *forkedTestRun);

void test_using_fork(YacuTestRun *testRun, const char *reportPath, ForkedAction forkedAction, char* failureMessage)
{
    FileReport fileReportState = {.filePath = reportPath};
    YacuReport fileReport = {
        .state = &fileReportState,
        .on_test_finished = file_report_on_test_finished,
        .on_suite_finished = file_report_on_suite_finished,
        .on_suites_finished = file_report_on_suites_finished};
    YacuReportPtr reports[] = {&fileReport, &END_OF_REPORTS};
    YacuTestRun forkedTestRun = {.result = OK, .message = "", .forked = false, .reports = reports};
    YacuProcessHandle pid = yacu_fork();
    if (is_forked(pid))
    {
        forkedAction(&forkedTestRun);
    }
    YacuStatus forkReturnCode = wait_for_forked(pid);
    FILE *reportFile = fopen(reportPath, "r");
    fgets(failureMessage, YACU_TEST_RUN_MESSAGE_MAX_SIZE, reportFile);
    fclose(reportFile);
    YACU_ASSERT_EQ_INT(testRun, forkReturnCode, TEST_FAILURE);
}

void forked_assert_failed_cmp_int(YacuTestRun *forkedTestRun)
{
    int small = -1;

    YACU_ASSERT_LT_INT(forkedTestRun, small, -2);
}

void test_assert_failed_cmp_int(YacuTestRun *testRun)
{
    const char reportPath[] = "failedCmpIntTest.log";
    char failureMessage[YACU_TEST_RUN_MESSAGE_MAX_SIZE];
    test_using_fork(testRun, reportPath, forked_assert_failed_cmp_int, failureMessage);
    YACU_ASSERT_IN_STR(testRun, "failures.c:", failureMessage);
    YACU_ASSERT_IN_STR(testRun, " - Assertion small < -2 (-1 < -2) failed!", failureMessage);
}

YacuTest assertionFailuresTests[] = {
    {"failedCmpIntTest", &test_assert_failed_cmp_int},
};
