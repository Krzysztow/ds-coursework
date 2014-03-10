#include "tests.h"

#include "tests/rrschedulertester.h"
#include "tests/operationstester.h"
#include "tests/lamportclocktest.h"

Tests::Tests()
{
}

void Tests::test()
{
    RRSchedulerTester tester;
    tester.test();

    OperationsTester tester2;
    tester2.test();

    LamportClockTest tester3;
    tester3.test();
}
