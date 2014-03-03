#ifndef ROUNDROBINMESSAGESCHEDULER_H
#define ROUNDROBINMESSAGESCHEDULER_H

#include <map>
#include <stdint.h>

class RoundRobinMediumParticipant;
class ParticipantData;

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

protected:
    bool registerParticipant(RoundRobinMediumParticipant *participant);
    bool deregisterParticipant(RoundRobinMediumParticipant *participant);
    int sendTo(int srcAddr, uint8_t data[], int size, int destAddr);
    int send(int srcAddr, uint8_t data[], int size);
    friend class RoundRobinMediumParticipant;

private:
    std::map<int, ParticipantData*> _participants;
};

#endif // ROUNDROBINMESSAGESCHEDULER_H
