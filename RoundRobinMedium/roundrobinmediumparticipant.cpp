#include "roundrobinmediumparticipant.h"

#include <assert.h>

#include "RoundRobinMedium/mediummessage.h"
#include "RoundRobinMedium/roundrobinmessagescheduler.h"

RoundRobinMediumParticipant::RoundRobinMediumParticipant(int mediumAddress, RoundRobinMessageScheduler *scheduler):
    _mediumAddress(mediumAddress),
    _scheduler(scheduler),
    _receiver(0)
{
    _scheduler->registerParticipant(this);
}

RoundRobinMediumParticipant::~RoundRobinMediumParticipant()
{
    _scheduler->deregisterParticipant(this);
}

int RoundRobinMediumParticipant::_send(uint8_t data[], int size, int destAddr)
{
    //just make sure there is such a destination participant (or broadcast)
    if (!_scheduler->isBcastAddress(destAddr)) {
        if (!_scheduler->isParticipantReachable(destAddr))//no destination - so there would be no connection, fail
            return -2;
    }

    //is sender registered with medium?
    int srcAddr = mediumAddress();
    if (!_scheduler->containsParticipant(srcAddr))
        return -3;

    MediumMessage *md = new MediumMessage(srcAddr, destAddr, data, size);
    _messages.push(md);

    return size;
}

int RoundRobinMediumParticipant::sendTo(uint8_t data[], int size, int destAddr)
{
    assert(!_scheduler->isBcastAddress(destAddr));
    if (_scheduler->isBcastAddress(destAddr))//for broadcasts use send() method
        return -1;

    return _send(data, size, destAddr);
}

int RoundRobinMediumParticipant::send(uint8_t data[], int size)
{
    return _send(data, size, _scheduler->bcastAddress());
}

void RoundRobinMediumParticipant::registerReceiver(MessageReceiver *receiver)
{
    _receiver = receiver;
}

MediumMessage *RoundRobinMediumParticipant::popMessage()
{
    if (0 == _messages.size())
        return 0;
    MediumMessage *mm = _messages.front();
    _messages.pop();
    return mm;
}

int RoundRobinMediumParticipant::mediumAddress()
{
    return _mediumAddress;
}

void RoundRobinMediumParticipant::receive(int srcAddr, uint8_t data[], int size)
{
    if (0 != _receiver)
        _receiver->receive(srcAddr, data, size);
}
