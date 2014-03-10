#include "processobject.h"

#include <cstring>

#include "applicationmessages.h"
#include "klogger.h"
#include "lamportclockhandler.h"
#include "mediumparticipant.h"
#include "operation.h"
#include "processobject.h"

//-- BEGIN OperationAction --

class OperationAction {
public:
    enum Type {
        OA_Send,
        OA_Receive,
        OA_Print,
        OA_MutexAcquire,
        OA_MutexCheckHeld,
        OA_MutexRelease
    };

    OperationAction(OperationAction::Type type):
        _type(type)
    {}

    virtual ~OperationAction() {}

    OperationAction::Type type() {
        return _type;
    }

private:
    OperationAction::Type _type;
};

class SendOrReceiveOperationAction:
        public OperationAction
{
public:
    SendOrReceiveOperationAction(OperationAction::Type type, SendOrRecvOperation *op):
        OperationAction(type),
        _op(op)
    {
        assert(OperationAction::OA_Receive == this->type() ||
               OperationAction::OA_Send == this->type());
        assert(Operation::OT_Send == _op->type() ||
               Operation::OT_Recv == _op->type());
    }

    SendOrRecvOperation *operation() {
        return _op;
    }

private:
    SendOrRecvOperation *_op;
};

class PrintOperationAction:
        public OperationAction
{
public:
    PrintOperationAction(PrintOperation *op):
        OperationAction(OperationAction::OA_Print),
        _op(op)
    {}

    PrintOperationAction(const std::string &message):
        OperationAction(OperationAction::OA_Print),
        _op(0),
        _message(message)
    {}

    const std::string &message() {
        if (0 != _op)
            return _op->message();
        else
            return _message;
    }

private:
    PrintOperation *_op;
    std::string _message;
};

//-- END OperationAction --

class ReceivedMessageData {
public:
    ReceivedMessageData(int srcAddress, AppMessages::TransmissionMsg *transMsg, int size):
        src(srcAddress),
        size(size)
    {
        data = new uint8_t[size];
        memcpy(data, (uint8_t*)(transMsg), size);
    }

    ~ReceivedMessageData() {
        delete []data;
    }

public:
    int src;
    uint8_t *data;
    int size;
};

class MutexMediumParticipant:
        public MediumParticipant{
public:
    MutexMediumParticipant(ProcessObject *procObj):
        _procObj(procObj)
    {}

    virtual int send(uint8_t data[], int size) {
        return _procObj->_send(data, size);
    }

    virtual int sendTo(uint8_t data[], int size, int destAddr) {
        return _procObj->_sendTo(data, size, destAddr);
    }

    virtual void registerReceiver(MessageReceiver *receiver) {
        (void)receiver;
        assert(false);
    }

    virtual int mediumAddress() {
        return -1;
    }

private:
    ProcessObject *_procObj;
};

static const int PrinterMutexResourceIdentifier = 0;

ProcessObject::ProcessObject(MediumParticipant *mediumParticipant, int totalProcessesNo, int procId):
    _medAccess(mediumParticipant),
    _mutexMedAccess(new MutexMediumParticipant(this)),
    _mutexHndlr(_mutexMedAccess, procId, totalProcessesNo, &_clock),
    _clockHandler(&_clock)
{
    _medAccess->registerReceiver(this);
}

ProcessObject::~ProcessObject()
{
    delete _mutexMedAccess;
}

void ProcessObject::setOperations(const std::list<Operation *> *operations)
{
    _operations = operations;
    _operIt = _operations->begin();
}

ReceivedMessageData *ProcessObject::_takeReceivedMessage(unsigned int srcProcessId) {
    std::list<ReceivedMessageData*>::iterator msgsIt = _rcvdMessages.begin();
    for (; _rcvdMessages.end() != msgsIt; ++msgsIt) {
        if ((*msgsIt)->src == srcProcessId) {
            ReceivedMessageData *msgData = (*msgsIt);
            _rcvdMessages.erase(msgsIt, msgsIt);
            return msgData;
        }
    }

    return 0;
}

