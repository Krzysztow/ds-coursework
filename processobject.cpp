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
    ProcessObjectPrivate(MediumParticipant *mediumParticipant, int totalProcessesNo, int procId, const std::string &dbgMsg = std::string());
    ~ProcessObjectPrivate();

    /**
     * @brief setDebugMessage - sets debug message prefix to be written, when @sa logger() output is invoked.
     * @param dbgMsg
     */
    void setDebugMessage(const std::string &dbgMsg);

    /**
     * @brief _buildPlan - translates set of Operations (from input file) to the set of actions that will be execyted in ProcessObject::execStep().
     * Every operation but print is translated directly to one atomic operations.
     * In case of print, I check if we are in a mutex block. If so, just atomic print
     * is appended to the list of operations. If not, mutex acquisition action is added
     * before printing, and mutex release after it.
     */
    void _buildPlan(const std::list<Operation *> *operations);

    /**
     * @brief _sendTo - appends data (after size bytes) with current Lamport clock and sends it to medium.
     * @warning: the data pointer has to have at least size + sizeof(LamportClock) bytes. Otherwise we overwrite stack!
     * @param data - serialized message to be sent;
     * @param size - size of the current message.
     * @param destAddress - address of the received (equivalent here to process id).
     * @return number of bytes sent (size + lamport clock size on success).
     */
    int _sendTo(uint8_t *data, int size, int destAddress);
    /**
     * @brief _send - same as above, but sends broadcast.
     */
    int _send(uint8_t *data, int size);

    /**
     * @brief _receive - handler to be invoked, whenever there is a message to the ProcessObject.
     * Actions taken are dependand on the message type:
     * - if message is of transport type it's added to the list of received messages (later on will be picked up in recv action execution, in execStep()
     * - if message is of mutex type, it's disapatched to the mutex handler;
     * - print messages are not handled.
     * @param srcAddress - who (processId==src address) sent it;
     * @param appMsg - application message, already
     * @param size
     * @param msgClock
     */
    void _receive(int srcAddress, AppMessages::AppMessage *appMsg, int size, LamportClock::LamportClockType msgClock);

    /**
     * @brief _takeReceivedMessage - looks for a first message from a given processId.
     * @note: this could be done with multiple map, not list as is.
     * @param srcProcessId - processId (src address) of the sender
     * @return
     */
    ReceivedMessageData *_takeReceivedMessage(unsigned int srcProcessId);

    /**
     * @brief doAction - sends message to the remote process.
     * @return true, if done (so that execution can proceed to another action, in next execStep() invocation).
     */
    bool doAction(SendOperationAction *action);
    /**
     * @brief doAction - tries to receive the message from particular process by looking for rcvdMessages.
     * @return true, if there was a message (so that execution can proceed to another action, in next execStep() invocation).
     */
    bool doAction(RecvOpeartionAction *action);
    /**
     * @brief doAction - requests mutexHandler to acquire mutex for a printer resource (in our case it's just one resource).
     * @return true, if the mutex was acquired (so that execution can proceeed to another action).
     */
    bool doAction(AcquireMutexAction *action);
    /**
     * @brief doAction - requests mutexHandler to release the held mutex for a printer resource.
     * @return always true - mutex release is done automatically.
     */
    bool doAction(ReleaseMutexAction *action);
    /**
     * @brief doAction sends print message to the network printer (the execution plan is built so that
     * we know, we have a mutex for that resource already).
     * @return always true, after message was sent.
     */
    bool doAction(PrintOperationAction *action);

