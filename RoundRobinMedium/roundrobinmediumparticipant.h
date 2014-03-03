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
    void receive(int srcAddr, uint8_t data[], int size);

public:
    virtual int send(uint8_t data[], int size);
    virtual int sendto(uint8_t data[], int size, int destAddr);
    virtual void registerReceiver(MessageReceiver *receiver);

private:
    int _mediumAddress;
    RoundRobinMessageScheduler *_scheduler;
    MessageReceiver *_receiver;
};

#endif // ROUNDROBINMEDIUMPARTICIPANT_H
