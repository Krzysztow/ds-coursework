#include "roundrobinmessagescheduler.h"

#include <memory>
#include <queue>
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

RoundRobinMessageScheduler::RoundRobinMessageScheduler()
{
}

int RoundRobinMessageScheduler::exec()
{
    std::map<int, ParticipantData*>::iterator currentParticipantIt;
    while (0 != _participants.size()) {
        currentParticipantIt = _participants.begin();
        for (; _participants.end() != currentParticipantIt; ++currentParticipantIt) {
            ParticipantData* pd = currentParticipantIt->second;

            if (0 != pd->messages.size()) {
                MessageData *md = pd->messages.front();
                pd->messages.pop();
                pd->participant->receive(md->senderAddress(), md->data(), md->dataSize());
                delete md;
            }
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

    return size;
}

int RoundRobinMessageScheduler::send(int srcAddr, uint8_t data[], int size)
{
    std::map<int, ParticipantData*>::iterator destIt = _participants.begin();

    if (_participants.end() == destIt)
        return 0;

    MessageData *md = new MessageData(srcAddr, data, size);
    for (;;) {
        ParticipantData *pd = destIt->second;
        ++destIt;
        if (_participants.end() == destIt) {
            pd->messages.push(md);
            break;
        }
        else {
            pd->messages.push(new MessageData(*md));
        }
    };

    return size;
}

