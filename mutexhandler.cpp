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

MutexHandler::MutexHandler(MediumParticipant *medAccess, int processesId, int allProcsNo, LamportClock *clock, const std::string &dbgMsg):
    _mediumAccess(medAccess),
    _procId(processesId),
    _otherProcsNo(allProcsNo - 1),
    _clock(clock),
    _dbgMessage(dbgMsg)
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
        msg.requestType = AppMessages::MutexRequired;
        msg.resourceId = resourceId;

        klogger(klogger::Info) << _dbgMessage << " mutex acquisition trial" << klogger::end();

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

            klogger(klogger::Info) << _dbgMessage << " mutex release, inform " << mrd->queuedReqs().size() << " processes" << klogger::end();
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
    msg.mutexMsg.requestType = (isAcquire ? AppMessages::MutexRequired : AppMessages::MutexGranted);
    msg.mutexMsg.resourceId = resourceId;
    msg.mutexMsg.procId = _procId;

    //we still have some spare place after the mutex message
    //!note: don't change msg fro, AppMessage to MutexMessage -> we need spare space to append lamport clock
    _mediumAccess->sendTo((uint8_t*)&msg, sizeof(AppMessages::MutexMsg), destAddr);
}

void MutexHandler::handleMessage(int srcAddress, AppMessages::MutexMsg *mutexMsg, LamportClock::LamportClockType msgClock)
{
    MutexesData::iterator mdIt = _mutexesData.find(mutexMsg->resourceId);
    bool handlerIsInterested = (_mutexesData.end() != mdIt);//if we were interested with this resource up to now? (we are in wanted or held state)
    switch (mutexMsg->requestType) {
    case (AppMessages::MutexGranted): {
        if (handlerIsInterested) {//we are interested in mutex and someone granted it to us
            MutexResourceData *mrd = mdIt->second;
            if (mrd->confirmationReceived()) {
                mrd->setState(MutexHandler::Held);
                klogger(klogger::Info) << _dbgMessage << " mutex acquired" << klogger::end();
            }
        }
        else {//we are not interested, but someone granted it to us
            klogger(klogger::Errors) << _dbgMessage << " mutex allowance, although not asked for" << klogger::end();
            assert(false);
        }
    }
        break;
    case (AppMessages::MutexRequired):
        if (handlerIsInterested) {//we are interested, but someone else too
            MutexResourceData *mrd = mdIt->second;
            if (MutexHandler::Wanted == mrd->state()) {//we are still waiting for all permissions, thus need to check who was earlier
                if (-1 == LamportClock::compareClocks(mrd->reqTime(), _procId, msgClock, srcAddress)) {//we requested earlier, just queue
                    mrd->appendAwaitingReq(srcAddress, mutexMsg->procId);
                }
                else {//the other process requested earlier - grant it permission
                    _sendMutexMsg(srcAddress, false, mutexMsg->resourceId);
                }
            }
            else if (MutexHandler::Held == mrd->state()) {//we already hold mutex, queue
                mrd->appendAwaitingReq(srcAddress, mutexMsg->procId);
            }
            else//can't be interested and not in one of those states
                assert(false);
        }
        else {//we are not interested, but others are. Grant him permission.
            _sendMutexMsg(srcAddress, false, mutexMsg->resourceId);
        }
        break;
    default:
        klogger(klogger::Errors) << "Unknown mutex type " << mutexMsg->type << klogger::end();
        assert(false);
    }
}

void MutexHandler::setDebuggMessage(const std::string &dbgMsg)
{
    _dbgMessage = dbgMsg;
}
