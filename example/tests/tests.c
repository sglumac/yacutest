#include <yacu.h>

void test_sum(YacuTestRun testRun)
{
}

YacuTest calculatorTests[] = {
    {"SumTest", &test_sum},
    END_OF_TESTS};

YacuSuite suites[] = {
    {"CalculatorSuite", calculatorTests},
    END_OF_SUITES};

int main(int argc, char const *argv[])
{
    YacuOptions options = yacu_process_args(argc, argv);
    return yacu_execute(options, suites);
}
