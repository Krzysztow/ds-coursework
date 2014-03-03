#include <iostream>
#include <stdio.h>
#include <assert.h>
#include <cstring>

using namespace std;

#include "RoundRobinMedium/roundrobinmediumparticipant.h"
#include "RoundRobinMedium/roundrobinmessagescheduler.h"

static int totalNumOfParticipants = 0;

class TestParticipant:
        MessageReceiver {
public:
    TestParticipant(int address, RoundRobinMessageScheduler *scheduler):
        _mediumParticipant(new RoundRobinMediumParticipant(address, scheduler)),
        _thisTxLeft(5)
    {
        _mediumParticipant->registerReceiver(this);
    }

    void sendSomeMessages() {
        if (_thisTxLeft <= 0)
            return;

        int r = rand() % 4;
        for (int i = 0; i < r; ++i) {
            uint8_t data[64];
            int destAddr = rand() % (totalNumOfParticipants + 1) - 1;
            int dataSize = sprintf((char*)data, "SEND: from %d to %d", _mediumParticipant->mediumAddress(), destAddr);
            std::cout << data << std::endl;
            if (destAddr >=0) {
                _mediumParticipant->sendto(data, dataSize, destAddr);
                ++_totalMessages;
            }
            else {
                _mediumParticipant->send(data, dataSize);
                _totalMessages += (totalNumOfParticipants - 1);//don't send to itself
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
        --_totalMessages;

        sendSomeMessages();
    }

    static int _totalMessages;
private:
    RoundRobinMediumParticipant *_mediumParticipant;
    int _thisTxLeft;
};

int TestParticipant::_totalMessages = 0;

#include <map>

int main()
{
    const int NoOfParticipants = 3;
    totalNumOfParticipants = NoOfParticipants;
    std::map<int, TestParticipant*> participants;

    RoundRobinMessageScheduler scheduler;
    for (int i = 0; i < NoOfParticipants; ++i) {
        TestParticipant *tp = new TestParticipant(i, &scheduler);
        participants[i] = tp;
    }

    int startingParticipant = rand() % NoOfParticipants;
    participants[startingParticipant]->act();

    scheduler.exec();

    TestParticipant *tp = participants[0];
    int left = tp->_totalMessages;
    assert(left == 0);
    for (int i = 0; i < NoOfParticipants; ++i) {
        delete participants[i];
    }

    return 0;
}