public:
    /**
     * @brief medAccess - pointer to medium to send messages to.
     */
    MediumParticipant *medAccess;

    /**
     * @brief opPlan - list of actions to be executed.
     */
    std::list<OperationAction *> opPlan;
    /**
     * @brief rcvdMessages - list of received messages.
     */
    std::list<ReceivedMessageData*> rcvdMessages;
    /**
     * @brief mutexMedAccess - wrapper for MutexHandler, so that when it sends messages
     * they ProcessObjectPrivate::_send() is invoked and Lamport Clock added.
     */
    MutexMediumParticipant *mutexMedAccess;
    friend class MutexMediumParticipant;
    MutexHandler mutexHndlr;

    /**
     * @brief clock - instance of the lamport clock for this process.
     */
    LamportClock clock;
    /**
     * @brief clockHandler - class translating message tail to the lamport clock (and vice versa).
     */
    LamportClockHandler clockHandler;

    /**
     * @brief dbgMessage - klogger() messages prefix.
     */
    std::string dbgMessage;
};


/**
 * @brief The ReceivedMessageData class - class for holding queued received messages.
 * Contains information about the source address (equivalent to process id), message content and its size.
 */
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

/**
 * @brief The OperationAction class - representing visitee to be visited by ProcessObjectPrivate instance
 * to execute apporpriate ProcessObjectPrivate::doAction() methods.
 */
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
        _sendTo((uint8_t*)&transMsg, sizeof(transMsg.header) + transMsg.header.dataLength, action->operation->destOrSrcProcId());
        std::cout << "sent p" << medAccess->mediumAddress() << " " << action->operation->message() << " p" << action->operation->destOrSrcProcId() << " " << clock.currValue() << std::endl;
    }
    else {
        klogger(klogger::Errors) << "Send operation error, data too long" << klogger::end();
    }

    //we're done, let it be deleted
    return true;
}

bool ProcessObjectPrivate::doAction(RecvOpeartionAction *action) {
    if (0 != rcvdMessages.size()) {
        //get message from an expected process
        ReceivedMessageData *msgData = _takeReceivedMessage(action->operation->destOrSrcProcId());
        if (0 != msgData) {
            //it has been received
            AppMessages::AppMessage *msg = (AppMessages::AppMessage*)msgData->data;
            assert(AppMessages::AppMsgTrans == msg->type);

            std::string message((const char*)msg->printMsg.data, msg->printMsg.header.dataLength);
            std::cout << "received p" << medAccess->mediumAddress() << " " << message << " p" << msgData->src << " " << clock.currValue() << std::endl;

            delete msgData;

            //done, may be deleted
            return true;
        }
    }

    klogger(klogger::Info) << dbgMessage << " waiting for message from " << action->operation->destOrSrcProcId() << klogger::end();

    return false;
}

