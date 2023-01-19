#include <yacu.h>

#include <calculator.h>

void test_sum(YacuTestRun testRun)
{
    double a = 2., b = 3.;
    double result = a + b;
    double tol = 1e-6;
    YACU_ASSERT_APPROX_EQ_DBL(testRun, result, 5., tol)
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
