#ifndef ROUNDROBINMEDIUMPARTICIPANT_H
#define ROUNDROBINMEDIUMPARTICIPANT_H

#include <queue>

#include "mediumparticipant.h"

class MediumMessage;
class RoundRobinMessageScheduler;

class RoundRobinMediumParticipant:
        public MediumParticipant
{
public:
    RoundRobinMediumParticipant(int mediumAddress, RoundRobinMessageScheduler *scheduler);
    virtual ~RoundRobinMediumParticipant();

public:
    virtual int send(uint8_t data[], int size);
    virtual int sendTo(uint8_t data[], int size, int destAddr);
    int mediumAddress();

    virtual void registerReceiver(MessageReceiver *receiver);

private:
    /**
     * @brief popMessage returns waiting medium message that was sent least recently (FIFO);
     * Ownership is transferred to caller. If no message left, null returned.
     * @return leasr recent message or null.
     */
    MediumMessage *popMessage();

    /**
     * @brief receive invoked by scheduler, when there is a message directed to participant.
     */
    void receive(int srcAddr, uint8_t data[], int size);

    friend class RoundRobinMessageScheduler;

private:
    int _send(uint8_t data[], int size, int destAddr);

private:
    int _mediumAddress;
    RoundRobinMessageScheduler *_scheduler;
    MessageReceiver *_receiver;

    std::queue<MediumMessage*> _messages;
};

#endif // ROUNDROBINMEDIUMPARTICIPANT_H
