#ifndef COMMON_H
#define COMMON_H

#include <yacu.h>

typedef void ForkedAction(YacuTestRun *forkedTestRun);

YacuStatus forked_test(YacuTestRun *testRun, const char *reportPath, ForkedAction forkedAction, char* failureMessage);

#endif // COMMON_H
