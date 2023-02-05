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

static void main_action(YacuTestRun *testRun, YacuStatus forkReturnCode, const char *forkMessage, const char* expectedMessage)
{
    YACU_ASSERT_EQ_INT(testRun, forkReturnCode, TEST_FAILURE);
    YACU_ASSERT_IN_STR(testRun, expectedMessage, forkMessage);
}

void test_using_fork(YacuTestRun *testRun, const char *reportPath, const char* expectedMessage, ForkedAction forkedAction)
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
    main_action(testRun, forkReturnCode, forkMessage, expectedMessage);
}

void forked_assert_failed_cmp_int(YacuTestRun* forkedTestRun)
{
    int small = -1;

    YACU_ASSERT_LT_INT(forkedTestRun, small, -2);
}

void test_assert_failed_cmp_int(YacuTestRun *testRun)
{
    const char reportPath[] = "failedCmpIntTest.log";
    test_using_fork(testRun, reportPath, "failures.c:70 - Assertion small < -2 (-1 < -2) failed!", forked_assert_failed_cmp_int);
}

YacuTest assertionFailuresTests[] = {
    {"failedCmpIntTest", &test_assert_failed_cmp_int},
};
