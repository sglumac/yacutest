#include <yacu.h>

#include <assertions.h>
#include <failures.h>
#include <others.h>

YacuSuite suites[] = {
    {"Assertions", assertionTests},
    {"AssertionFailures", assertionFailuresTests},
    {"Others", otherTests},
    END_OF_SUITES};

int main()
{
    YacuOptions options = yacu_default_options();
    yacu_execute(options, suites);
    return 0;
}
