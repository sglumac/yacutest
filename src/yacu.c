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

static bool end_of_suites(const YacuSuite suite)
{
    return suite.name == NULL;
}

static bool end_of_tests(const YacuTest test)
{
    return test.name == NULL;
}

static bool end_of_reports(YacuReport *report)
{
    return report == NULL ? false : report->action == NULL && report->state == NULL;
}

YacuOptions yacu_default_options()
{
    YacuOptions options = {
        .suiteName = NULL,
        .testName = NULL,
        .jUnitPath = NULL,
        .stdoutReport = true,
        .customReport = NULL};
    return options;
}

static void vbuffer_append(char *buffer, size_t bufferMaxSize, const char *format, va_list args)
{
    size_t bufferLength = strlen(buffer);
    vsnprintf(buffer + bufferLength, bufferMaxSize - bufferLength, format, args);
}

static void buffer_append(char *buffer, size_t bufferMaxSize, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vbuffer_append(buffer, bufferMaxSize, format, args);
    va_end(args);
}

static void process_test_or_suite_arg(int i, int argc, char const *argv[], YacuOptions *options, bool withTest)
{
    if (argc <= i + (withTest ? 2 : 1))
    {
        exit(WRONG_ARGS);
    }
    options->suiteName = argv[i + 1];
    if (withTest)
    {
        options->testName = argv[i + 2];
    }
}

YacuOptions yacu_process_args(int argc, char const *argv[])
{
    YacuOptions options = yacu_default_options();
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--test") == 0)
        {
            process_test_or_suite_arg(i, argc, argv, &options, true);
            i = i + 2;
        }
        else if (strcmp(argv[i], "--suite") == 0)
        {
            process_test_or_suite_arg(i, argc, argv, &options, false);
            i++;
        }
        else if (strcmp(argv[i], "--junit") == 0)
        {
            if (argc <= i + 1)
            {
                exit(WRONG_ARGS);
            }
            options.jUnitPath = argv[i + 1];
            i++;
        }
        else
        {
            exit(WRONG_ARGS);
        }
    }
    return options;
}

typedef struct JUnitReport
{
    char jUnitBuffer[YACU_TEST_RUN_MESSAGE_MAX_SIZE];
    const char *jUnitPath;
} JUnitReport;

#define UNUSED(x) (void)(x)

static void junit_report_action(YacuReportState state, YacuReportEvent reportEvent, const YacuSuite *suite, const YacuTestRun *testRun)
{
    JUnitReport *current = (JUnitReport *)state;
    switch (reportEvent)
    {
    case SUITE_STARTED:
        buffer_append(current->jUnitBuffer, YACU_TEST_RUN_MESSAGE_MAX_SIZE,
                      "  <testsuite package=\"\" id=\"0\" name=\"%s\"", suite->name);
        buffer_append(current->jUnitBuffer, YACU_TEST_RUN_MESSAGE_MAX_SIZE,
                      " timestamp=\"1900-12-12T%11:11:11\"");
        buffer_append(current->jUnitBuffer, YACU_TEST_RUN_MESSAGE_MAX_SIZE,
                      " hostname=\"-\" tests=\"4\" failures=\"2\" errors=\"1\" time=\"3\">\n");
        buffer_append(current->jUnitBuffer, YACU_TEST_RUN_MESSAGE_MAX_SIZE,
                      "    <properties/>\n");
        break;
    case TEST_RUN_STARTED:
        buffer_append(current->jUnitBuffer, YACU_TEST_RUN_MESSAGE_MAX_SIZE,
                      "    <testcase classname=\"\" name=\"%s\" time=\"0.0\">\n",
                      testRun->test->name);
        break;
    case TEST_RUN_FINISHED:
        buffer_append(current->jUnitBuffer, YACU_TEST_RUN_MESSAGE_MAX_SIZE,
                      "    </testcase>\n");
        break;
    case SUITE_FINISHED:
        buffer_append(current->jUnitBuffer, YACU_TEST_RUN_MESSAGE_MAX_SIZE,
                      "    <system-out/>\n"
                      "    <system-err/>\n"
                      "  </testsuite>\n");
        break;
    case TESTING_FINISHED:
        buffer_append(current->jUnitBuffer, YACU_TEST_RUN_MESSAGE_MAX_SIZE,
                      "</testsuites>\n");
        if (current->jUnitPath != NULL)
        {
            FILE *jUnitFile = fopen(current->jUnitPath, "w");
            if (jUnitFile == NULL)
            {
                exit(FILE_FAIL);
            }
            fprintf(jUnitFile, "%s", current->jUnitBuffer);
            fflush(jUnitFile);
            fclose(jUnitFile);
        }
        break;
    default:
        return;
    }
}

static void stdout_on_test_finished(const YacuTestRun *testRun)
{
    switch (testRun->result)
    {
    case OK:
        printf("    OK\n");
        break;
    case TEST_ERROR:
        printf("    ERROR\n");
        break;
    case TEST_FAILURE:
        printf("    FAILURE\n");
        break;
    case WRONG_ARGS:
        printf("    WRONG_ARGS\n");
        break;
    case FORK_FAIL:
        printf("    FORK_FAIL\n");
        break;
    case FILE_FAIL:
        printf("    FILE_FAIL\n");
        break;
    case FATAL:
        printf("    FATAL\n");
        break;
    }
    if (strlen(testRun->message) > 0)
    {
        printf("    %s\n", testRun->message);
    }
}

