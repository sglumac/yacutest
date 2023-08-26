#ifndef COMMON_H
#define COMMON_H

#include <yacu.h>

typedef void ForkedAction(YacuTestRun *forkedTestRun);

YacuStatus forked_test(YacuTestRun *testRun, const char *reportPath, ForkedAction forkedAction, char *failureMessage);

YacuProcessHandle yacu_fork();

bool is_forked(YacuProcessHandle pid);

YacuStatus wait_for_forked(YacuProcessHandle forkedId);

#endif // COMMON_H
