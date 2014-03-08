#include "processobject.h"

#include "mediumparticipant.h"
#include "processobject.h"
#include "lamportclockhandler.h"

ProcessObject::ProcessObject(MediumParticipant *mediumParticipant):
    _medAccess(mediumParticipant),
    _clock(new LamportClockHandler())
{
    _medAccess->registerReceiver(this);
}

void ProcessObject::setOperations(const std::list<Operation *> *operations)
{
    _operations = operations;
    _operIt = _operations->begin();
}

ScheduledObject::StepResult ProcessObject::execStep()
{
    return ScheduledObject::MayFinish;
}

#include <cstring>

void ProcessObject::receive(int srcAddress, uint8_t data[], int size)
{
    //lamport clock is piggy-backed to the message
    LamportClock clockValue;
    memcpy(&clockValue, data + size - sizeof(clockValue), sizeof(clockValue));
    _clock->messageClockReceived(clockValue);


}
