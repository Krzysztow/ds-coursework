#include "processobject.h"

#include "mediumparticipant.h"
#include "processobject.h"

ProcessObject::ProcessObject(MediumParticipant *mediumParticipant):
    _medAccess(mediumParticipant)
{
   _medAccess->registerReceiver(this);
}

ScheduledObject::StepResult ProcessObject::execStep()
{
    return ScheduledObject::MayFinish;
}

void ProcessObject::receive(int srcAddress, uint8_t data[], int size)
{
}
