#ifndef LAMPORTCLOCKHANDLER_H
#define LAMPORTCLOCKHANDLER_H

#include <stdint.h>

#include "lamportclock.h"

class LamportClockHandler
{
public:
    LamportClockHandler(LamportClock *clock);

    int appendClockToMsg(uint8_t *message, int currentSize, int maxSize);
    int removeClockFromMsg(uint8_t *msg, int currentSize, LamportClock::LamportClockType *msgClock);

private:
    LamportClock *_clock;
};

#endif // LAMPORTCLOCKHANDLER_H