bool ProcessObjectPrivate::doAction(AcquireMutexAction *action) {
    (void)action;
    //ask fora mutex on a printer resource, only once. If we are already asking for it (Wanted state), do nothing.
    //if it's gained, proceed with antother action.
    const MutexHandler::MutexState state = mutexHndlr.state(PrinterMutexResourceIdentifier);
    switch (state) {
    case (MutexHandler::Released):
        mutexHndlr.acquire(PrinterMutexResourceIdentifier);
        break;
    case (MutexHandler::Wanted):
        klogger(klogger::Info) << dbgMessage << " waiting for mutex" << klogger::end();
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
    //release the mutex only if it's held by us
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
    //being here, means we we are holding the mutex, so can send message to be printed

    //other operation actions are realised with messages, thus clock is updated. Here it's not a case -> do it manually.
    clock.eventOccured();

    //we are going to send a message to the network printer, construct its content
    std::stringstream s;
    s << "printed p" << medAccess->mediumAddress() << " " << action->message << " " << clock.currValue();

    //going to send a message
    AppMessages::PrintMsg printMsg;
    printMsg.header.type = AppMessages::AppMsgPrint;
    //! todo: check if the string is not too long!
    std::string fullMsg = s.str();
    printMsg.header.dataLength = fullMsg.copy((char*)printMsg.data, fullMsg.size());

    _sendTo((uint8_t*)&printMsg, sizeof(AppMessages::PrintMsgHeader) + printMsg.header.dataLength, PrinterMediumAddress);

    return true;
}

/**
 * @brief The MutexMediumParticipant class - just a wrapper class, to make
 * mutexhandler not aware of MediumParticipant.
 */
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

ProcessObject::ProcessObject(MediumParticipant *mediumParticipant, int totalProcessesNo, int procId):
    _procObjPriv(new ProcessObjectPrivate(mediumParticipant, totalProcessesNo, procId))
{
    _procObjPriv->medAccess->registerReceiver(this);

    std::stringstream s;
    s << "p" << procId;
    _procName = s.str();

    _procObjPriv->setDebugMessage(_procName);
}

ProcessObject::~ProcessObject()
{
    delete _procObjPriv;
}

void ProcessObject::buildPlan(const std::list<Operation *> *operations)
{
    _procObjPriv->_buildPlan(operations);
}

ProcessObjectPrivate::ProcessObjectPrivate(MediumParticipant *mediumParticipant, int totalProcessesNo, int procId, const std::string &dbgMsg):
        medAccess(mediumParticipant),
        mutexMedAccess(new MutexMediumParticipant(this)),
        mutexHndlr(mutexMedAccess, procId, totalProcessesNo, &clock),
        clockHandler(&clock),
        dbgMessage(dbgMsg)
{
}

ProcessObjectPrivate::~ProcessObjectPrivate()
{
    delete mutexMedAccess;
}

void ProcessObjectPrivate::setDebugMessage(const std::string &dbgMsg)
{
    this->dbgMessage = dbgMsg;
    mutexHndlr.setDebuggMessage(dbgMsg);
}

ReceivedMessageData *ProcessObjectPrivate::_takeReceivedMessage(unsigned int srcProcessId) {
    std::list<ReceivedMessageData*>::iterator msgsIt = rcvdMessages.begin();
    //iterate over a list of up-to-now received messages and look for a one with matching sender id
    for (; rcvdMessages.end() != msgsIt; ++msgsIt) {
        if ((*msgsIt)->src == srcProcessId) {
            //found, remove from the list and return pointer to message
            ReceivedMessageData *msgData = (*msgsIt);
            rcvdMessages.erase(msgsIt);
            return msgData;
        }
    }

    return 0;
}

ScheduledObject::StepResult ProcessObject::execStep()
{
    //are there any other actions to be executed?
    if (0 == _procObjPriv->opPlan.size())
        return ScheduledObject::MayFinish;

    //take one action, execyte it and check if it's done
    OperationAction *action = _procObjPriv->opPlan.front();
    if (action->doAction(_procObjPriv)) {
        //action done, we can fortget about it
        _procObjPriv->opPlan.pop_front();
        delete action;
    }

    return ScheduledObject::NotFinished;
}

#include <cstring>

int ProcessObjectPrivate::_sendTo(uint8_t *data, int size, int destAddress)
{
    //append clock to the unicast message
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
    //append clock to the broadcast message
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
        //we got transmission message, put it on the recevied messages list, to be picked up when, receive action is executed.
        rcvdMessages.push_back(new ReceivedMessageData(srcAddress, &(appMsg->transMsg), size));
    }
        break;
    case (AppMessages::AppMsgPrint): {
        //we are not interested int this kind of message
        assert(false);
    }
        break;
    case (AppMessages::AppMsgMutex): {
        //release, ackuire or allow acquisition message
        mutexHndlr.handleMessage(srcAddress, &(appMsg->mutexMsg), msgClock);
    }
        break;
    default:
        klogger(klogger::Errors) << dbgMessage << "received unexpected message " << appMsg->type;
        assert(false);
    }
}

void ProcessObject::receive(int srcAddress, uint8_t data[], int size)
{
    //lamport clock is piggy-backed to the message, strip it and update current clock
    //this probably should be moved ot ProcessObjectPrivate::_receive(), since appending with LamportClock is done there as well.
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

    //iterate over the list of operations to be done and create appropriate plan
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
