#include "lamportclock.h"

LamportClock::LamportClock():
    _currValue(0)
{
}

LamportClock::LamportClockType LamportClock::currValue()
{
    return _currValue;
}

void LamportClock::updateValue(LamportClock::LamportClockType otherValue)
{
    if (otherValue > _currValue)
        _currValue = otherValue;

    eventOccured();
}

void LamportClock::eventOccured()
{
    ++_currValue;
}

int LamportClock::compareClocks(LamportClock::LamportClockType c1, int procId1, LamportClock::LamportClockType c2, int procId2)
{
    if (c1 > c2)
        return 1;
    else if (c2 < c1)
        return -1;
    else {
        return (procId1 > procId2) ? 1 : -1;
    }
}