ScheduledObject::StepResult ProcessObject::execStep()
{
    if (0 == _opPlan.size()) {
        _buildPlan();
    }

    if (0 == _opPlan.size())
        return ScheduledObject::MayFinish;

    bool removeOpAction = false;
    OperationAction *action = _opPlan.front();
    switch (action->type()) {
    case (OperationAction::OA_Send): {
        AppMessages::TransmissionMsg transMsg;
        transMsg.header.type = AppMessages::AppMsgTrans;
        SendOrReceiveOperationAction *sAction = dynamic_cast<SendOrReceiveOperationAction*>(action);
        assert(0 != sAction);
        assert(OperationAction::OA_Send == sAction->type());
        SendOrRecvOperation *sendOp = sAction->operation();
        transMsg.header.dataLength = sendOp->message().size();
        if (transMsg.header.dataLength <= APP_MSG_MAX_DATA_LENGTH) {
            memcpy(transMsg.data, sendOp->message().c_str(), transMsg.header.dataLength);
            klogger() << "sent p" << _medAccess->mediumAddress() << " " << sendOp->message() << " p" << sendOp->destOrSrcProcId() << " " << _clock.currValue() << klogger::end();
            _sendTo((uint8_t*)&transMsg, sizeof(transMsg.header) + transMsg.header.dataLength, sendOp->destOrSrcProcId());
        }
        else {
            klogger(klogger::Errors) << "Send operation error, data too long" << klogger::end();
        }

        removeOpAction = true;
    }
        break;
    case (OperationAction::OA_Receive): {
        if (0 != _rcvdMessages.size()) {
            SendOrReceiveOperationAction *rcvAction = dynamic_cast<SendOrReceiveOperationAction*>(action);
            assert(0 != rcvAction);
            assert(OperationAction::OA_Receive == rcvAction->type());
            ReceivedMessageData *msgData = _takeReceivedMessage(rcvAction->operation()->destOrSrcProcId());
            if (0 != msgData) {
                AppMessages::AppMessage *msg = (AppMessages::AppMessage*)msgData->data;
                assert(AppMessages::AppMsgTrans == msg->type);
                std::string message((const char*)msg->printMsg.data, msg->printMsg.header.dataLength);
                _opPlan.push_back(new PrintOperationAction(message));

                klogger() << "received p" << _medAccess->mediumAddress() << " " << message << " p" << msgData->src << " " << _clock.currValue() << klogger::end();

                _rcvdMessages.pop_front();
                delete msgData;

                removeOpAction = true;
            }
            else {
                //is this even possible for recv to be called in different order than send?
                klogger(klogger::Errors) << "None message from " << rcvAction->operation()->destOrSrcProcId() << " for " << _medAccess->mediumAddress() << klogger::end();
            }
        }
        else {
            SendOrReceiveOperationAction *rcvAction = dynamic_cast<SendOrReceiveOperationAction*>(action);
            klogger(klogger::Info) << "Process p" << _medAccess->mediumAddress() << " waiting for msg from p" << rcvAction->operation()->destOrSrcProcId() << klogger::end();
        }
    }
        break;
    case (OperationAction::OA_MutexAcquire): {
        _mutexHndlr.acquire(PrinterMutexResourceIdentifier);

        removeOpAction = true;
    }
        break;
    case (OperationAction::OA_MutexCheckHeld): {
        if (MutexHandler::Held == _mutexHndlr.state(PrinterMutexResourceIdentifier)) {
            removeOpAction = true;
        }
        else {
            klogger(klogger::Info) << "Mutex still not held for " << _medAccess->mediumAddress() << " (state: " << _mutexHndlr.state(PrinterMutexResourceIdentifier) << ")" << klogger::end();
        }
    }
        break;
    case (OperationAction::OA_MutexRelease): {
        if (MutexHandler::Held == _mutexHndlr.state(PrinterMutexResourceIdentifier)) {
            _mutexHndlr.release(PrinterMutexResourceIdentifier);

            removeOpAction = true;
        }
        else {
            assert(false);
        }
    }
        break;
    case (OperationAction::OA_Print): {
        //other operation actions are realised with messages, thus clock is updated. Here it's not a case -> do it manually.
        _clock.eventOccured();
        //being here, means we had checked for mutex being held
        PrintOperationAction *pAction = dynamic_cast<PrintOperationAction*>(action);
        assert(0 != pAction);
        klogger() << "printed p" << _medAccess->mediumAddress() << " " << pAction->message() << " " << _clock.currValue() << klogger::end();
        removeOpAction = true;

        break;
    }
    default:
        assert(false);
    }

    if (removeOpAction) {
        delete action;
        _opPlan.pop_front();
    }

    return ScheduledObject::NotFinished;
}

