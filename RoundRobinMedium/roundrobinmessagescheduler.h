#ifndef ROUNDROBINMESSAGESCHEDULER_H
#define ROUNDROBINMESSAGESCHEDULER_H

#include <map>
#include <queue>
#include <stdint.h>

class RoundRobinMediumParticipant;
class ParticipantData;
class MessageData;

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
    //de/registration functions should be called out of exec()
    //thewise messages counter gets lost (and it's not in a scope of the project to correct it).
    bool registerParticipant(RoundRobinMediumParticipant *participant);
    bool deregisterParticipant(RoundRobinMediumParticipant *participant);

    //transmission methods
    int sendTo(int srcAddr, uint8_t data[], int size, int destAddr);
    int send(int srcAddr, uint8_t data[], int size);

    friend class RoundRobinMediumParticipant;

private:
    std::map<int, ParticipantData*> _participants;
    std::queue<MessageData*> _bcastMsgs;

    int _outStandingMsgsCnt;
    void _receiveMesage(RoundRobinMediumParticipant *participant, MessageData *md);
};

#endif // ROUNDROBINMESSAGESCHEDULER_H
