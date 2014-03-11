#ifndef PROCESSOBJECT_H
#define PROCESSOBJECT_H

#include <list>
#include <string>

#include "mediumparticipant.h"
#include "mutexhandler.h"
#include "scheduledobject.h"
#include "lamportclock.h"
#include "lamportclockhandler.h"

class Operation;
class ProcessObjectPrivate;

class ProcessObject:
        public ScheduledObject,
        public MessageReceiver
{
public:
    ProcessObject(MediumParticipant *mediumParticipant, int totalProcessesNo, int procId);
    ~ProcessObject();

    void buildPlan(const std::list<Operation *> *operations);

public://inherited from ScheduledObject
    virtual StepResult execStep();

public://inherited from MessageReceiver
    virtual void receive(int srcAddress, uint8_t data[], int size);

private:
    ProcessObjectPrivate *_procObjPriv;
    std::string _procName;
};

#endif // PROCESSOBJECT_H
