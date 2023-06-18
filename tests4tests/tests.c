#include <yacu.h>

#include <assertions.h>
#include <failures.h>
#include <others.h>


YacuSuite suites[] = {
    {"Assertions", assertionTests},
    {"AssertionFailures", assertionFailuresTests},
    {"Others", otherTests},
    END_OF_SUITES};

int main(int argc, char const *argv[])
{
    YacuOptions options = yacu_process_args(argc, argv);
    yacu_execute(options, suites);
    return 0;
}
