#include "scheduledmediumdispatcher.h"

#include "mediumdispatcher.h"

ScheduledMediumDispatcher::ScheduledMediumDispatcher(MediumDispatcher *medDispatcher):
    _medDispatcher(medDispatcher)
{
}

ScheduledObject::StepResult ScheduledMediumDispatcher::execStep()
{
    return (MediumDispatcher::MD_NoMoreMessages == _medDispatcher->dispatchMessages() ?
                MayFinish : NotFinished);
}
