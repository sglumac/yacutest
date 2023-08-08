#include <common.h>
#include <stdio.h>
#define UNUSED(x) (void)(x)

typedef struct FileReport
{
    const char *filePath;
} FileReport;

static void file_report_action(YacuReportState state, YacuReportEvent reportEvent, const struct YacuSuite *suite, const struct YacuTestRun *testRun)
{
    UNUSED(suite);
    FileReport *fileReport = state;

    switch (reportEvent)
    {
    case TEST_RUN_FINISHED:
    {
        FILE *reportFile = fopen(fileReport->filePath, "w");
        fputs(testRun->message, reportFile);
        fflush(reportFile);
        fclose(reportFile);
        break;
    }
    default:
        break;
    }
}

YacuStatus forked_test(YacuTestRun *testRun, const char *reportPath, ForkedAction forkedAction, char *failureMessage)
{
    UNUSED(testRun);
    FileReport fileReportState = {.filePath = reportPath};
    YacuReport fileReport = {.state = &fileReportState, .action = file_report_action};
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
