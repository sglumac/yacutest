#include <common.h>
#include <stdio.h>
#define UNUSED(x) (void)(x)

typedef struct FileReport
{
    const char *filePath;
} FileReport;

YacuStatus forked_test(YacuTestRun *testRun, const char *reportPath, ForkedAction forkedAction, char *failureMessage)
{
    UNUSED(testRun);
    YacuTestRun forkedTestRun;
    YacuProcessHandle pid = yacu_fork();
    if (is_forked(pid))
    {
        forkedAction(&forkedTestRun);
    }
    YacuStatus forkReturnCode = wait_for_forked(pid);
    // FILE *reportFile = fopen(reportPath, "r");
    // fgets(failureMessage, YACU_TEST_RUN_MESSAGE_MAX_SIZE, reportFile);
    UNUSED(failureMessage);
    UNUSED(reportPath);
    // fclose(reportFile);
    return forkReturnCode;
}
