#include "mutexhandler.h"

#include <assert.h>

#include "applicationmessages.h"

class MutexRequest {
public:
    MutexRequest(int srcAddress, int procId):
        _srcAddr(srcAddress),
        _procId(procId)
    {}

    int srcAddress() {
        return _srcAddr;
    }

    int procId() {
        return _procId;
    }

private:
    int _srcAddr;
    int _procId;
};

class MutexResourceData {
public:
    MutexResourceData(MutexHandler::MutexState state, int awaitingConfirmationsNo):
        _state(state),
        _awaitingConfirmations(awaitingConfirmationsNo)
    {
    }

    MutexHandler::MutexState state() {
        return _state;
    }

    void setState(MutexHandler::MutexState state) {
        _state = state;
    }

    bool confirmationReceived() {
        return (0 == --_awaitingConfirmations);
    }

    void appendAwaitingReq(int srcAddress, int procId) {
        _queuedRequests.push_back(new MutexRequest(srcAddress, procId));
    }

    std::list<MutexRequest*> &queuedReqs() {
        return _queuedRequests;
    }

private:
    std::list<MutexRequest*> _queuedRequests;
    MutexHandler::MutexState _state;
    int _awaitingConfirmations;
};

MutexHandler::MutexHandler(MediumParticipant *medAccess, int processesId, int allProcsNo, LamportClock *clock):
    _mediumAccess(medAccess),
    _procId(processesId),
    _otherProcsNo(allProcsNo - 1),
    _clock(clock)
{
}

MutexHandler::MutexState MutexHandler::state(int resourceId)
{
    MutexesData::iterator f = _mutexesData.find(resourceId);
    if (_mutexesData.end() != f)
        return f->second->state();
    return MutexHandler::Released;
}

void MutexHandler::acquire(int resourceId)
{
    if (Released == state(resourceId)) {
        MutexAppMsg msg;
        msg.type = AppMsgMutex;
        msg.isAcquire = true;
        msg.resourceId = resourceId;

        _mediumAccess->send((uint8_t*)&msg, sizeof(msg));
        _mutexesData[resourceId] = new MutexResourceData(MutexHandler::Wanted, _otherProcsNo);
    }
    else {
        assert(false);
    }
}


void MutexHandler::release(int resourceId)
{
    MutexesData::iterator mdIt = _mutexesData.find(resourceId);
    if (_mutexesData.end() != mdIt) {
        MutexResourceData *mrd = mdIt->second;
        if (Held == mrd->state()) {
            _mutexesData.erase(mdIt);

            std::list<MutexRequest*> &queuedMutxReqs = mrd->queuedReqs();
            while (! queuedMutxReqs.empty()) {
                MutexRequest *mutReq = queuedMutxReqs.front();
                queuedMutxReqs.pop_front();

                _sendMutexMsg(mutReq->srcAddress(), false, resourceId);

                delete mutReq;
            }

            delete mrd;
        }
        else {
            assert(false);
        }
    }
    else {
        assert(false);
    }
}

//void MutexHandler::_sendBcastMutexMsg(bool isAcquire, int resourceId) {
//    MutexAppMsg msg;
//    msg.type = AppMsgMutex;
//    msg.isAcquire = isAcquire;
//    msg.resourceId = resourceId;

//    _mediumAccess->send((uint8_t*)&msg, sizeof(msg));
//}

void MutexHandler::_sendMutexMsg(int destAddr, bool isAcquire, int resourceId)
{
    MutexAppMsg msg;
    msg.type = AppMsgMutex;
    msg.isAcquire = isAcquire;
    msg.resourceId = resourceId;
    msg.procId = _procId;

    _mediumAccess->sendTo((uint8_t*)&msg, sizeof(msg), destAddr);
}

void MutexHandler::handleMessage(int srcAddress, MutexAppMsg *mutexMsg, LamportClock::LamportClockType msgClock)
{
    MutexesData::iterator mdIt = _mutexesData.find(mutexMsg->resourceId);
    if (_mutexesData.end() == mdIt) {//not acquired by us, neither we want it
        _sendMutexMsg(srcAddress, false, mutexMsg->resourceId);
    }
    else {
        //either we want it or have it
        MutexResourceData *mrd = mdIt->second;
        if (mutexMsg->isAcquire) {//it's a request for mutex
            if (MutexHandler::Wanted == mrd->state()) {//we want mutex
                if (-1 == LamportClock::compareClocks(_clock->currValue(), _procId, msgClock, srcAddress)) {//we requested earlier
                    //do nothing, just queue
                    mrd->appendAwaitingReq(srcAddress, mutexMsg->procId);
                }
                else {//the other process requested earlier
                    _sendMutexMsg(srcAddress, false, mutexMsg->resourceId);
                }
            }
            else if (MutexHandler::Held == mrd->state()) {//we hold mutex
                mrd->appendAwaitingReq(srcAddress, mutexMsg->procId);
            }
            else
                assert(false);
        }
        else {//it's an allowance for mutex
            if (MutexHandler::Wanted == mrd->state()) {//we indeed wanted it
                if (mrd->confirmationReceived()) {
                    mrd->setState(MutexHandler::Held);
                }
            }
            else {//anything else, shouldn't happen
                assert(false);
            }
        }
    }
}
