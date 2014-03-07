#include "RoundRobinMedium/rrschedulertester.h"

#include <assert.h>
#include <cstring>
#include <iostream>
#include <map>
#include <stdio.h>
#include <time.h>

#include "RoundRobinMedium/roundrobinmediumdispatcher.h"
#include "RoundRobinMedium/mediumparticipantimpl.h"
#include "mediumparticipant.h"

using namespace std;

static int totalNumOfParticipants = 0;

class TestParticipant:
        MessageReceiver {
public:
    TestParticipant(int address, RoundRobinMediumDispatcher *scheduler, int *totalCntrPtr):
        _mediumParticipant(new MediumParticipantImpl(address, scheduler)),
        _thisTxLeft(5),
        _totalMessagesPtr(totalCntrPtr)
    {
        _mediumParticipant->registerReceiver(this);
    }

    void sendSomeMessages() {
        if (_thisTxLeft <= 0)
            return;

        int r = rand() % 3;
        for (int i = 0; i < r; ++i) {
            uint8_t data[64];
            int destAddr = rand() % (totalNumOfParticipants + 1) - 1;
            int dataSize = sprintf((char*)data, "SEND: from %d to %d", _mediumParticipant->mediumAddress(), destAddr);
            std::cout << data << std::endl;
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

    virtual void receive(int srcAddress, uint8_t data[], int size) {
        assert(strlen((char*)data) == size);
        std::cout << "RCVD: " <<  data <<  "from " << srcAddress << std::endl;
        --(*_totalMessagesPtr);

        sendSomeMessages();
    }

private:
    MediumParticipantImpl *_mediumParticipant;
    int _thisTxLeft;
    int * const _totalMessagesPtr;
};


int RRSchedulerTester::test()
{
    const int NoOfParticipants = 4;
    totalNumOfParticipants = NoOfParticipants;
    std::map<int, TestParticipant*> participants;
    int totalMsgs = 0;

    timespec t;
    clock_gettime(CLOCK_MONOTONIC_COARSE, &t);
    srand(t.tv_nsec + t.tv_sec);

    RoundRobinMediumDispatcher scheduler;
    for (int i = 0; i < NoOfParticipants; ++i) {
        TestParticipant *tp = new TestParticipant(i, &scheduler, &totalMsgs);
        participants[i] = tp;
    }

    int startingParticipant = rand() % NoOfParticipants;
    participants[startingParticipant]->act();

    scheduler.exec();

    assert(totalMsgs == 0);
    for (int i = 0; i < NoOfParticipants; ++i) {
        delete participants[i];
    }

    return 0;
}