static void stdout_report_action(YacuReportState state, YacuReportEvent reportEvent, const YacuSuite *suite, const YacuTestRun *testRun)
{
    UNUSED(state);
    switch (reportEvent)
    {
    case SUITE_STARTED:
        printf("#%s\n", suite->name);
        break;
    case TEST_RUN_STARTED:
        printf("  ##%s\n", testRun->test->name);
        break;
    case TEST_RUN_FINISHED:
        stdout_on_test_finished(testRun);
        break;
    default:
        return;
    }
}

static void on_suite_started(YacuReportPtr *reports, const YacuSuite *suite)
{
    for (YacuReportPtr *reportPtr2Ptr = reports; !end_of_reports(*reportPtr2Ptr); reportPtr2Ptr++)
    {
        if (*reportPtr2Ptr == NULL)
        {
            continue;
        }
        YacuReport *report = *reportPtr2Ptr;
        report->action(report->state, SUITE_STARTED, suite, NULL);
    }
}

static void on_test_started(YacuReportPtr *reports, const YacuSuite *suite, const YacuTestRun *testRun)
{
    for (YacuReportPtr *reportPtr2Ptr = reports; !end_of_reports(*reportPtr2Ptr); reportPtr2Ptr++)
    {
        if (*reportPtr2Ptr == NULL)
        {
            continue;
        }
        YacuReport *report = *reportPtr2Ptr;
        report->action(report->state, TEST_RUN_STARTED, suite, testRun);
    }
}

static void on_test_finished(YacuReportPtr *reports, const YacuSuite *suite, const YacuTestRun *testRun)
{
    for (YacuReportPtr *reportPtr2Ptr = reports; !end_of_reports(*reportPtr2Ptr); reportPtr2Ptr++)
    {
        if (*reportPtr2Ptr == NULL)
        {
            continue;
        }
        YacuReport *report = *reportPtr2Ptr;
        report->action(report->state, TEST_RUN_FINISHED, suite, testRun);
    }
}

static void on_suite_finished(YacuReportPtr *reports, const YacuSuite *suite)
{
    for (YacuReportPtr *reportPtr2Ptr = reports; !end_of_reports(*reportPtr2Ptr); reportPtr2Ptr++)
    {
        if (*reportPtr2Ptr == NULL)
        {
            continue;
        }
        YacuReport *report = *reportPtr2Ptr;
        report->action(report->state, SUITE_FINISHED, suite, NULL);
    }
}

static void on_testing_finished(YacuReportPtr *reports)
{
    for (YacuReportPtr *reportPtr2Ptr = reports; !end_of_reports(*reportPtr2Ptr); reportPtr2Ptr++)
    {
        if (*reportPtr2Ptr == NULL)
        {
            continue;
        }
        YacuReport *report = *reportPtr2Ptr;
        report->action(report->state, TESTING_FINISHED, NULL, NULL);
    }
}

YacuReport END_OF_REPORTS = {NULL, NULL};

static YacuStatus yacu_run_test(const YacuSuite *suite, const YacuTest *test, YacuReportPtr *reports, const void *runData)
{
    YacuTestRun testRun = {.result = OK, .message = "", .reports = reports, .runData = runData, .test = test, .suite = suite};
    on_test_started(reports, suite, &testRun);
    test->fcn(&testRun);
    on_test_finished(reports, suite, &testRun);
    return testRun.result;
}

void yacu_assert(YacuTestRun *testRun, bool condition, const char *fmt, ...)
{
    if (!(condition))
    {
        testRun->result = TEST_FAILURE;

        va_list args;
        va_start(args, fmt);
        vbuffer_append(testRun->message, YACU_TEST_RUN_MESSAGE_MAX_SIZE, fmt, args);
        va_end(args);
        if (testRun->result == TEST_FAILURE)
        {
            on_test_finished(testRun->reports, testRun->suite, testRun);
            on_suite_finished(testRun->reports, testRun->suite);
            on_testing_finished(testRun->reports);
        }
        exit(TEST_FAILURE);
    }
}

static JUnitReport junit_initial_state(const char *jUnitPath)
{
    JUnitReport jUnitInitial;
    strcpy(jUnitInitial.jUnitBuffer, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                                     "<testsuites>\n");
    jUnitInitial.jUnitPath = jUnitPath;
    return jUnitInitial;
}

YacuStatus yacu_execute(YacuOptions options, const YacuSuite *suites)
{
    YacuStatus runStatus = OK;
    JUnitReport jUnitInitial = junit_initial_state(options.jUnitPath);
    YacuReport jUnitReport = {&jUnitInitial, junit_report_action};
    YacuReport stdoutReport = {NULL, stdout_report_action};

    YacuReportPtr reports[] = {&jUnitReport, &stdoutReport, options.customReport, &END_OF_REPORTS};

    for (const YacuSuite *suiteIt = suites; !end_of_suites(*suiteIt); suiteIt++)
    {
        if (options.suiteName == NULL || strcmp(options.suiteName, suiteIt->name) == 0)
        {
            on_suite_started(reports, suiteIt);
            for (const YacuTest *testIt = suiteIt->tests; !end_of_tests(*testIt); testIt++)
            {
                if (options.testName == NULL || strcmp(options.testName, testIt->name) == 0)
                {
                    yacu_run_test(suiteIt, testIt, reports, options.runData);
                }
            }
        }
        on_suite_finished(reports, suiteIt);
    }
    on_testing_finished(reports);
    return runStatus;
}
