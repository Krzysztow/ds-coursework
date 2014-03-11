#include "mutexhandler.h"

#include <assert.h>

#include "applicationmessages.h"
#include "klogger.h"

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
    MutexResourceData(MutexHandler::MutexState state, int awaitingConfirmationsNo, LamportClock::LamportClockType requestTime):
        _state(state),
        _awaitingConfirmations(awaitingConfirmationsNo),
        _reqTime(requestTime)
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

    LamportClock::LamportClockType reqTime() {
        return _reqTime;
    }

private:
    std::list<MutexRequest*> _queuedRequests;
    MutexHandler::MutexState _state;

    int _awaitingConfirmations;
    LamportClock::LamportClockType _reqTime;
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
        AppMessages::MutexMsg msg;
        msg.type = AppMessages::AppMsgMutex;
        msg.isAcquire = true;
        msg.resourceId = resourceId;

        _mediumAccess->send((uint8_t*)&msg, sizeof(msg));
        //be sure to create MutexResourceData after send - lamport clock is incremented
        _mutexesData[resourceId] = new MutexResourceData(MutexHandler::Wanted, _otherProcsNo, _clock->currValue());
    }
    else {
        assert(false);
    }
}

#include <iostream>
void MutexHandler::release(int resourceId)
{
    MutexesData::iterator mdIt = _mutexesData.find(resourceId);
    if (_mutexesData.end() != mdIt) {
        MutexResourceData *mrd = mdIt->second;
        if (Held == mrd->state()) {
            _mutexesData.erase(mdIt);

            klogger(klogger::Info) << "releasing mutex - informing " << mrd->queuedReqs().size() << " processes" << klogger::end();
            //std::list<MutexRequest*> &queuedMutxReqs = mrd->queuedReqs();
            while (! mrd->queuedReqs().empty()) {
                MutexRequest *mutReq = mrd->queuedReqs().front();
                mrd->queuedReqs().pop_front();

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
    AppMessages::AppMessage msg;
    msg.mutexMsg.type = AppMessages::AppMsgMutex;
    msg.mutexMsg.isAcquire = isAcquire;
    msg.mutexMsg.resourceId = resourceId;
    msg.mutexMsg.procId = _procId;

    //we still have some spare place after the mutex message
    //!note: don't change msg fro, AppMessage to MutexMessage -> we need spare space to append lamport clock
    _mediumAccess->sendTo((uint8_t*)&msg, sizeof(AppMessages::MutexMsg), destAddr);
}

void MutexHandler::handleMessage(int srcAddress, AppMessages::MutexMsg *mutexMsg, LamportClock::LamportClockType msgClock)
{
    MutexesData::iterator mdIt = _mutexesData.find(mutexMsg->resourceId);
    if (_mutexesData.end() == mdIt) {//not acquired by us, neither we want it
        if (mutexMsg->isAcquire) {
            _sendMutexMsg(srcAddress, false, mutexMsg->resourceId);
        }
        else {
            assert(false);
        }
    }
    else {
        //either we want it or have it
        MutexResourceData *mrd = mdIt->second;
        if (mutexMsg->isAcquire) {//it's a request for mutex
            if (MutexHandler::Wanted == mrd->state()) {//we want mutex
                if (-1 == LamportClock::compareClocks(mrd->reqTime(), _procId, msgClock, srcAddress)) {//we requested earlier
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
