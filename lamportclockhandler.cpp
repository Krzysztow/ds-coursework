#include "lamportclockhandler.h"

#include <assert.h>
#include <cstring>

#include "lamportclock.h"

LamportClockHandler::LamportClockHandler(LamportClock *clock):
    _clock(clock)
{
}

int LamportClockHandler::appendClockToMsg(uint8_t *message, int currentSize, int maxSize)
{
    if (maxSize - currentSize < sizeof(LamportClock::LamportClockType)) {
        assert(false);
        return -1;
    }

    _clock->eventOccured();
    LamportClock::LamportClockType value = _clock->currValue();
    uint8_t *ptr = &message[currentSize];
    memcpy(&message[currentSize], &value, sizeof(LamportClock::LamportClockType));

    return sizeof(LamportClock::LamportClockType);
}

int LamportClockHandler::removeClockFromMsg(uint8_t *msg, int currentSize, LamportClock::LamportClockType *msgClock)
{
    if (currentSize < sizeof(LamportClock::LamportClockType)) {
        assert(false);
        return -1;
    }
    else {
        memcpy(msgClock, msg + currentSize - sizeof(LamportClock::LamportClockType), sizeof(LamportClock::LamportClockType));
        _clock->updateValue(*msgClock);
        return sizeof(LamportClock::LamportClockType);
    }
}

