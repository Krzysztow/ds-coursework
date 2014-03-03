#include "roundrobinmediumparticipant.h"

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

int RoundRobinMediumParticipant::send(uint8_t data[], int size)
{
    return _scheduler->send(_mediumAddress, data, size);
}

int RoundRobinMediumParticipant::sendto(uint8_t data[], int size, int destAddr)
{
    return _scheduler->sendTo(_mediumAddress, data, size, destAddr);
}

void RoundRobinMediumParticipant::registerReceiver(MessageReceiver *receiver)
{
    _receiver = receiver;
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
