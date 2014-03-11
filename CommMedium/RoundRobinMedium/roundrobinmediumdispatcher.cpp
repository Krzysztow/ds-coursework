#include "roundrobinmediumdispatcher.h"

#include <assert.h>

#include "klogger.h"
#include "mediumparticipantimpl.h"
#include "mediummessage.h"

struct ParticipantData {
    ParticipantData(MediumParticipantImpl *participant):
        participant(participant) {}

    MediumParticipantImpl *participant;
};

RoundRobinMediumDispatcher::RoundRobinMediumDispatcher()
{
}

void RoundRobinMediumDispatcher::_receiveMesage(MediumParticipantImpl *participant, MediumMessage *md)
{
    participant->receive(md->senderAddress(), md->data(), md->dataSize());
}

MediumDispatcher::DispatchedMessagesResult RoundRobinMediumDispatcher::dispatchMessages()
{
    klogger(klogger::Info) << "-- Dispatcher loop starts" << klogger::end();
    std::map<int, ParticipantData*>::iterator currentParticipantIt;

    currentParticipantIt = _participants.begin();

    bool moreMessagesToSend = false;

    //pass pending messages to specific participants
    for (; _participants.end() != currentParticipantIt; ++currentParticipantIt) {
        ParticipantData* pd = currentParticipantIt->second;
        MediumMessage *md = pd->participant->popMessage();

        if (0 != md) {
            moreMessagesToSend = true;
            if (MD_BroadcastAddress == md->receiverAddress()) {
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
                    continue;

                ParticipantData *pd = destIt->second;
                _receiveMesage(pd->participant, md);
            }

            delete md;
        }
    }

    klogger(klogger::Info) << "-- Dispatcher loop ends" << klogger::end();
    return (moreMessagesToSend ? MD_NotDone : MD_NoMoreMessages);
}

bool RoundRobinMediumDispatcher::registerParticipant(MediumParticipantImpl *participant)
{
    //dont register for bcast address
    if (MD_BroadcastAddress == participant->mediumAddress())
        return false;

    //is the address already assigned?
    if (0 != _participants.count(participant->mediumAddress()))
        return false;

    _participants[participant->mediumAddress()] = new ParticipantData(participant);

    return true;
}

bool RoundRobinMediumDispatcher::deregisterParticipant(MediumParticipantImpl *participant)
{
    return (1 == _participants.erase(participant->mediumAddress()));
}

bool RoundRobinMediumDispatcher::isParticipantReachable(int address)
{
    return (0 != _participants.count(address));
}

bool RoundRobinMediumDispatcher::containsParticipant(int address)
{
    return (0 != _participants.count(address));
}

int RoundRobinMediumDispatcher::_send(int srcAddr, uint8_t data[], int size, int destAddr)
{
    //just make sure there is such a destination participant (or broadcast)
    if (MD_BroadcastAddress != destAddr) {
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

int RoundRobinMediumDispatcher::sendTo(int srcAddr, uint8_t data[], int size, int destAddr)
{
    assert(MD_BroadcastAddress != destAddr);
    if (MD_BroadcastAddress == destAddr)//for broadcasts use send() method
        return -1;

    return _send(srcAddr, data, size, destAddr);
}

int RoundRobinMediumDispatcher::send(int srcAddr, uint8_t data[], int size)
{
    return _send(srcAddr, data, size, MD_BroadcastAddress);
}
