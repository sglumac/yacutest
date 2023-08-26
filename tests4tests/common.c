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

YacuProcessHandle yacu_fork()
{
    fflush(stdout);
    fflush(stderr);
#ifdef FORK_AVAILABLE
    pid_t pid = fork();
    if (pid < 0)
    {
        exit(FORK_FAIL);
    }
    return pid;
#else
    return -1;
#endif
}

bool is_forked(YacuProcessHandle pid)
{
#ifdef FORK_AVAILABLE
    return pid == 0;
#else
    return false;
#endif
}

YacuStatus wait_for_forked(YacuProcessHandle forkedId)
{
#ifdef FORK_AVAILABLE
    int status;
    waitpid(forkedId, &status, 0);
    if (WIFEXITED(status))
    {
        YacuStatus testStatus = WEXITSTATUS(status);
        printf("exited, status=%d\n", testStatus);
        return testStatus;
    }
    else if (WIFSIGNALED(status))
    {
        printf("killed by signal %d, status=%d\n", WTERMSIG(status), WEXITSTATUS(status));
        return TEST_ERROR;
    }
    else if (WIFSTOPPED(status))
    {
        printf("stopped by signal %d\n", WSTOPSIG(status));
        return TEST_ERROR;
    }
    else if (WIFCONTINUED(status))
    {
        printf("continued\n");
        return TEST_ERROR;
    }
    else
    {
        return TEST_ERROR;
    }
#else
    return OK;
#endif
}

YacuStatus forked_test(YacuTestRun *testRun, const char *reportPath, ForkedAction forkedAction, char *failureMessage)
{
    UNUSED(testRun);
    FileReport fileReportState = {.filePath = reportPath};
    YacuReport fileReport = {.state = &fileReportState, .action = file_report_action};
    YacuReportPtr reports[] = {&fileReport, &END_OF_REPORTS};
    YacuTestRun forkedTestRun = {.result = OK, .message = "", .reports = reports};
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
