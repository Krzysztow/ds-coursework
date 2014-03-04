#ifndef ROUNDROBINMESSAGESCHEDULER_H
#define ROUNDROBINMESSAGESCHEDULER_H

#include <map>
#include <queue>
#include <stdint.h>

class RoundRobinMediumParticipant;
class ParticipantData;
class MediumMessage;

class RoundRobinMessageScheduler
{
public:
    RoundRobinMessageScheduler();

    /**
     * @brief loop start communication loop when all participants are registerd.
     * Loop is exited once all participants deregistered from it.
     * @return 0 on success (always?).
     */
    int exec();

    bool registerParticipant(RoundRobinMediumParticipant *participant);
    bool deregisterParticipant(RoundRobinMediumParticipant *participant);

    bool isParticipantReachable(int address);
    bool containsParticipant(int address);
    bool isBcastAddress(int address);
    int bcastAddress();

    //transmission methods
    /**
     * @brief sendTo direct transmission
     * @param srcAddr
     * @param data
     * @param size
     * @param destAddr
     * @return
     */
    int sendTo(int srcAddr, uint8_t data[], int size, int destAddr);
    /**
     * @brief send broadcast transmission
     * @param srcAddr
     * @param data
     * @param size
     * @return
     */
    int send(int srcAddr, uint8_t data[], int size);

    friend class RoundRobinMediumParticipant;

private:
    std::map<int, ParticipantData*> _participants;

    void _receiveMesage(RoundRobinMediumParticipant *participant, MediumMessage *md);
    int _send(int srcAddr, uint8_t data[], int size, int destAddr);
};

#endif // ROUNDROBINMESSAGESCHEDULER_H
