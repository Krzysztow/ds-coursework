#include "processobject.h"

#include <cstring>
#include <iostream>

#include "applicationmessages.h"
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
    ReceivedMessageData(int srcAddress, TransmissionAppMsg *transMsg):
        src(srcAddress),
        size(transMsg->dataLength)
    {
        data = new uint8_t[size];
        memcpy(data, (uint8_t*)(transMsg->data), size);
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

private:
    ProcessObject *_procObj;
};

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

ScheduledObject::StepResult ProcessObject::execStep()
{
    if (0 == _opPlan.size()) {
        _buildPlan();
    }

    if (0 == _opPlan.size())
        return ScheduledObject::MayFinish;

    OperationAction *action = _opPlan.front();
    switch (action->type()) {
    case (OperationAction::OA_Send): {
        TransmissionAppMsg transMsg;
        SendOrReceiveOperationAction *sAction = dynamic_cast<SendOrReceiveOperationAction*>(action);
        assert(0 != sAction);
        assert(OperationAction::OA_Send == sAction->type());
        SendOrRecvOperation *sendOp = sAction->operation();
        transMsg.dataLength = sendOp->message().size();
        if (transMsg.dataLength <= APP_MSG_MAX_DATA_LENGTH) {
            memcpy(transMsg.data, sendOp->message().c_str(), transMsg.dataLength);
            _medAccess->sendTo((uint8_t*)&transMsg, sizeof(transMsg) + transMsg.dataLength, sendOp->destOrSrcProcId());
        }
        else {
            std::cerr << "Send operation error, data too long" << std::endl;
        }

        delete action;
        _opPlan.pop_front();
    }
        break;
    case (OperationAction::OA_Receive): {
        if (0 != _rcvdMessages.size()) {
            SendOrReceiveOperationAction *rcvAction = dynamic_cast<SendOrReceiveOperationAction*>(action);
            assert(0 != rcvAction);
            assert(OperationAction::OA_Receive == rcvAction->type());
            ReceivedMessageData *msgData = _rcvdMessages.front();
            AppMessage *msg = (AppMessage*)msgData->data;
            assert(AppMsgTrans == msg->type);
            if (rcvAction->operation()->destOrSrcProcId() == msgData->src) {
                std::string message((const char*)msg->printMsg.data, msg->printMsg.dataLength);
                _opPlan.push_back(new PrintOperationAction(message));

                _rcvdMessages.pop_front();
                delete msgData;
            }
            else {
                //is this even possible for recv to be called in different order than send?
                std::cerr << "Unexpected sender" << std::endl;
            }
        }
    }
        break;
    case (OperationAction::OA_MutexAcquire): {

    }
        break;
    case (OperationAction::OA_MutexRelease): {

    }
        break;
    case (OperationAction::OA_Print): {

        break;
    }
    default:
        assert(false);
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
        return _medAccess->sendTo(data, size, destAddress);
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
        return _medAccess->send(data, size);
    }
}

void ProcessObject::_receive(int srcAddress, AppMessage *appMsg, int size, LamportClock::LamportClockType msgClock)
{
    switch (appMsg->type) {
    case (AppMsgTrans): {
        _rcvdMessages.push_back(new ReceivedMessageData(srcAddress, &(appMsg->transMsg)));
    }
        break;
    case (AppMsgPrint): {
        assert(false);
    }
        break;
    case (AppMsgMutex): {
        _mutexHndlr.handleMessage(srcAddress, &(appMsg->mutexMsg), msgClock);
    }
        break;
    default:
        std::cerr << "Received unexpected message " << appMsg->type;
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
        AppMessage *appMsg = (AppMessage*)data;
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
        _opPlan.push_back(new OperationAction(OperationAction::OA_MutexAcquire));
        _opPlan.push_back(new OperationAction(OperationAction::OA_MutexAcquire));
        _opPlan.push_back(new OperationAction(OperationAction::OA_MutexAcquire));
    }
        break;
    case (Operation::OT_BeginMutex): {
        _opPlan.push_back(new OperationAction(OperationAction::OA_MutexAcquire));
        op = *_operIt;
        while (Operation::OT_Print == op->type()) {
            PrintOperation *pOp = dynamic_cast<PrintOperation*>(op);
            assert(0 != pOp);
            _opPlan.push_back(new PrintOperationAction(pOp));
            op = *(++_operIt);
        }
        assert(Operation::OT_EndMutex == op->type());
        _opPlan.push_back(new OperationAction(OperationAction::OA_MutexRelease));
    }
        break;
    case (Operation::OT_EndMutex):
        std::cerr << "That should never happen!" << std::endl;
        assert(false);
        break;
    default:
        assert(false);
    }
}
