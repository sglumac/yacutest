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

void yacu_report(YacuTestRun testRun, const char *msgFormat, ...)
{
    va_list args;
    va_start(args, msgFormat);
    vfprintf(testRun.report, msgFormat, args);
    fflush(testRun.report);
    va_end(args);
}

YacuOptions default_options()
{
    YacuOptions options = {
        .action = RUN_TESTS,
        .fork = true,
        .suiteName = NULL,
        .testName = NULL,
        .reportFile = NULL};
    return options;
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
                         " --report <file>\n"
                         "     Store the test reports in <file>.\n"
                         "     If <file> == \"stdout\" the test reports are written to standard output.\n"
                         "     Test reports are written to standard output by default.\n"
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

static void invalid_arguments()
{
    printf("Invalid arguments!\n");
    printf("%s\n", helpString);
    exit(WRONG_ARGS);
}

static void process_test_or_suite_arg(int i, int argc, char const *argv[], YacuOptions *options, bool withTest)
{
    if (argc <= i + (withTest ? 2 : 1))
    {
        invalid_arguments();
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
        else if (strcmp(argv[i], "--report") == 0)
        {
            if (argc <= i + 1)
            {
                invalid_arguments();
            }
            options.reportFile = argv[i + 1];
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
        printf("Fork failed!");
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

YacuExitCode wait_for_forked(YacuProcessHandle forkedId)
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
        return INTERRUPTED;
    }
#else
    return OK;
#endif
}

static void yacu_run_test(YacuTestRun testRun, YacuTest test)
{
    yacu_report(testRun, "#test: %s\n", test.name);
    bool testPassed = false;
    if (testRun.options.fork)
    {
        YacuProcessHandle pid = yacu_fork();
        if (is_forked(pid))
        {
            test.fcn(testRun);
            exit(OK);
        }
        else
        {
            testPassed = wait_for_forked(pid) == OK;
        }
    }
    else
    {
        test.fcn(testRun);
        testPassed = true;
    }
    const char *testResult = testPassed ? "." : "F";
    yacu_report(testRun, "#result: %s\n", testResult);
}

static void yacu_run_suite(YacuTestRun testRun, YacuSuite suite)
{
    yacu_report(testRun, "##suite: %s\n", suite.name);
    for (const YacuTest *test = suite.tests; !end_of_tests(*test); test++)
    {
        yacu_run_test(testRun, *test);
    }
}

static void yacu_run_suites(YacuTestRun testRun, const YacuSuite *suites)
{
    for (const YacuSuite *suiteIt = suites; !end_of_suites(*suiteIt); suiteIt++)
    {
        yacu_run_suite(testRun, *suiteIt);
    }
}

static void run_tests(YacuOptions options, const YacuSuite *suites)
{
    FILE *reportFile = strcmp(options.reportFile, "stdout") == 0 ? fopen(options.reportFile, "w") : stdout;
    if (reportFile == NULL)
    {
        exit(FILE_FAIL);
   }
   YacuTestRun testRun = {options, reportFile};
   if (options.suiteName)
   {
        for (const YacuSuite *suiteIt = suites; !end_of_suites(*suiteIt); suiteIt++)
        {
            if (strcmp(options.suiteName, suiteIt->name) == 0)
            {
                if (options.testName)
                {
                    for (const YacuTest *testIt = suiteIt->tests; !end_of_tests(*testIt); testIt++)
                    {
                        if (strcmp(options.testName, testIt->name) == 0)
                        {
                            yacu_run_test(testRun, *testIt);
                        }
                    }
                }
                else
                {
                    yacu_run_suite(testRun, *suiteIt);
                }
            }
        }
   }
   else
   {
        yacu_run_suites(testRun, suites);
   }
   fclose(testRun.report);
}

YacuExitCode yacu_execute(YacuOptions options, const YacuSuite *suites)
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
