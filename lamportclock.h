#ifndef LAMPORTCLOCK_H
#define LAMPORTCLOCK_H

class LamportClock
{
public:
    typedef unsigned int LamportClockType;
    LamportClock();

    LamportClockType currValue();
    void updateValue(LamportClockType otherValue);
    void eventOccured();

    static int compareClocks(LamportClockType c1, int procId1, LamportClockType c2, int procId2);

private:

    LamportClockType _currValue
    ;
};

#endif // LAMPORTCLOCK_H
