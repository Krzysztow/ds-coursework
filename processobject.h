#ifndef PROCESSOBJECT_H
#define PROCESSOBJECT_H

#include <list>

#include "mediumparticipant.h"
#include "mutexhandler.h"
#include "scheduledobject.h"
#include "lamportclock.h"
#include "lamportclockhandler.h"

class Operation;
class OperationAction;
class MutexMediumParticipant;
class ReceivedMessageData;
union AppMessage;

class ProcessObject:
        public ScheduledObject,
        public MessageReceiver
{
public:
    ProcessObject(MediumParticipant *mediumParticipant, int totalProcessesNo, int procId);
    ~ProcessObject();

    void setOperations(const std::list<Operation *> *operations);

public://inherited from ScheduledObject
    virtual StepResult execStep();

public://inherited from MessageReceiver
    virtual void receive(int srcAddress, uint8_t data[], int size);

private:
    void _buildPlan();
    int _sendTo(uint8_t *data, int size, int destAddress);
    int _send(uint8_t *data, int size);
    void _receive(int srcAddress, AppMessage *appMsg, int size, LamportClock::LamportClockType msgClock);


private:
    MediumParticipant *_medAccess;

    const std::list<Operation *>  *_operations;
    std::list<Operation *>::const_iterator _operIt;
    std::list<OperationAction *> _opPlan;
    std::list<ReceivedMessageData*> _rcvdMessages;

    MutexMediumParticipant *_mutexMedAccess;
    friend class MutexMediumParticipant;
    MutexHandler _mutexHndlr;

    LamportClock _clock;
    LamportClockHandler _clockHandler;
};

#endif // PROCESSOBJECT_H
