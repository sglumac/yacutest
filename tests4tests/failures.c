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

    switch (result)
    {
    case OK:
        break;
    case TEST_ERROR:
        break;
    case TEST_FAILURE:
    {
        FILE *reportFile = fopen(fileReport->filePath, "w");
        fputs(message, reportFile);
        fflush(reportFile);
        fclose(reportFile);
        break;
    }
    }
}

static void file_report_on_suite_finished(YacuReportState state) {}

static void file_report_on_suites_finished(YacuReportState state) {}

typedef void ForkedAction(YacuTestRun *forkedTestRun);
typedef void MainAction(YacuTestRun *testRun, YacuStatus forkReturnCode, const char *forkMessage);

void test_using_fork(YacuTestRun *testRun, const char *reportPath, MainAction mainAction, ForkedAction forkedAction)
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
    char forkMessage[YACU_TEST_RUN_MESSAGE_MAX_SIZE];
    FILE *reportFile = fopen(reportPath, "r");
    fgets(forkMessage, YACU_TEST_RUN_MESSAGE_MAX_SIZE, reportFile);
    fclose(reportFile);
    mainAction(testRun, forkReturnCode, forkMessage);
}

void forked_assert_failed_cmp_int(YacuTestRun* forkedTestRun)
{
    int small = -1;

    YACU_ASSERT_LT_INT(forkedTestRun, small, -2);
}

void main_assert_failed_cmp_int(YacuTestRun *testRun, YacuStatus forkReturnCode, const char *forkMessage)
{
    YACU_ASSERT_EQ_INT(testRun, forkReturnCode, TEST_FAILURE);
    YACU_ASSERT_IN_STR(testRun, "failures.c:65 - Assertion small < -2 (-1 < -2) failed!", forkMessage);
}

void test_assert_failed_cmp_int(YacuTestRun *testRun)
{
    const char reportPath[] = "failedCmpIntTest.log";
    test_using_fork(testRun, reportPath, main_assert_failed_cmp_int, forked_assert_failed_cmp_int);
}

YacuTest assertionFailuresTests[] = {
    {"failedCmpIntTest", &test_assert_failed_cmp_int},
};
