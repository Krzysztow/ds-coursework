#ifndef PROCESSOBJECT_H
#define PROCESSOBJECT_H

#include <list>

#include "mediumparticipant.h"
#include "scheduledobject.h"

class MediumParticipant;
class LamportClockHandler;
class Operation;

class ProcessObject:
        public ScheduledObject,
        public MessageReceiver
{
public:
    ProcessObject(MediumParticipant *mediumParticipant);

    void setOperations(const std::list<Operation *> *operations);

public://inherited from ScheduledObject
    virtual StepResult execStep();

public://inherited from MessageReceiver
    virtual void receive(int srcAddress, uint8_t data[], int size);

private:
    MediumParticipant *_medAccess;
    LamportClockHandler *_clock;

    const std::list<Operation *>  *_operations;
    std::list<Operation *>::const_iterator _operIt;
};

#endif // PROCESSOBJECT_H
