#include "processobject.h"

#include <cstring>
#include <sstream>

#include "applicationmessages.h"
#include "klogger.h"
#include "lamportclockhandler.h"
#include "mediumparticipant.h"
#include "operation.h"
#include "processobject.h"


static const int PrinterMutexResourceIdentifier = 0;
static const int PrinterMediumAddress = 255;

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

//-- BEGIN OperationAction --

class OperationAction {
public:
    OperationAction()
    {}

    virtual ~OperationAction() {}

    virtual bool doAction(ProcessObject *procObj) = 0;
};

class SendOperationAction:
        public OperationAction {
public:
    SendOperationAction(SendOrRecvOperation *op):
        _op(op) {
        assert(Operation::OT_Send == _op->type());
    }

    virtual bool doAction(ProcessObject *procObj) {
        AppMessages::TransmissionMsg transMsg;
        transMsg.header.type = AppMessages::AppMsgTrans;
        transMsg.header.dataLength = _op->message().size();
        if (transMsg.header.dataLength <= APP_MSG_MAX_DATA_LENGTH) {
            memcpy(transMsg.data, _op->message().c_str(), transMsg.header.dataLength);
            klogger() << "sent p" << procObj->_medAccess->mediumAddress() << " " << _op->message() << " p" << _op->destOrSrcProcId() << " " << procObj->_clock.currValue() << klogger::end();
            procObj->_sendTo((uint8_t*)&transMsg, sizeof(transMsg.header) + transMsg.header.dataLength, _op->destOrSrcProcId());
        }
        else {
            klogger(klogger::Errors) << "Send operation error, data too long" << klogger::end();
        }

        //we're done, let it be deleted
        return true;
    }

private:
    SendOrRecvOperation *_op;
};

class RecvOpeartionAction:
        public OperationAction {
public:
    RecvOpeartionAction(SendOrRecvOperation *op):
        _op(op) {
        assert(Operation::OT_Recv == _op->type());
    }

    virtual bool doAction(ProcessObject *procObj) {
        if (0 != procObj->_rcvdMessages.size()) {
            ReceivedMessageData *msgData = procObj->_takeReceivedMessage(_op->destOrSrcProcId());
            if (0 != msgData) {
                AppMessages::AppMessage *msg = (AppMessages::AppMessage*)msgData->data;
                assert(AppMessages::AppMsgTrans == msg->type);

                //we shouldn't send to print the received message
                std::string message((const char*)msg->printMsg.data, msg->printMsg.header.dataLength);
                //_opPlan.push_back(new PrintOperationAction(message));

                klogger() << "received p" << procObj->_medAccess->mediumAddress() << " " << message << " p" << msgData->src << " " << procObj->_clock.currValue() << klogger::end();

                procObj->_rcvdMessages.pop_front();
                delete msgData;

                //done, may be deleted
                return true;
            }
            else {
                //is this even possible for recv to be called in different order than send?
                klogger(klogger::Errors) << "None message from " << _op->destOrSrcProcId() << " for " << procObj->_medAccess->mediumAddress() << klogger::end();
            }
        }
        else {
            klogger(klogger::Info) << "Process p" << procObj->_medAccess->mediumAddress() << " waiting for msg from p" << _op->destOrSrcProcId() << klogger::end();
        }

        return false;
    }

private:
    SendOrRecvOperation *_op;
};

class AcquireMutexAction:
        public OperationAction
{
public:
    AcquireMutexAction():
        _state(NotHeld)
    {}

    virtual bool doAction(ProcessObject *procObj) {
        if (NotHeld == _state) {
            procObj->_mutexHndlr.acquire(PrinterMutexResourceIdentifier);
            _state = Waiting;
        }
        else {
            if (MutexHandler::Held == procObj->_mutexHndlr.state(PrinterMutexResourceIdentifier)) {
                return true;
            }
        }
        return false;
    }

private:
    enum AcquisitionState {
        NotHeld,
        Waiting
    };
    AcquisitionState _state;
};

class ReleaseMutexAction:
        public OperationAction
{
public:
    virtual bool doAction(ProcessObject *procObj) {
        if (MutexHandler::Held == procObj->_mutexHndlr.state(PrinterMutexResourceIdentifier)) {
            procObj->_mutexHndlr.release(PrinterMutexResourceIdentifier);

            return true;
        }
        else {
            assert(false);
        }

        return false;
    }
};

class PrintOperationAction:
        public OperationAction
{
public:
    PrintOperationAction(const std::string &message):
        _message(message)
    {}

    virtual bool doAction(ProcessObject *procObj) {
        //other operation actions are realised with messages, thus clock is updated. Here it's not a case -> do it manually.
        procObj->_clock.eventOccured();
        //being here, means we had checked for mutex being held
        std::stringstream s;
        s << "printed p" << procObj->_medAccess->mediumAddress() << " " << _message << " " << procObj->_clock.currValue();

        AppMessages::PrintMsg printMsg;
        printMsg.header.type = AppMessages::AppMsgPrint;
        //! todo: check if the string is not too long!
        std::string fullMsg = s.str();
        printMsg.header.dataLength = fullMsg.copy((char*)printMsg.data, fullMsg.size());

        procObj->_sendTo((uint8_t*)&printMsg, sizeof(AppMessages::PrintMsgHeader) + printMsg.header.dataLength, PrinterMediumAddress);

        return true;
    }

private:
    std::string _message;
};

//-- END OperationAction --

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

    OperationAction *action = _opPlan.front();
    if (action->doAction(this)) {
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
    case (Operation::OT_Send): {
        SendOrRecvOperation *sOp = dynamic_cast<SendOrRecvOperation*>(op);
        assert(0 != sOp);
        _opPlan.push_back(new SendOperationAction(sOp));
    }
        break;
    case (Operation::OT_Recv): {
        SendOrRecvOperation *rOp = dynamic_cast<SendOrRecvOperation*>(op);
        assert(0 != rOp);
        _opPlan.push_back(new RecvOpeartionAction(rOp));
    }
        break;
    case (Operation::OT_Print): {//print out of mutex block
        PrintOperation *pOp = dynamic_cast<PrintOperation*>(op);
        assert(0 != pOp);
        _opPlan.push_back(new AcquireMutexAction());
        _opPlan.push_back(new PrintOperationAction(pOp->message()));
        _opPlan.push_back(new ReleaseMutexAction());
    }
        break;
    case (Operation::OT_BeginMutex): {
        _opPlan.push_back(new AcquireMutexAction());
        op = *_operIt;
        while (Operation::OT_Print == op->type()) {
            PrintOperation *pOp = dynamic_cast<PrintOperation*>(op);
            assert(0 != pOp);
            _opPlan.push_back(new PrintOperationAction(pOp->message()));
            op = *(++_operIt);
        }
        assert(Operation::OT_EndMutex == op->type());
        ++_operIt;
        _opPlan.push_back(new ReleaseMutexAction());
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
