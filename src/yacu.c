/****************************************************************************
Yet Another C Unit (YACU) testing framework

MIT License

Copyright (c) 2023 Slaven Glumac

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************/
#include <yacu.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(x) (void)(x)

static bool end_of_suites(const YacuSuite suite)
{
    return suite.name == NULL;
}

static bool end_of_tests(const YacuTest test)
{
    return test.name == NULL;
}

YacuOptions yacu_default_options()
{
    YacuOptions options = {
        .suiteName = NULL,
        .testName = NULL};
    return options;
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
        return TEST_FAILURE;
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

static void yacu_basic_log(void *logData, const char *message)
{
    UNUSED(logData);
    printf("%s\n", message);
}

static void yacu_run_test(YacuTest test, const void *runData)
{
    YacuTestRun testRun = {.log = yacu_basic_log, .runData = runData};
    test.fcn(&testRun);
}

void yacu_assert(YacuTestRun *testRun, bool condition, const char *fmt, ...)
{
    if (!(condition))
    {
        va_list args;
        va_start(args, fmt);
        char message[YACU_TEST_RUN_MESSAGE_MAX_SIZE];
        vsnprintf(message, YACU_TEST_RUN_MESSAGE_MAX_SIZE, fmt, args);
        va_end(args);
        testRun->log(testRun->logData, message);
        exit(TEST_FAILURE);
    }
}

void yacu_execute(YacuOptions options, const YacuSuite *suites)
{
    for (const YacuSuite *suiteIt = suites; !end_of_suites(*suiteIt); suiteIt++)
    {
        if (options.suiteName == NULL || strcmp(options.suiteName, suiteIt->name) == 0)
        {
            for (const YacuTest *testIt = suiteIt->tests; !end_of_tests(*testIt); testIt++)
            {
                if (options.testName == NULL || strcmp(options.testName, testIt->name) == 0)
                {
                    yacu_run_test(*testIt, options.runData);
                }
            }
        }
    }
}