#include <cstring>

int ProcessObject::_sendTo(uint8_t *data, int size, int destAddress)
{
    int ret = _clockHandler.appendClockToMsg(data, size, APP_MSG_MAX_DATA_LENGTH);
    if (ret < 0) {
        assert(false);
        return -1;
    }
    else {
        size += ret;
        ret = _medAccess->sendTo(data, size, destAddress);

        return ret;
    }
}

int ProcessObject::_send(uint8_t *data, int size)
{
    int ret = _clockHandler.appendClockToMsg(data, size, APP_MSG_MAX_DATA_LENGTH);
    if (ret < 0) {
        assert(false);
        return -1;
    }
    else {
        size += ret;
        ret = _medAccess->send(data, size);
        return ret;
    }
}

void ProcessObject::_receive(int srcAddress, AppMessages::AppMessage *appMsg, int size, LamportClock::LamportClockType msgClock)
{
    switch (appMsg->type) {
    case (AppMessages::AppMsgTrans): {
        _rcvdMessages.push_back(new ReceivedMessageData(srcAddress, &(appMsg->transMsg), size));
    }
        break;
    case (AppMessages::AppMsgPrint): {
        assert(false);
    }
        break;
    case (AppMessages::AppMsgMutex): {
        _mutexHndlr.handleMessage(srcAddress, &(appMsg->mutexMsg), msgClock);
    }
        break;
    default:
        klogger(klogger::Errors) << "Received unexpected message " << appMsg->type;
        assert(false);
    }
}

void ProcessObject::receive(int srcAddress, uint8_t data[], int size)
{
    //lamport clock is piggy-backed to the message
    LamportClock::LamportClockType msgClock;
    int ret = _clockHandler.removeClockFromMsg(data, size, &msgClock);
    if (ret < 0) {
        assert(false);
    }
    else {
        AppMessages::AppMessage *appMsg = (AppMessages::AppMessage*)data;
        _receive(srcAddress, appMsg, size - ret, msgClock);
    }
}

void ProcessObject::_buildPlan()
{
    assert(0 == _opPlan.size());
    if (_operations->end() == _operIt)
        return;

    Operation *op = *_operIt;
    ++_operIt;

    switch (op->type()) {
    case (Operation::OT_Send)://fall through
    case (Operation::OT_Recv): {
        SendOrRecvOperation *srOp = dynamic_cast<SendOrRecvOperation*>(op);
        assert(0 != srOp);
        _opPlan.push_back(new SendOrReceiveOperationAction(Operation::OT_Send == srOp->type() ?
                                                               OperationAction::OA_Send : OperationAction::OA_Receive, srOp));
    }
        break;
    case (Operation::OT_Print): {//print out of mutex block
        PrintOperation *pOp = dynamic_cast<PrintOperation*>(op);
        assert(0 != pOp);
        _opPlan.push_back(new OperationAction(OperationAction::OA_MutexAcquire));
        _opPlan.push_back(new OperationAction(OperationAction::OA_MutexCheckHeld));
        _opPlan.push_back(new PrintOperationAction(pOp->message()));
        _opPlan.push_back(new OperationAction(OperationAction::OA_MutexRelease));
    }
        break;
    case (Operation::OT_BeginMutex): {
        _opPlan.push_back(new OperationAction(OperationAction::OA_MutexAcquire));
        _opPlan.push_back(new OperationAction(OperationAction::OA_MutexCheckHeld));
        op = *_operIt;
        while (Operation::OT_Print == op->type()) {
            PrintOperation *pOp = dynamic_cast<PrintOperation*>(op);
            assert(0 != pOp);
            _opPlan.push_back(new PrintOperationAction(pOp));
            op = *(++_operIt);
        }
        assert(Operation::OT_EndMutex == op->type());
        ++_operIt;
        _opPlan.push_back(new OperationAction(OperationAction::OA_MutexRelease));
    }
        break;
    case (Operation::OT_EndMutex):
        klogger(klogger::Errors) << "That should never happen!" << klogger::end();
        assert(false);
        break;
    default:
        assert(false);
    }
}
