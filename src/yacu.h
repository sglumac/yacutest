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

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

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

typedef enum YacuReportEvent
{
    SUITE_STARTED = 31,
    TEST_RUN_STARTED = 32,
    TEST_RUN_FINISHED = 33,
    SUITE_FINISHED = 34,
    TESTING_FINISHED = 35,
} YacuReportEvent;

struct YacuTestRun;

struct YacuSuite;

typedef void *YacuReportState;
typedef void (*YacuReportAction)(YacuReportState state, YacuReportEvent reportEvent, const struct YacuSuite *suite, const struct YacuTestRun *testRun);

typedef struct YacuReport
{
    YacuReportState state;
    YacuReportAction action;
} YacuReport;

typedef YacuReport *YacuReportPtr;

extern YacuReport END_OF_REPORTS;

typedef struct YacuOptions
{
    const char *suiteName;
    const char *testName;
    const char *jUnitPath;
    bool stdoutReport;
    YacuReport *customReport;
    const void *runData;
} YacuOptions;

YacuOptions yacu_default_options();

struct YacuTestRun;

typedef void (*YacuTestFcn)(struct YacuTestRun *testRun);

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
} YacuSuite;

#define END_OF_SUITES \
    {                 \
        NULL, NULL    \
    }

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
    YacuReportPtr *reports;
    const void *runData;
    const YacuSuite *suite;
    const YacuTest *test;
} YacuTestRun;

void yacu_apply_cmd_args(YacuOptions *options, int argc, char const *argv[]);

YacuStatus yacu_execute(YacuOptions options, const YacuSuite *suites);

void test_run_message_append(YacuTestRun *testRun, const char *format, ...);

void yacu_assert(YacuTestRun *testRun, bool condition, const char *fmt, ...);

#define YACU_ASSERT(testRun, condition, label, fmt, ...) \
    yacu_assert(testRun, condition, "%s:%d - Assertion %s (" fmt ") failed!", __FILE__, __LINE__, label, __VA_ARGS__)

#define YACU_ASSERT_TRUE(testRun, condition) \
    YACU_ASSERT(testRun, condition, #condition, "%d", condition)

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

#define YACU_ASSERT_EQ_CHAR(testRun, left, right) YACU_ASSERT_CMP(testRun, "%c", "%c", left, ==, right)

#define YACU_ABS(x) ((x) > 0 ? (x) : -(x))

#define YACU_ASSERT_APPROX_EQ(testRun, leftfmt, rightfmt, tolfmt, left, right, tol) \
    YACU_ASSERT(testRun,                                                            \
                YACU_ABS((left) - (right)) < (tol),                                 \
                "|" #left " - " #right "| < " #tol,                                 \
                "|" leftfmt " - " rightfmt "| < " tolfmt,                           \
                left, right, tol)

#define YACU_ASSERT_APPROX_EQ_DBL(testRun, left, right, tol) \
    YACU_ASSERT_APPROX_EQ(testRun, "%lf", "%lf", "%lf", left, right, tol)

#if defined(__unix__) || defined(UNIX) || defined(__linux__) || defined(LINUX)
#define FORK_AVAILABLE
typedef pid_t YacuProcessHandle;
#include <unistd.h>
#include <sys/wait.h>
#else
typedef int YacuProcessHandle;
#endif

#endif // YACU_H
