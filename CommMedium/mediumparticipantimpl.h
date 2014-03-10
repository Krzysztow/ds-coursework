#ifndef MEDIUMPARTICIPANTIMPL_H
#define MEDIUMPARTICIPANTIMPL_H

#include <queue>

#include "mediumparticipant.h"

class MediumMessage;
class MediumDispatcher;

class MediumParticipantImpl:
        public MediumParticipant
{
public:
    MediumParticipantImpl(int mediumAddress, MediumDispatcher *scheduler);
    virtual ~MediumParticipantImpl();

public:
    virtual int send(uint8_t data[], int size);
    virtual int sendTo(uint8_t data[], int size, int destAddr);
    virtual int mediumAddress();

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

    friend class RoundRobinMediumDispatcher;

private:
    int _send(uint8_t data[], int size, int destAddr);

private:
    int _mediumAddress;
    MediumDispatcher *_scheduler;
    MessageReceiver *_receiver;

    std::queue<MediumMessage*> _messages;
};

#endif // MEDIUMPARTICIPANTIMPL_H
