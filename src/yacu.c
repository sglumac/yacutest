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
    return report == NULL
               ? false
               : report->on_suite_finished == NULL && report->on_suite_started == NULL && report->on_suites_finished == NULL && report->on_suites_started == NULL && report->on_test_finished == NULL && report->on_test_started == NULL && report->state == NULL;
}

YacuOptions yacu_default_options()
{
    YacuOptions options = {
        .action = RUN_TESTS,
        .fork = true,
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

const char *helpString = "Usage: [<option1> <option2>...]\n"
                         " --no-fork\n"
                         "     Do not use fork from unistd.h to run the tests.\n"
                         "     The tests are executed sequentially and execution stops when the first one fails.\n"
                         " --test <suiteName> <testName>\n"
                         "     Run a single test named <testName> from the suite named <suiteName>.\n"
                         " --suite <suiteName>\n"
                         "     Run all tests from the suite named <suiteName>.\n"
                         " --list\n"
                         "     Prints the list of all available suites and tests.\n"
                         "     This should be the only option used.\n"
                         " --junit <xmlFile>"
                         "     Save the test reports in <xmlFile> in JUnit format.\n"
                         " --help\n"
                         "     Prints this help message and exits.\n"
                         "     This should be the only option used.";

static void list_suites(FILE *file, const YacuSuite *suites)
{
    for (const YacuSuite *suite = suites; !end_of_suites(*suite); suite++)
    {
        fprintf(file, "%s\n", suite->name);
        for (const YacuTest *test = suite->tests; !end_of_tests(*test); test++)
        {
            fprintf(file, "- %s\n", test->name);
        }
    }
}

static void process_test_or_suite_arg(int i, int argc, char const *argv[], YacuOptions *options, bool withTest)
{
    if (argc <= i + (withTest ? 2 : 1))
    {
        exit(WRONG_ARGS);
    }
    options->suiteName = argv[i + 1];
    options->action = RUN_TESTS;
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
        if (strcmp(argv[i], "--help") == 0)
        {
            options.action = HELP;
        }
        else if (strcmp(argv[i], "--list") == 0)
        {
            options.action = LIST;
        }
        else if (strcmp(argv[i], "--test") == 0)
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
        else if (strcmp(argv[i], "--no-fork") == 0)
        {
            options.fork = false;
        }
        else
        {
            exit(WRONG_ARGS);
        }
    }
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

typedef struct JUnitReport
{
    char jUnitBuffer[YACU_TEST_RUN_MESSAGE_MAX_SIZE];
    const char *jUnitPath;
} JUnitReport;

#define UNUSED(x) (void)(x)
static void junit_on_start_suites(YacuReportState state)
{
    UNUSED(state);
}

static void junit_on_start_suite(YacuReportState state, const char *suiteName)
{
    JUnitReport *current = (JUnitReport *)state;
    buffer_append(current->jUnitBuffer, YACU_TEST_RUN_MESSAGE_MAX_SIZE,
                  "  <testsuite package=\"\" id=\"0\" name=\"%s\"", suiteName);
    buffer_append(current->jUnitBuffer, YACU_TEST_RUN_MESSAGE_MAX_SIZE,
                  " timestamp=\"1900-12-12T%11:11:11\"");
    buffer_append(current->jUnitBuffer, YACU_TEST_RUN_MESSAGE_MAX_SIZE,
                  " hostname=\"-\" tests=\"4\" failures=\"2\" errors=\"1\" time=\"3\">\n");
    buffer_append(current->jUnitBuffer, YACU_TEST_RUN_MESSAGE_MAX_SIZE,
                  "    <properties/>\n");
}

static void junit_on_test_start(YacuReportState state, const char *testName)
{
    JUnitReport *current = (JUnitReport *)state;
    buffer_append(current->jUnitBuffer, YACU_TEST_RUN_MESSAGE_MAX_SIZE,
                  "    <testcase classname=\"\" name=\"%s\" time=\"0.0\">\n",
                  testName);
}

static void junit_on_test_finished(YacuReportState state, YacuStatus result, const char *message)
{
    UNUSED(result);
    UNUSED(message);
    JUnitReport *current = (JUnitReport *)state;
    buffer_append(current->jUnitBuffer, YACU_TEST_RUN_MESSAGE_MAX_SIZE,
                  "    </testcase>\n");
}

static void junit_on_suite_finished(YacuReportState state)
{
    JUnitReport *current = state;
    buffer_append(current->jUnitBuffer, YACU_TEST_RUN_MESSAGE_MAX_SIZE,
                  "    <system-out/>\n"
                  "    <system-err/>\n"
                  "  </testsuite>\n");
}

static void junit_on_suites_finished(YacuReportState state)
{
    JUnitReport *current = (JUnitReport *)state;
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
}

static void stdout_on_start_suites(YacuReportState state)
{
    UNUSED(state);
}

static void stdout_on_start_suite(YacuReportState state, const char *suiteName)
{
    UNUSED(state);
    printf("#%s\n", suiteName);
}

static void stdout_on_test_start(YacuReportState state, const char *testName)
{
    UNUSED(state);
    printf("  ##%s\n", testName);
}

static void stdout_on_test_finished(YacuReportState state, YacuStatus result, const char *message)
{
    UNUSED(state);
    switch (result)
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
    if (strlen(message) > 0)
    {
        printf("    %s\n", message);
    }
}

static void stdout_on_suite_finished(YacuReportState state)
{
    UNUSED(state);
}

static void stdout_on_suites_finished(YacuReportState state)
{
    UNUSED(state);
}

static void on_suites_started(YacuReportPtr *reports)
{
    for (YacuReportPtr *reportPtr2Ptr = reports; !end_of_reports(*reportPtr2Ptr); reportPtr2Ptr++)
    {
        if (*reportPtr2Ptr == NULL)
        {
            continue;
        }
        YacuReport *report = *reportPtr2Ptr;
        report->on_suites_started(report->state);
    }
}

static void on_suite_started(YacuReportPtr *reports, const char *suiteName)
{
    for (YacuReportPtr *reportPtr2Ptr = reports; !end_of_reports(*reportPtr2Ptr); reportPtr2Ptr++)
    {
        if (*reportPtr2Ptr == NULL)
        {
            continue;
        }
        YacuReport *report = *reportPtr2Ptr;
        report->on_suite_started(report->state, suiteName);
    }
}

static void on_test_started(YacuReportPtr *reports, const char *testName)
{
    for (YacuReportPtr *reportPtr2Ptr = reports; !end_of_reports(*reportPtr2Ptr); reportPtr2Ptr++)
    {
        if (*reportPtr2Ptr == NULL)
        {
            continue;
        }
        YacuReport *report = *reportPtr2Ptr;
        report->on_test_started(report->state, testName);
    }
}

static void on_test_finished(YacuReportPtr *reports, YacuStatus result, const char *message)
{
    for (YacuReportPtr *reportPtr2Ptr = reports; !end_of_reports(*reportPtr2Ptr); reportPtr2Ptr++)
    {
        if (*reportPtr2Ptr == NULL)
        {
            continue;
        }
        YacuReport *report = *reportPtr2Ptr;
        report->on_test_finished(report->state, result, message);
    }
}

static void on_suite_finished(YacuReportPtr *reports)
{
    for (YacuReportPtr *reportPtr2Ptr = reports; !end_of_reports(*reportPtr2Ptr); reportPtr2Ptr++)
    {
        if (*reportPtr2Ptr == NULL)
        {
            continue;
        }
        YacuReport *report = *reportPtr2Ptr;
        report->on_suite_finished(report->state);
    }
}

static void on_suites_finished(YacuReportPtr *reports)
{
    for (YacuReportPtr *reportPtr2Ptr = reports; !end_of_reports(*reportPtr2Ptr); reportPtr2Ptr++)
    {
        if (*reportPtr2Ptr == NULL)
        {
            continue;
        }
        YacuReport *report = *reportPtr2Ptr;
        report->on_suites_finished(report->state);
    }
}

YacuReport END_OF_REPORTS = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};

static YacuStatus yacu_run_test(bool forked, YacuTest test, YacuReportPtr *reports, const void *runData)
{
    YacuTestRun testRun = {.result = OK, .message = "", .forked = forked, .reports = reports, .runData = runData};
    test.fcn(&testRun);
    on_test_finished(reports, testRun.result, testRun.message);
    return testRun.result;
}

static YacuStatus yacu_run_forked_test(YacuTest test, YacuReportPtr *reports, const void *runData)
{
    YacuStatus returnCode;
    YacuProcessHandle pid = yacu_fork();
    if (is_forked(pid))
    {
        exit(yacu_run_test(true, test, reports, runData));
    }
    else
    {
        returnCode = wait_for_forked(pid);
    }
    if (returnCode != OK)
    {
        on_test_finished(reports, returnCode, "Something failed!");
    }
    return returnCode;
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
        if (!testRun->forked && testRun->result == TEST_FAILURE)
        {
            on_test_finished(testRun->reports, testRun->result, testRun->message);
            on_suite_finished(testRun->reports);
            on_suites_finished(testRun->reports);
        }
        exit(TEST_FAILURE);
    }
}

