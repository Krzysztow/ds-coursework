#ifndef PROCESSOBJECT_H
#define PROCESSOBJECT_H

#include "mediumparticipant.h"
#include "scheduledobject.h"

class MediumParticipant;
class ProcessObject:
        public ScheduledObject,
        public MessageReceiver
{
public:
    ProcessObject(MediumParticipant *mediumParticipant);
    virtual StepResult execStep();

public://inherited from MessageReceiver
    virtual void receive(int srcAddress, uint8_t data[], int size);

private:
    MediumParticipant *_medAccess;
};

#endif // PROCESSOBJECT_H
