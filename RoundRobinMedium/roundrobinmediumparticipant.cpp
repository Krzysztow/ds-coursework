#include "roundrobinmediumparticipant.h"

#include "RoundRobinMedium/roundrobinmessagescheduler.h"

RoundRobinMediumParticipant::RoundRobinMediumParticipant(int mediumAddress, RoundRobinMessageScheduler *scheduler):
    _mediumAddress(mediumAddress),
    _scheduler(scheduler)
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


int RoundRobinMediumParticipant::mediumAddress()
{
    return _mediumAddress;
}
