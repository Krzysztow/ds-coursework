#include "tests/rrschedulertester.h"
#include "tests/operationstester.h"

int main(int argc, const char *argv[]) {
    RRSchedulerTester tester;
    tester.test();

    OperationsTester tester2;
    tester2.test();

    return 0;
}
