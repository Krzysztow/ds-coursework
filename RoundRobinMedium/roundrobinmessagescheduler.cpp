#include "roundrobinmessagescheduler.h"

#include <assert.h>

#include "RoundRobinMedium/roundrobinmediumparticipant.h"
#include "RoundRobinMedium/mediummessage.h"

enum {
    RRMS_BroadcastAddress = -1
};

struct ParticipantData {
    ParticipantData(RoundRobinMediumParticipant *participant):
        participant(participant) {}

    RoundRobinMediumParticipant *participant;
};

RoundRobinMessageScheduler::RoundRobinMessageScheduler()
{
}

void RoundRobinMessageScheduler::_receiveMesage(RoundRobinMediumParticipant *participant, MediumMessage *md)
{
    participant->receive(md->senderAddress(), md->data(), md->dataSize());
}

int RoundRobinMessageScheduler::exec()
{
    std::map<int, ParticipantData*>::iterator currentParticipantIt;

    bool lastLoopHadMessage = true;
    //loop until there are some messages outstanding
    while (lastLoopHadMessage) {
        lastLoopHadMessage = false;

        currentParticipantIt = _participants.begin();

        //pass pending messages to specific participants
        for (; _participants.end() != currentParticipantIt; ++currentParticipantIt) {
            ParticipantData* pd = currentParticipantIt->second;
            MediumMessage *md = pd->participant->popMessage();

            if (0 != md) {
                lastLoopHadMessage = true;

                if (RRMS_BroadcastAddress == md->receiverAddress()) {
                    //dispatch to all but sender
                    std::map<int, ParticipantData*>::iterator destPartIt = _participants.begin();
                    for (; _participants.end() != destPartIt; ++destPartIt) {
                        if (destPartIt == currentParticipantIt)//we don't send bcast message to ourselves
                            continue;

                        ParticipantData *pd = destPartIt->second;
                        _receiveMesage(pd->participant, md);
                    }
                }
                else {
                    //dispatch to the given receiver
                    std::map<int, ParticipantData*>::iterator destIt = _participants.find(md->receiverAddress());
                    if (_participants.end() == destIt)
                        return 0;

                    ParticipantData *pd = destIt->second;
                    _receiveMesage(pd->participant, md);
                }

                delete md;
            }

        }
    }

    return 0;
}

bool RoundRobinMessageScheduler::registerParticipant(RoundRobinMediumParticipant *participant)
{
    //dont register for bcast address
    if (RRMS_BroadcastAddress == participant->mediumAddress())
        return false;

    //is the address already assigned?
    if (0 != _participants.count(participant->mediumAddress()))
        return false;

    _participants[participant->mediumAddress()] = new ParticipantData(participant);

    return true;
}

bool RoundRobinMessageScheduler::deregisterParticipant(RoundRobinMediumParticipant *participant)
{
    return (1 == _participants.erase(participant->mediumAddress()));
}

bool RoundRobinMessageScheduler::isParticipantReachable(int address)
{
    return (0 != _participants.count(address));
}

bool RoundRobinMessageScheduler::containsParticipant(int address)
{
    return (0 != _participants.count(address));
}

bool RoundRobinMessageScheduler::isBcastAddress(int address)
{
    return (RRMS_BroadcastAddress == address);
}

int RoundRobinMessageScheduler::bcastAddress()
{
    return RRMS_BroadcastAddress;
}

int RoundRobinMessageScheduler::_send(int srcAddr, uint8_t data[], int size, int destAddr)
{
    //just make sure there is such a destination participant (or broadcast)
    if (RRMS_BroadcastAddress != destAddr) {
        std::map<int, ParticipantData*>::iterator destIt = _participants.find(destAddr);
        if (_participants.end() == destIt)//no destination - so there would be no connection, fail
            return -2;
    }

    //is sender registered with medium?
    std::map<int, ParticipantData*>::iterator srcIt = _participants.find(srcAddr);
    if (_participants.end() == srcIt)
        return -3;

    ParticipantData *pd = srcIt->second;

    MediumMessage *md = new MediumMessage(srcAddr, destAddr, data, size);
    pd->participant->_messages.push(md);

    return size;
}

int RoundRobinMessageScheduler::sendTo(int srcAddr, uint8_t data[], int size, int destAddr)
{
    assert(RRMS_BroadcastAddress != destAddr);
    if (RRMS_BroadcastAddress == destAddr)//for broadcasts use send() method
        return -1;

    return _send(srcAddr, data, size, destAddr);
}

int RoundRobinMessageScheduler::send(int srcAddr, uint8_t data[], int size)
{
    return _send(srcAddr, data, size, RRMS_BroadcastAddress);
}
