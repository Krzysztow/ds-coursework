#include "lamportclockhandler.h"

LamportClockHandler::LamportClockHandler():
    _currClock(0)
{
}

void LamportClockHandler::eventOccured()
{
    ++_currClock;
}

void LamportClockHandler::messageClockReceived(LamportClock msgClock)
{
    if (msgClock > _currClock)
        _currClock = msgClock;

    ++_currClock;
}

LamportClock LamportClockHandler::currentClock()
{
    return _currClock;
}
