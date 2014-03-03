#ifndef ROUNDROBINMEDIUMPARTICIPANT_H
#define ROUNDROBINMEDIUMPARTICIPANT_H

#include "mediumparticipant.h"

class RoundRobinMessageScheduler;

class RoundRobinMediumParticipant:
        public MediumParticipant
{
public:
    RoundRobinMediumParticipant(int mediumAddress, RoundRobinMessageScheduler *scheduler);
    virtual ~RoundRobinMediumParticipant();

    int mediumAddress();

public:
    virtual int send(uint8_t data[], int size);
    virtual int sendto(uint8_t data[], int size, int destAddr);\

private:
    int _mediumAddress;
    RoundRobinMessageScheduler *_scheduler;
};

#endif // ROUNDROBINMEDIUMPARTICIPANT_H
