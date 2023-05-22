#include <yacu.h>
#include <assertions.h>

void test_assert_cmp_int(YacuTestRun *testRun)
{
    int small = -1;

    YACU_ASSERT_LT_INT(testRun, small, 0);
    YACU_ASSERT_LE_INT(testRun, small, -1);
    YACU_ASSERT_EQ_INT(testRun, small, -1);
    YACU_ASSERT_GE_INT(testRun, small, -1);
    YACU_ASSERT_GT_INT(testRun, small, -2);
}

void test_assert_cmp_uint(YacuTestRun *testRun)
{
    unsigned int small = 1;

    YACU_ASSERT_LT_UINT(testRun, small, 2);
    YACU_ASSERT_LE_UINT(testRun, small, 1);
    YACU_ASSERT_EQ_UINT(testRun, small, 1);
    YACU_ASSERT_GE_UINT(testRun, small, 1);
    YACU_ASSERT_GT_UINT(testRun, small, 0);
}

void test_assert_eq_char(YacuTestRun *testRun)
{
    char x = 'a';

    YACU_ASSERT_EQ_CHAR(testRun, x, 'a');
}

void test_assert_eq_dbl(YacuTestRun *testRun)
{
    double x = 1.1;

    YACU_ASSERT_APPROX_EQ_DBL(testRun, x, 1.2, 0.2);
}

void test_assert_true(YacuTestRun *testRun)
{
    int x = 1;

    YACU_ASSERT_TRUE(testRun, x == 1);
}

YacuTest assertionTests[] = {
    {"cmpIntTest", &test_assert_cmp_int},
    {"cmpUIntTest", &test_assert_cmp_uint},
    {"eqCharTest", &test_assert_eq_char},
    {"eqDblTest", &test_assert_eq_dbl},
    {"trueTest", &test_assert_true},
    END_OF_TESTS};
