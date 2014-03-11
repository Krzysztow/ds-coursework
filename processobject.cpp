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

class ReceivedMessageData;
class OperationAction;
class MutexMediumParticipant;

class SendOperationAction;
class RecvOpeartionAction;
class AcquireMutexAction;
class ReleaseMutexAction;
class PrintOperationAction;

class ProcessObjectPrivate {
public:
    ProcessObjectPrivate(MediumParticipant *mediumParticipant, int totalProcessesNo, int procId);
    ~ProcessObjectPrivate();

    void _buildPlan(const std::list<Operation *> *operations);
    int _sendTo(uint8_t *data, int size, int destAddress);
    int _send(uint8_t *data, int size);
    void _receive(int srcAddress, AppMessages::AppMessage *appMsg, int size, LamportClock::LamportClockType msgClock);
    ReceivedMessageData *_takeReceivedMessage(unsigned int srcProcessId);

    bool doAction(SendOperationAction *action);
    bool doAction(RecvOpeartionAction *action);
    bool doAction(AcquireMutexAction *action);
    bool doAction(ReleaseMutexAction *action);
    bool doAction(PrintOperationAction *action);

public:
    MediumParticipant *medAccess;

    std::list<OperationAction *> opPlan;
    std::list<ReceivedMessageData*> rcvdMessages;

    MutexMediumParticipant *mutexMedAccess;
    friend class MutexMediumParticipant;
    MutexHandler mutexHndlr;

    LamportClock clock;
    LamportClockHandler clockHandler;
};

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
    virtual ~OperationAction() {}
    virtual bool doAction(ProcessObjectPrivate *procObjPriv) = 0;
};

class SendOperationAction:
        public OperationAction {
public:
    SendOperationAction(SendOrRecvOperation *op):
        operation(op) {
        assert(Operation::OT_Send == operation->type());
    }

    virtual bool doAction(ProcessObjectPrivate *procObjPriv) {
        return procObjPriv->doAction(this);
    }

public:
    SendOrRecvOperation *operation;
};

class RecvOpeartionAction:
        public OperationAction {
public:
    RecvOpeartionAction(SendOrRecvOperation *op):
        operation(op) {
        assert(Operation::OT_Recv == operation->type());
    }

    virtual bool doAction(ProcessObjectPrivate *procObjPriv) {
        return procObjPriv->doAction(this);
    }

public:
    SendOrRecvOperation *operation;
};

class AcquireMutexAction:
        public OperationAction
{
public:
    virtual bool doAction(ProcessObjectPrivate *procObjPriv) {
        return procObjPriv->doAction(this);
    }
};

class ReleaseMutexAction:
        public OperationAction
{
public:
    virtual bool doAction(ProcessObjectPrivate *procObjPriv) {
        return procObjPriv->doAction(this);
    }
};

class PrintOperationAction:
        public OperationAction
{
public:
    PrintOperationAction(const std::string &message):
        message(message)
    {}

    virtual bool doAction(ProcessObjectPrivate *procObjPriv) {
        return procObjPriv->doAction(this);
    }

public:
    std::string message;
};

//-- END OperationAction --


bool ProcessObjectPrivate::doAction(SendOperationAction *action) {
    AppMessages::TransmissionMsg transMsg;
    transMsg.header.type = AppMessages::AppMsgTrans;
    transMsg.header.dataLength = action->operation->message().size();
    if (transMsg.header.dataLength <= APP_MSG_MAX_DATA_LENGTH) {
        memcpy(transMsg.data, action->operation->message().c_str(), transMsg.header.dataLength);
        klogger() << "sent p" << medAccess->mediumAddress() << " " << action->operation->message() << " p" << action->operation->destOrSrcProcId() << " " << clock.currValue() << klogger::end();
        _sendTo((uint8_t*)&transMsg, sizeof(transMsg.header) + transMsg.header.dataLength, action->operation->destOrSrcProcId());
    }
    else {
        klogger(klogger::Errors) << "Send operation error, data too long" << klogger::end();
    }

    //we're done, let it be deleted
    return true;
}