static YacuStatus run_tests(YacuOptions options, const YacuSuite *suites)
{
    YacuStatus runStatus = OK;
    JUnitReport jUnitInitial = {
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<testsuites>\n",
        options.jUnitPath};
    YacuReport jUnitReport = {
        &jUnitInitial,
        junit_on_start_suites, junit_on_start_suite, junit_on_test_start,
        junit_on_test_finished, junit_on_suite_finished, junit_on_suites_finished};
    YacuReport stdoutReport = {
        NULL,
        stdout_on_start_suites, stdout_on_start_suite, stdout_on_test_start,
        stdout_on_test_finished, stdout_on_suite_finished, stdout_on_suites_finished};

    YacuReportPtr reports[] = {&jUnitReport, &stdoutReport, options.customReport, &END_OF_REPORTS};

    on_suites_started(reports);
    for (const YacuSuite *suiteIt = suites; !end_of_suites(*suiteIt); suiteIt++)
    {
        if (options.suiteName == NULL || strcmp(options.suiteName, suiteIt->name) == 0)
        {
            on_suite_started(reports, suiteIt->name);
            for (const YacuTest *testIt = suiteIt->tests; !end_of_tests(*testIt); testIt++)
            {
                if (options.testName == NULL || strcmp(options.testName, testIt->name) == 0)
                {
                    on_test_started(reports, testIt->name);
                    YacuStatus testStatus = options.fork
                                                ? yacu_run_forked_test(*testIt, reports, options.runData)
                                                : yacu_run_test(false, *testIt, reports, options.runData);
                    if (testStatus == TEST_ERROR || (testStatus == TEST_FAILURE && runStatus != TEST_ERROR))
                    {
                        runStatus = testStatus;
                    }
                }
            }
        }
        on_suite_finished(reports);
    }
    on_suites_finished(reports);
    return runStatus;
}

YacuStatus yacu_execute(YacuOptions options, const YacuSuite *suites)
{
    switch (options.action)
    {
    case LIST:
        list_suites(stdout, suites);
        return OK;
    case RUN_TESTS:
        return run_tests(options, suites);
    case HELP:
        printf("%s\n", helpString);
        return OK;
    default:
        return FATAL;
    }
}
