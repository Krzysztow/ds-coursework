#ifndef ROUNDROBINMEDIUMDISPATCHER_H
#define ROUNDROBINMEDIUMDISPATCHER_H

#include <map>
#include <queue>
#include <stdint.h>

#include "mediumdispatcher.h"

class MediumParticipantImpl;
class ParticipantData;
class MediumMessage;

/**
 * @brief The RoundRobinMediumDispatcher class - dispatcher class that takes
 * iterates over it's participants pops one least recent message for each, and sends it to
 * appropriate destinations.
 *
 * This seems to simulate network communication pretty well:
 * - message ordering is preserved (at least within one process);
 * - if one process sends many messages it will not use all the bus, but will share it fairly with other processes.
 */

class RoundRobinMediumDispatcher:
        public MediumDispatcher
{
public:
    RoundRobinMediumDispatcher();

    /**
     * @brief runs message dispatching over all praticipants, once.
     * @return 0 if there are yet messages to be dispatched.
     */
    virtual DispatchedMessagesResult dispatchMessages();

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

    /**
     * @brief containsParticipant checks if participant with @param address is registered on the bus.
     */
    virtual bool containsParticipant(int address);
    /**
     * @brief isParticipantReachable now does tha same as @sa containsParticipant()
     * The idea is to be able to check
     * @return
     */
    virtual bool isParticipantReachable(int srcAddress, int destAddress);

private:
    std::map<int, ParticipantData*> _participants;

    void _receiveMesage(MediumParticipantImpl *participant, MediumMessage *md);
    int _send(int srcAddr, uint8_t data[], int size, int destAddr);
};

#endif // ROUNDROBINMEDIUMDISPATCHER_H
