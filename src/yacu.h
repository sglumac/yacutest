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

#ifndef YACU_H
#define YACU_H

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

typedef enum YacuAction
{
    HELP = 0,
    LIST = 1,
    RUN_TESTS = 2
} YacuAction;

typedef enum YacuStatus
{
    OK = 0,
    TEST_FAILURE = 1,
    WRONG_ARGS = 2,
    FORK_FAIL = 3,
    FILE_FAIL = 4,
    TEST_ERROR = 5,
    FATAL = 99,
} YacuStatus;

typedef void *YacuReportState;
typedef void (*YacuReportOnSuitesStarted)(YacuReportState state);
typedef void (*YacuReportOnSuiteStarted)(YacuReportState state, const char *suiteName);
typedef void (*YacuReportOnTestStarted)(YacuReportState state, const char *testName);
typedef void (*YacuReportOnTestFinished)(YacuReportState state, YacuStatus result, const char *message);
typedef void (*YacuReportOnSuiteFinished)(YacuReportState state);
typedef void (*YacuReportOnSuitesFinished)(YacuReportState state);

typedef struct YacuReport
{
    YacuReportState state;
    YacuReportOnSuitesStarted on_suites_started;
    YacuReportOnSuiteStarted on_suite_started;
    YacuReportOnTestStarted on_test_started;
    YacuReportOnTestFinished on_test_finished;
    YacuReportOnSuiteFinished on_suite_finished;
    YacuReportOnSuitesFinished on_suites_finished;
} YacuReport;

typedef YacuReport *YacuReportPtr;

extern YacuReport END_OF_REPORTS;

typedef struct YacuOptions
{
    YacuAction action;
    const char *suiteName;
    const char *testName;
    bool fork;
    const char *jUnitPath;
    YacuReport *customReport;
} YacuOptions;

YacuOptions default_options();

#ifndef YACU_JUNIT_MAX_SIZE
#define YACU_JUNIT_MAX_SIZE 1000000
#endif

#ifndef YACU_TEST_RUN_MESSAGE_MAX_SIZE
#define YACU_TEST_RUN_MESSAGE_MAX_SIZE 100000
#endif

typedef struct YacuTestRun
{
    YacuStatus result;
    char message[YACU_TEST_RUN_MESSAGE_MAX_SIZE];
    bool forked;
    YacuReportPtr *reports;
} YacuTestRun;

typedef void (*YacuTestFcn)(YacuTestRun *testRun);

typedef struct YacuTest
{
    const char *name;
    YacuTestFcn fcn;
} YacuTest;

#define END_OF_TESTS \
    {                \
        NULL, NULL   \
    }

typedef struct YacuSuite
{
    const char *name;
    const YacuTest *tests;
    YacuOptions options;
} YacuSuite;

#define END_OF_SUITES \
    {                 \
        NULL, NULL    \
    }

YacuOptions yacu_process_args(int argc, char const *argv[]);

YacuStatus yacu_execute(YacuOptions options, const YacuSuite *suites);

void test_run_message_append(YacuTestRun *testRun, const char *format, ...);

void yacu_assert(YacuTestRun *testRun, bool condition, const char *fmt, ...);

#define YACU_ASSERT(testRun, condition, label, fmt, ...) \
    yacu_assert(testRun, condition, "%s:%d - Assertion %s (" fmt ") failed!", __FILE__, __LINE__, label, __VA_ARGS__)

#define YACU_ASSERT_EQ_STR(testRun, left, right) \
    YACU_ASSERT(testRun, strcmp(left, right) == 0, #left " == " #right, "\"%s\" == \"%s\"", left, right)

#define YACU_ASSERT_IN_STR(testRun, left, right) \
    YACU_ASSERT(testRun, strstr(right, left) != NULL, #left " IN " #right, "\"%s\" IN \"%s\"", left, right)

#define YACU_ASSERT_CMP(testRun, leftfmt, rightfmt, left, cmp, right) \
    YACU_ASSERT(testRun, left cmp right, #left " " #cmp " " #right, leftfmt " " #cmp " " rightfmt, left, right)

#define YACU_ASSERT_CMP_INT(testRun, left, cmp, right) YACU_ASSERT_CMP(testRun, "%d", "%d", left, cmp, right)

#define YACU_ASSERT_LT_INT(testRun, left, right) YACU_ASSERT_CMP_INT(testRun, left, <, right)
#define YACU_ASSERT_LE_INT(testRun, left, right) YACU_ASSERT_CMP_INT(testRun, left, <=, right)
#define YACU_ASSERT_EQ_INT(testRun, left, right) YACU_ASSERT_CMP_INT(testRun, left, ==, right)
#define YACU_ASSERT_GT_INT(testRun, left, right) YACU_ASSERT_CMP_INT(testRun, left, >, right)
#define YACU_ASSERT_GE_INT(testRun, left, right) YACU_ASSERT_CMP_INT(testRun, left, >=, right)

#define YACU_ASSERT_CMP_UINT(testRun, left, cmp, right) YACU_ASSERT_CMP(testRun, "%u", "%u", left, cmp, right)

#define YACU_ASSERT_LT_UINT(testRun, left, right) YACU_ASSERT_CMP_UINT(testRun, left, <, right)
#define YACU_ASSERT_LE_UINT(testRun, left, right) YACU_ASSERT_CMP_UINT(testRun, left, <=, right)
#define YACU_ASSERT_EQ_UINT(testRun, left, right) YACU_ASSERT_CMP_UINT(testRun, left, ==, right)
#define YACU_ASSERT_GT_UINT(testRun, left, right) YACU_ASSERT_CMP_UINT(testRun, left, >, right)
#define YACU_ASSERT_GE_UINT(testRun, left, right) YACU_ASSERT_CMP_UINT(testRun, left, >=, right)

#define YACU_ABS(x) (x > 0 ? x : -x)

#define YACU_ASSERT_APPROX_EQ(testRun, leftfmt, rightfmt, tolfmt, a, b, tol) \

#define YACU_ASSERT_APPROX_EQ_DBL(testRun, left, b, tol) YACU_ASSERT_APPROX_EQ(testRun, "%lf", "%lf", "%lf", a, b, tol)

#if defined(__unix__) || defined(UNIX) || defined(__linux__) || defined(LINUX)
#define FORK_AVAILABLE
typedef pid_t YacuProcessHandle;
#include <unistd.h>
#include <sys/wait.h>
#else
typedef int YacuProcessHandle;
#endif

YacuProcessHandle yacu_fork();

bool is_forked(YacuProcessHandle pid);

YacuStatus wait_for_forked(YacuProcessHandle forkedId);

#endif // YACU_H