bool ProcessObjectPrivate::doAction(RecvOpeartionAction *action) {
    if (0 != rcvdMessages.size()) {
        ReceivedMessageData *msgData = _takeReceivedMessage(action->operation->destOrSrcProcId());
        if (0 != msgData) {
            AppMessages::AppMessage *msg = (AppMessages::AppMessage*)msgData->data;
            assert(AppMessages::AppMsgTrans == msg->type);

            //we shouldn't send to print the received message
            std::string message((const char*)msg->printMsg.data, msg->printMsg.header.dataLength);
            //_opPlan.push_back(new PrintOperationAction(message));

            klogger() << "received p" << medAccess->mediumAddress() << " " << message << " p" << msgData->src << " " << clock.currValue() << klogger::end();

            rcvdMessages.pop_front();
            delete msgData;

            //done, may be deleted
            return true;
        }
        else {
            //is this even possible for recv to be called in different order than send?
            klogger(klogger::Errors) << "None message from " << action->operation->destOrSrcProcId() << " for " << medAccess->mediumAddress() << klogger::end();
        }
    }
    else {
        klogger(klogger::Info) << "Process p" << medAccess->mediumAddress() << " waiting for msg from p" << action->operation->destOrSrcProcId() << klogger::end();
    }

    return false;
}

bool ProcessObjectPrivate::doAction(AcquireMutexAction *action) {
    (void)action;
    const MutexHandler::MutexState state = mutexHndlr.state(PrinterMutexResourceIdentifier);
    switch (state) {
    case (MutexHandler::Released):
        mutexHndlr.acquire(PrinterMutexResourceIdentifier);
        break;
    case (MutexHandler::Wanted):
        klogger(klogger::Info) << "p" << medAccess->mediumAddress() << " waiting for mutex" << klogger::end();
        break;
    case (MutexHandler::Held):
        return true;
        break;
    default:
        assert(false);
    }

    return false;
}

bool ProcessObjectPrivate::doAction(ReleaseMutexAction *action)
{
    (void)action;
    const MutexHandler::MutexState state = mutexHndlr.state(PrinterMutexResourceIdentifier);
    switch (state) {
    case (MutexHandler::Released):
    case (MutexHandler::Wanted):
        assert(false);
        break;
    case (MutexHandler::Held):
        mutexHndlr.release(PrinterMutexResourceIdentifier);
        return true;
        break;
    default:
        assert(false);
    }

    return false;
}

bool ProcessObjectPrivate::doAction(PrintOperationAction *action)
{
    //other operation actions are realised with messages, thus clock is updated. Here it's not a case -> do it manually.
    clock.eventOccured();
    std::stringstream s;
    s << "printed p" << medAccess->mediumAddress() << " " << action->message << " " << clock.currValue();

    //being here, means we we are holding the mutex, so can send message to be printed
    AppMessages::PrintMsg printMsg;
    printMsg.header.type = AppMessages::AppMsgPrint;
    //! todo: check if the string is not too long!
    std::string fullMsg = s.str();
    printMsg.header.dataLength = fullMsg.copy((char*)printMsg.data, fullMsg.size());

    _sendTo((uint8_t*)&printMsg, sizeof(AppMessages::PrintMsgHeader) + printMsg.header.dataLength, PrinterMediumAddress);

    return true;
}

class MutexMediumParticipant:
        public MediumParticipant{
public:
    MutexMediumParticipant(ProcessObjectPrivate *procObjPriv):
        _procObjPriv(procObjPriv)
    {}

    virtual int send(uint8_t data[], int size) {
        return _procObjPriv->_send(data, size);
    }

    virtual int sendTo(uint8_t data[], int size, int destAddr) {
        return _procObjPriv->_sendTo(data, size, destAddr);
    }

    virtual void registerReceiver(MessageReceiver *receiver) {
        (void)receiver;
        assert(false);
    }

    virtual int mediumAddress() {
        return -1;
    }

private:
    ProcessObjectPrivate *_procObjPriv;
};

ProcessObjectPrivate::ProcessObjectPrivate(MediumParticipant *mediumParticipant, int totalProcessesNo, int procId):
        medAccess(mediumParticipant),
        mutexMedAccess(new MutexMediumParticipant(this)),
        mutexHndlr(mutexMedAccess, procId, totalProcessesNo, &clock),
        clockHandler(&clock)
{
}

ProcessObjectPrivate::~ProcessObjectPrivate()
{
    delete mutexMedAccess;
}


ProcessObject::ProcessObject(MediumParticipant *mediumParticipant, int totalProcessesNo, int procId):
    _procObjPriv(new ProcessObjectPrivate(mediumParticipant, totalProcessesNo, procId))
{
    _procObjPriv->medAccess->registerReceiver(this);

    std::stringstream s;
    s << "p" << procId;
    _procName = s.str();
}

ProcessObject::~ProcessObject()
{
    delete _procObjPriv;
}

