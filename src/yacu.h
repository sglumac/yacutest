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

typedef enum YacuAction
{
    HELP = 0,
    LIST = 1,
    RUN_TESTS = 2
} YacuAction;

typedef struct YacuOptions
{
    YacuAction action;
    const char *suiteName;
    const char *testName;
    bool fork;
    const char *jUnitPath;
} YacuOptions;

YacuOptions default_options();

typedef enum YacuReturnCode
{
    OK = 0,
    WRONG_ARGS = 1,
    TEST_FAILURE = 2,
    FORK_FAIL = 3,
    FILE_FAIL = 4,
    FATAL = 5,
    TEST_ERROR = 6
} YacuReturnCode;

#ifndef YACU_JUNIT_MAX_SIZE
#define YACU_JUNIT_MAX_SIZE 1000000
#endif

#ifndef YACU_TEST_RUN_MESSAGE_MAX_SIZE
#define YACU_TEST_RUN_MESSAGE_MAX_SIZE 100000
#endif

typedef struct YacuTestRun
{
    YacuReturnCode result;
    char message[YACU_TEST_RUN_MESSAGE_MAX_SIZE];

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
} YacuSuite;

#define END_OF_SUITES \
    {                 \
        NULL, NULL    \
    }

YacuOptions yacu_process_args(int argc, char const *argv[]);

YacuReturnCode yacu_execute(YacuOptions options, const YacuSuite *suites);

void test_run_message_append(YacuTestRun *testRun, const char *format, ...);

#define YACU_ASSERT_CMP(testRun, afmt, bfmt, a, cmp, b)                                              \
    {                                                                                                \
        if (!(a cmp b))                                                                              \
        {                                                                                            \
            test_run_message_append(testRun,                                                         \
                                    "Condition %s %s %s (" afmt " %s " bfmt ") failed at (%s:%d)\n", \
                                    #a, #cmp, #b, a, #cmp, b, __FILE__, __LINE__);                   \
            testRun->result = TEST_FAILURE;                                                          \
            exit(TEST_FAILURE);                                                                      \
        }                                                                                            \
    }

#define YACU_ASSERT_CMP_INT(testRun, a, cmp, b) YACU_ASSERT_CMP(testRun, "%d", "%d", a, cmp, b)

#define YACU_ASSERT_LT_INT(testRun, a, b) YACU_ASSERT_CMP_INT(testRun, a, <, b)
#define YACU_ASSERT_LE_INT(testRun, a, b) YACU_ASSERT_CMP_INT(testRun, a, <=, b)
#define YACU_ASSERT_EQ_INT(testRun, a, b) YACU_ASSERT_CMP_INT(testRun, a, ==, b)
#define YACU_ASSERT_GT_INT(testRun, a, b) YACU_ASSERT_CMP_INT(testRun, a, >, b)
#define YACU_ASSERT_GE_INT(testRun, a, b) YACU_ASSERT_CMP_INT(testRun, a, >=, b)

#define YACU_ASSERT_CMP_UINT(testRun, a, cmp, b) YACU_ASSERT_CMP(testRun, "%u", "%u", a, cmp, b)

#define YACU_ASSERT_LT_UINT(testRun, a, b) YACU_ASSERT_CMP_UINT(testRun, a, <, b)
#define YACU_ASSERT_LE_UINT(testRun, a, b) YACU_ASSERT_CMP_UINT(testRun, a, <=, b)
#define YACU_ASSERT_EQ_UINT(testRun, a, b) YACU_ASSERT_CMP_UINT(testRun, a, ==, b)
#define YACU_ASSERT_GT_UINT(testRun, a, b) YACU_ASSERT_CMP_UINT(testRun, a, >, b)
#define YACU_ASSERT_GE_UINT(testRun, a, b) YACU_ASSERT_CMP_UINT(testRun, a, >=, b)

#define YACU_ABS(x) (x > 0 ? x : -x)

#define YACU_ASSERT_APPROX_EQ(testRun, afmt, bfmt, tolfmt, a, b, tol)                                                    \
    {                                                                                                                    \
        if (!(YACU_ABS(a - b) < tol))                                                                                    \
        {                                                                                                                \
            test_run_message_append(testRun,                                                                             \
                                    "Condition |%s - %s| < %s (|" afmt " - " bfmt "| < " tolfmt ") failed at (%s:%d)\n", \
                                    #a, #b, #tol, a, b, tol, __FILE__, __LINE__);                                        \
            testRun->result = TEST_FAILURE;                                                                              \
            exit(TEST_FAILURE);                                                                                          \
        }                                                                                                                \
    }

#define YACU_ASSERT_APPROX_EQ_DBL(testRun, a, b, tol) YACU_ASSERT_APPROX_EQ(testRun, "%lf", "%lf", "%lf", a, b, tol)

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

YacuReturnCode wait_for_forked(YacuProcessHandle forkedId);

#endif // YACU_H
