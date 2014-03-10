#include "lamportclocktest.h"

#include <assert.h>
#include <stdint.h>

#include "lamportclockhandler.h"

LamportClockTest::LamportClockTest()
{
}

void LamportClockTest::test()
{
    LamportClock clock;
    LamportClockHandler _handler(&clock);

    int ret;
    //int ret = _handler.appendClockToMsg(0, 0, 0);
    //assert(0 > ret);

    for (int i = 0; i < 5; ++i) {
        const int MsgSize = (i + 1) * sizeof(LamportClock::LamportClockType);
        uint8_t fakeMessage[MsgSize];
        ret = _handler.appendClockToMsg(fakeMessage, i * sizeof(LamportClock::LamportClockType), MsgSize);
        assert(sizeof(LamportClock::LamportClockType) == ret);

        LamportClock::LamportClockType msgClock;
        ret = _handler.removeClockFromMsg(fakeMessage, MsgSize, &msgClock);
        assert(sizeof(LamportClock::LamportClockType) == ret);
        assert((2*i+1) == msgClock);
        assert((2*i+2) == clock.currValue());
    }
}
