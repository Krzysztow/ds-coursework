#ifndef ROUNDROBINMEDIUMDISPATCHER_H
#define ROUNDROBINMEDIUMDISPATCHER_H

#include <map>
#include <queue>
#include <stdint.h>

#include "mediumdispatcher.h"

class MediumParticipantImpl;
class ParticipantData;
class MediumMessage;

class RoundRobinMediumDispatcher:
        public MediumDispatcher
{
public:
    RoundRobinMediumDispatcher();

    /**
     * @brief loop start communication loop when all participants are registerd.
     * Loop is exited once all participants deregistered from it.
     * @return 0 on success (always?).
     */
    virtual int exec();

    //transmission methods
    /**
     * @brief sendTo direct transmission
     * @param srcAddr
     * @param data
     * @param size
     * @param destAddr
     * @return
     */
    virtual int sendTo(int srcAddr, uint8_t data[], int size, int destAddr);
    /**
     * @brief send broadcast transmission
     * @param srcAddr
     * @param data
     * @param size
     * @return
     */
    virtual int send(int srcAddr, uint8_t data[], int size);

    virtual bool registerParticipant(MediumParticipantImpl *participant);
    virtual bool deregisterParticipant(MediumParticipantImpl *participant);

    virtual bool isParticipantReachable(int address);
    virtual bool containsParticipant(int address);

private:
    std::map<int, ParticipantData*> _participants;

    void _receiveMesage(MediumParticipantImpl *participant, MediumMessage *md);
    int _send(int srcAddr, uint8_t data[], int size, int destAddr);
};

#endif // ROUNDROBINMEDIUMDISPATCHER_H
