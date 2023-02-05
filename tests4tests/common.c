#include <common.h>

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

void forked_test(YacuTestRun *testRun, const char *reportPath, ForkedAction forkedAction, char* failureMessage)
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
    return forkReturnCode;
}
