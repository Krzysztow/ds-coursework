#include "roundrobinmessagescheduler.h"

#include <memory>
#include <cstring>

#include "RoundRobinMedium/roundrobinmediumparticipant.h"

//smart pointers would be better
class MessageData {
public:
    MessageData(int sender, uint8_t data[], int size):
        size(size),
        sender(sender)
    {
        buffer = new uint8_t[size * sizeof(data)];
        memcpy(buffer, data, size * sizeof(data));
        refsCnt = new int(1);
    }

    MessageData(const MessageData &other) {
        buffer = other.buffer;
        size = other.size;
        refsCnt = other.refsCnt;
        ++(*refsCnt);
    }

    ~MessageData() {
        if (0 == --(*refsCnt) ) {
            delete refsCnt;
            delete []buffer;
        }
    }

    uint8_t *data() {
        return buffer;
    }

    int dataSize() {
        return size;
    }

    int senderAddress() {
        return this->sender;
    }

private:
    int size;
    int sender;

    int *refsCnt;
    uint8_t *buffer;
};

struct ParticipantData {
    ParticipantData(RoundRobinMediumParticipant *participant):
        participant(participant) {}

    RoundRobinMediumParticipant *participant;
    std::queue<MessageData*> messages;
};

RoundRobinMessageScheduler::RoundRobinMessageScheduler():
    _outStandingMsgsCnt(0)
{
}

void RoundRobinMessageScheduler::_receiveMesage(RoundRobinMediumParticipant *participant, MessageData *md)
{
    participant->receive(md->senderAddress(), md->data(), md->dataSize());
    --_outStandingMsgsCnt;
}

int RoundRobinMessageScheduler::exec()
{
    std::map<int, ParticipantData*>::iterator currentParticipantIt;

    //loop until there are some messages outstanding
    while (0 != _outStandingMsgsCnt) {
        currentParticipantIt = _participants.begin();

        //pass pending messages to specific participants
        for (; _participants.end() != currentParticipantIt; ++currentParticipantIt) {
            ParticipantData* pd = currentParticipantIt->second;

            if (0 != pd->messages.size()) {
                MessageData *md = pd->messages.front();
                pd->messages.pop();
                _receiveMesage(pd->participant, md);
                delete md;
            }

        }

        //distribute broadcast message
        if (0 != _bcastMsgs.size()) {
            MessageData *md = _bcastMsgs.front();
            _bcastMsgs.pop();

            currentParticipantIt = _participants.begin();
            for (; _participants.end() != currentParticipantIt; ++currentParticipantIt) {
                if (md->senderAddress() != currentParticipantIt->first) {
                    _receiveMesage(currentParticipantIt->second->participant, md);
                }
            }

            delete md;
        }
    }

    return 0;
}

bool RoundRobinMessageScheduler::registerParticipant(RoundRobinMediumParticipant *participant)
{
    if (0 != _participants.count(participant->mediumAddress()))
        return false;

    _participants[participant->mediumAddress()] = new ParticipantData(participant);

    return true;
}

bool RoundRobinMessageScheduler::deregisterParticipant(RoundRobinMediumParticipant *participant)
{
    return (1 == _participants.erase(participant->mediumAddress()));
}

int RoundRobinMessageScheduler::sendTo(int srcAddr, uint8_t data[], int size, int destAddr)
{
    std::map<int, ParticipantData*>::iterator destIt = _participants.find(destAddr);
    if (_participants.end() == destIt)
        return 0;

    ParticipantData *pd = destIt->second;

    MessageData *md = new MessageData(srcAddr, data, size);
    pd->messages.push(md);
    ++_outStandingMsgsCnt;

    return size;
}

int RoundRobinMessageScheduler::send(int srcAddr, uint8_t data[], int size)
{
    int msgsToSend = _participants.size();
    //if sender is in the participants, it will not get the message
    if (_participants.end() != _participants.find(srcAddr))
        --msgsToSend;

    if (0 != msgsToSend) {//make sure there is not only a sender registered
        _outStandingMsgsCnt += msgsToSend;
        _bcastMsgs.push(new MessageData(srcAddr, data, size));
        return size;
    }
    return 0;
}
