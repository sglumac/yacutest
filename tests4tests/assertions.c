#include <yacu.h>
#include <assertions.h>

int GET_INT_CALLS = 0;

int get_int(int x)
{
    GET_INT_CALLS++;
    return x;
}

void test_assert_cmp_int(YacuTestRun *testRun)
{
    YACU_ASSERT_LT_INT(testRun, get_int(-1), get_int(0));
    YACU_ASSERT_LE_INT(testRun, get_int(-1), get_int(-1));
    YACU_ASSERT_EQ_INT(testRun, get_int(-1), get_int(-1));
    YACU_ASSERT_GE_INT(testRun, get_int(-1), get_int(-1));
    YACU_ASSERT_GT_INT(testRun, get_int(-1), get_int(-2));

    YACU_ASSERT_EQ_INT(testRun, GET_INT_CALLS, 10);
}

unsigned int GET_UINT_CALLS = 0;

unsigned int get_uint(unsigned int x)
{
    GET_UINT_CALLS++;
    return x;
}

void test_assert_cmp_uint(YacuTestRun *testRun)
{
    YACU_ASSERT_LT_UINT(testRun, get_uint(1), get_uint(2));
    YACU_ASSERT_LE_UINT(testRun, get_uint(1), get_uint(1));
    YACU_ASSERT_EQ_UINT(testRun, get_uint(1), get_uint(1));
    YACU_ASSERT_GE_UINT(testRun, get_uint(1), get_uint(1));
    YACU_ASSERT_GT_UINT(testRun, get_uint(1), get_uint(0));

    YACU_ASSERT_EQ_UINT(testRun, GET_UINT_CALLS, 10);
}

unsigned int GET_CHAR_CALLS = 0;

char get_char(char x)
{
    GET_CHAR_CALLS++;
    return x;
}

void test_assert_eq_char(YacuTestRun *testRun)
{
    char x = 'a';

    YACU_ASSERT_EQ_CHAR(testRun, get_char(x), get_char('a'));

    YACU_ASSERT_EQ_UINT(testRun, GET_CHAR_CALLS, 2);
}

unsigned int GET_DBL_CALLS = 0;

double get_double(double x)
{
    GET_DBL_CALLS++;
    return x;
}

void test_assert_eq_dbl(YacuTestRun *testRun)
{
    double x = 1.1;

    YACU_ASSERT_APPROX_EQ_DBL(testRun, get_double(x), get_double(1.2), get_double(0.2));

    YACU_ASSERT_EQ_UINT(testRun, GET_DBL_CALLS, 3);
}

unsigned int GET_BOOL_CALLS = 0;

bool get_bool(bool x)
{
    GET_BOOL_CALLS++;
    return x;
}

void test_assert_true(YacuTestRun *testRun)
{
    int x = 1;

    YACU_ASSERT_TRUE(testRun, get_bool(x == 1));

    YACU_ASSERT_EQ_UINT(testRun, GET_BOOL_CALLS, 1);
}

YacuTest assertionTests[] = {
    {"cmpIntTest", &test_assert_cmp_int},
    {"cmpUIntTest", &test_assert_cmp_uint},
    {"eqCharTest", &test_assert_eq_char},
    {"eqDblTest", &test_assert_eq_dbl},
    {"trueTest", &test_assert_true},
    END_OF_TESTS};
