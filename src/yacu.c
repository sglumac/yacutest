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
    return suite.name == 0;
}

static bool end_of_tests(const YacuTest test)
{
    return test.name == 0;
}

YacuOptions default_options()
{
    YacuOptions options = {
        .action = RUN_TESTS,
        .fork = true,
        .suiteName = NULL,
        .testName = NULL,
        .jUnitPath = NULL};
    return options;
}

static void buffer_append(char *buffer, size_t bufferMaxSize, const char *format, ...)
{
    size_t bufferLength = strlen(buffer);
    va_list args;
    va_start(args, format);
    snprintf(buffer + bufferLength, YACU_JUNIT_MAX_SIZE - bufferLength, format, args);
    va_end(args);
}

void test_run_message_append(YacuTestRun *testRun, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    buffer_append(testRun->message, YACU_TEST_RUN_MESSAGE_MAX_SIZE, format, args);
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
    bool testFound = false;
}

YacuOptions yacu_process_args(int argc, char const *argv[])
{
    YacuOptions options = default_options();
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

YacuReturnCode wait_for_forked(YacuProcessHandle forkedId)
{
#ifdef FORK_AVAILABLE
    int status;
    waitpid(forkedId, &status, 0);
    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }
    else
    {
        return TEST_ERROR;
    }
#else
    return OK;
#endif
}

static YacuTestRun yacu_run_test(bool forked, YacuTest test)
{
    YacuTestRun testRun = {OK, ""};
    bool testPassed = false;
    YacuReturnCode returnCode;
    if (forked)
    {
        YacuProcessHandle pid = yacu_fork();
        if (is_forked(pid))
        {
            test.fcn(&testRun);
            exit(OK);
        }
        else
        {
            returnCode = wait_for_forked(pid);
        }
    }
    else
    {
        test.fcn(&testRun);
    }
    return testRun;
}

static void run_tests(YacuOptions options, const YacuSuite *suites)
{
    FILE *jUnitFile = options.jUnitPath == NULL ? NULL : fopen(options.jUnitPath, "w");
    if (options.jUnitPath != NULL && jUnitFile == NULL)
    {
        exit(FILE_FAIL);
    }
    char jUnitBuffer[YACU_TEST_RUN_MESSAGE_MAX_SIZE] =
        "<?xml version=\" 1.0 \" encoding=\" UTF - 8 \"?>\n"
        "<testsuites>\n";
    for (const YacuSuite *suiteIt = suites; !end_of_suites(*suiteIt); suiteIt++)
    {
        if (options.suiteName == NULL || strcmp(options.suiteName, suiteIt->name) == 0)
        {
            buffer_append(jUnitBuffer, YACU_TEST_RUN_MESSAGE_MAX_SIZE, "add suite\n");
            for (const YacuTest *testIt = suiteIt->tests; !end_of_tests(*testIt); testIt++)
            {
                if (options.testName == NULL || strcmp(options.testName, testIt->name) == 0)
                {
                    buffer_append(jUnitBuffer, YACU_TEST_RUN_MESSAGE_MAX_SIZE, "add test\n");
                    YacuTestRun testRun = yacu_run_test(options.fork, *testIt);

                    switch (testRun.result)
                    {
                    case OK:
                        printf(".");
                        break;
                    case TEST_FAILURE:
                        buffer_append(jUnitBuffer, YACU_TEST_RUN_MESSAGE_MAX_SIZE, "add failure\n");
                        printf("F");
                        break;
                    case TEST_ERROR:
                        buffer_append(jUnitBuffer, YACU_TEST_RUN_MESSAGE_MAX_SIZE, "add error\n");
                        printf("E");
                        break;
                    default:
                        break;
                    }
                    if (strlen(testRun.message) > 0)
                    {
                        printf("%s\n", testRun.message);
                    }
                }
            }
        }
    }
    if (jUnitFile != NULL)
    {
        fprintf(jUnitFile, jUnitBuffer);
        fclose(jUnitFile);
    }
}

YacuReturnCode yacu_execute(YacuOptions options, const YacuSuite *suites)
{
    switch (options.action)
    {
    case LIST:
        list_suites(stdout, suites);
        break;
    case RUN_TESTS:
        run_tests(options, suites);
        break;
    case HELP:
        printf("%s\n", helpString);
        break;
    default:
        return FATAL;
    }
    return OK;
}
