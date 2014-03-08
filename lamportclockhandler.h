#ifndef LAMPORTCLOCKHANDLER_H
#define LAMPORTCLOCKHANDLER_H

typedef unsigned int LamportClock;

class LamportClockHandler
{
public:
    LamportClockHandler();

    void eventOccured();
    void messageClockReceived(LamportClock msgClock);

    LamportClock currentClock();

private:
    LamportClock _currClock;
};

#endif // LAMPORTCLOCKHANDLER_H
