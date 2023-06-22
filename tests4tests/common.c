#include <common.h>
#include <stdio.h>
#define UNUSED(x) (void)(x)

typedef struct FileReport
{
    const char *filePath;
} FileReport;

static void file_report_on_suites_started(YacuReportState state)
{
    FileReport *fileReport = state;
    fclose(fopen(fileReport->filePath, "w"));
}

static void file_report_on_test_finished(YacuReportState state, YacuStatus result)
{
    UNUSED(state);
    UNUSED(result);
}

static void file_report_on_test_error(YacuReportState state, const char *message)
{
    FileReport *fileReport = state;

    FILE *reportFile = fopen(fileReport->filePath, "a");
    fprintf(reportFile, "%s", message);
    fflush(reportFile);
    fclose(reportFile);
}

static void file_report_on_suite_finished(YacuReportState state) { UNUSED(state); }

static void file_report_on_suites_finished(YacuReportState state) { UNUSED(state); }

YacuStatus forked_test(YacuTestRun *testRun, const char *reportPath, ForkedAction forkedAction, char *failureMessage)
{
    UNUSED(testRun);
    FileReport fileReportState = {.filePath = reportPath};
    YacuReport fileReport = {
        .state = &fileReportState,
        .on_suites_started = file_report_on_suites_started,
        .on_test_error = file_report_on_test_error,
        .on_test_finished = file_report_on_test_finished,
        .on_suite_finished = file_report_on_suite_finished,
        .on_suites_finished = file_report_on_suites_finished};
    YacuReportPtr reports[] = {&fileReport, &END_OF_REPORTS};
    YacuTestRun forkedTestRun = {.message = "", .forked = false, .reports = reports};
    file_report_on_suites_started(&fileReportState);
    YacuProcessHandle pid = yacu_fork();
    if (is_forked(pid))
    {
        forkedAction(&forkedTestRun);
    }
    YacuStatus forkReturnCode = wait_for_forked(pid);
    FILE *reportFile = fopen(reportPath, "r");
    fgets(failureMessage, YACU_TEST_RUN_MESSAGE_MAX_SIZE, reportFile);
    fclose(reportFile);
    return forkReturnCode;
}