void ProcessObject::buildPlan(const std::list<Operation *> *operations)
{
    _procObjPriv->_buildPlan(operations);
}

ReceivedMessageData *ProcessObjectPrivate::_takeReceivedMessage(unsigned int srcProcessId) {
    std::list<ReceivedMessageData*>::iterator msgsIt = rcvdMessages.begin();
    for (; rcvdMessages.end() != msgsIt; ++msgsIt) {
        if ((*msgsIt)->src == srcProcessId) {
            ReceivedMessageData *msgData = (*msgsIt);
            rcvdMessages.erase(msgsIt, msgsIt);
            return msgData;
        }
    }

    return 0;
}

ScheduledObject::StepResult ProcessObject::execStep()
{
    if (0 == _procObjPriv->opPlan.size())
        return ScheduledObject::MayFinish;

    OperationAction *action = _procObjPriv->opPlan.front();
    if (action->doAction(_procObjPriv)) {
        _procObjPriv->opPlan.pop_front();
        delete action;
    }

    return ScheduledObject::NotFinished;
}

#include <cstring>

int ProcessObjectPrivate::_sendTo(uint8_t *data, int size, int destAddress)
{
    int ret = clockHandler.appendClockToMsg(data, size, APP_MSG_MAX_DATA_LENGTH);
    if (ret < 0) {
        assert(false);
        return -1;
    }
    else {
        size += ret;
        ret = medAccess->sendTo(data, size, destAddress);

        return ret;
    }
}

int ProcessObjectPrivate::_send(uint8_t *data, int size)
{
    int ret = clockHandler.appendClockToMsg(data, size, APP_MSG_MAX_DATA_LENGTH);
    if (ret < 0) {
        assert(false);
        return -1;
    }
    else {
        size += ret;
        ret = medAccess->send(data, size);
        return ret;
    }
}

void ProcessObjectPrivate::_receive(int srcAddress, AppMessages::AppMessage *appMsg, int size, LamportClock::LamportClockType msgClock)
{
    switch (appMsg->type) {
    case (AppMessages::AppMsgTrans): {
        rcvdMessages.push_back(new ReceivedMessageData(srcAddress, &(appMsg->transMsg), size));
    }
        break;
    case (AppMessages::AppMsgPrint): {
        assert(false);
    }
        break;
    case (AppMessages::AppMsgMutex): {
        mutexHndlr.handleMessage(srcAddress, &(appMsg->mutexMsg), msgClock);
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
    int ret = _procObjPriv->clockHandler.removeClockFromMsg(data, size, &msgClock);
    if (ret < 0) {
        assert(false);
    }
    else {
        AppMessages::AppMessage *appMsg = (AppMessages::AppMessage*)data;
        _procObjPriv->_receive(srcAddress, appMsg, size - ret, msgClock);
    }
}

void ProcessObjectPrivate::_buildPlan(const std::list<Operation *> *operations)
{
    assert(0 == opPlan.size());

    std::list<Operation *>::const_iterator operIt = operations->begin();

    Operation *op = 0;
    bool isInsideMutexBlock = false;
    while (operations->end() != operIt) {
        op = *operIt;
        ++operIt;

        switch (op->type()) {
        case (Operation::OT_Send): {
            SendOrRecvOperation *sOp = dynamic_cast<SendOrRecvOperation*>(op);
            assert(0 != sOp);
            opPlan.push_back(new SendOperationAction(sOp));
        }
            break;
        case (Operation::OT_Recv): {
            SendOrRecvOperation *rOp = dynamic_cast<SendOrRecvOperation*>(op);
            assert(0 != rOp);
            opPlan.push_back(new RecvOpeartionAction(rOp));
        }
            break;
        case (Operation::OT_Print): {//print out of mutex block
            PrintOperation *pOp = dynamic_cast<PrintOperation*>(op);
            assert(0 != pOp);
            if (! isInsideMutexBlock)
                opPlan.push_back(new AcquireMutexAction());
            opPlan.push_back(new PrintOperationAction(pOp->message()));
            if (! isInsideMutexBlock)
                opPlan.push_back(new ReleaseMutexAction());
        }
            break;
        case (Operation::OT_BeginMutex): {
            opPlan.push_back(new AcquireMutexAction());
            isInsideMutexBlock = true;
        }
            break;
        case (Operation::OT_EndMutex):
            opPlan.push_back(new ReleaseMutexAction());
            isInsideMutexBlock = false;
            break;
        default:
            assert(false);
        }
    }
}
