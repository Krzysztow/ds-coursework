#include "tests/rrschedulertester.h"

#include <assert.h>
#include <cstring>
#include <iostream>
#include <map>
#include <stdio.h>
#include <time.h>

#include "klogger.h"
#include "mediumparticipantimpl.h"
#include "mediumparticipant.h"
#include "RoundRobinMedium/roundrobinmediumdispatcher.h"
#include "scheduledobject.h"


using namespace std;

static int totalNumOfParticipants = 0;

class TestParticipant:
        public MessageReceiver,
        public ScheduledObject
{
public:
    TestParticipant(int address, RoundRobinMediumDispatcher *scheduler, int *totalCntrPtr):
        _mediumParticipant(new MediumParticipantImpl(address, scheduler)),
        _thisTxLeft(5),
        _totalMessagesPtr(totalCntrPtr)
    {
        _mediumParticipant->registerReceiver(this);
        scheduler->registerParticipant(_mediumParticipant);
    }

    void sendSomeMessages() {
        if (_thisTxLeft <= 0)
            return;

        int r = rand() % 3;
        for (int i = 0; i < r; ++i) {
            uint8_t data[64];
            int destAddr = rand() % (totalNumOfParticipants + 1) - 1;
            int dataSize = sprintf((char*)data, "SEND: from %d to %d", _mediumParticipant->mediumAddress(), destAddr);
            klogger(klogger::Tests) << data << klogger::end();
            if (destAddr >=0) {
                _mediumParticipant->sendTo(data, dataSize, destAddr);
                ++(*_totalMessagesPtr);
            }
            else {
                _mediumParticipant->send(data, dataSize);
                *_totalMessagesPtr += (totalNumOfParticipants - 1);//don't send to itself
            }

            --_thisTxLeft;
        }
    }

    void act() {
        sendSomeMessages();
    }

    virtual StepResult execStep() {
        sendSomeMessages();
        return (_thisTxLeft > 0 ? ScheduledObject::NotFinished : ScheduledObject::MayFinish);
    }

    virtual void receive(int srcAddress, uint8_t data[], int size) {
        assert(strlen((char*)data) == size);
        klogger(klogger::Tests) << "RCVD: " <<  data <<  "from " << srcAddress << klogger::end();
        --(*_totalMessagesPtr);
    }

private:
    MediumParticipantImpl *_mediumParticipant;
    int _thisTxLeft;
    int * const _totalMessagesPtr;
};

#include "scheduledmediumdispatcher.h"
#include "RoundRobinScheduler/roundrobinscheduler.h"

int RRSchedulerTester::test()
{
    const int NoOfParticipants = 4;
    totalNumOfParticipants = NoOfParticipants;
    std::map<int, TestParticipant*> participants;
    int totalMsgs = 0;

    timespec t;
    clock_gettime(CLOCK_MONOTONIC_COARSE, &t);
    srand(t.tv_nsec + t.tv_sec);


    RoundRobinScheduler sched;
    RoundRobinMediumDispatcher disp;
    ScheduledMediumDispatcher dispatcher(&disp);
    sched.registerObject(&dispatcher);
    for (int i = 0; i < NoOfParticipants; ++i) {
        TestParticipant *tp = new TestParticipant(i, &disp, &totalMsgs);
        participants[i] = tp;
        sched.registerObject(tp);
    }

    int startingParticipant = rand() % NoOfParticipants;
    participants[startingParticipant]->act();

    sched.exec();

    assert(totalMsgs == 0);
    for (int i = 0; i < NoOfParticipants; ++i) {
        delete participants[i];
    }

    return 0;
}

