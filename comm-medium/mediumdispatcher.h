#ifndef MEDIUMDISPATCHER_H
#define MEDIUMDISPATCHER_H

#include <stdint.h>

class MediumParticipantImpl;

class MediumDispatcher {
public:
    enum DispatchedMessagesResult {
        MD_NotDone,
        MD_NoMoreMessages
    };

    /**
     * @brief loop start communication loop when all participants are registerd.
     * Loop is exited once all participants deregistered from it.
     * @return 0 on success (always?).
     */
    virtual DispatchedMessagesResult dispatchMessages() = 0;

    //transmission methods
    /**
     * @brief sendTo direct transmission
     * @param srcAddr
     * @param data
     * @param size
     * @param destAddr
     * @return
     */
    virtual int sendTo(int srcAddr, uint8_t data[], int size, int destAddr) = 0;
    /**
     * @brief send broadcast transmission
     * @param srcAddr
     * @param data
     * @param size
     * @return
     */
    virtual int send(int srcAddr, uint8_t data[], int size) = 0;

    virtual bool registerParticipant(MediumParticipantImpl *participant) = 0;
    virtual bool deregisterParticipant(MediumParticipantImpl *participant) = 0;

    virtual bool isParticipantReachable(int address) = 0;
    virtual bool containsParticipant(int address) = 0;

protected:
    enum {
        MD_BroadcastAddress = -1
    };

public:
    bool isBcastAddress(int address);
    int bcastAddress();

};

#endif // MEDIUMDISPATCHER_H
