#ifndef SCHEDULEDMEDIUMDISPATCHER_H
#define SCHEDULEDMEDIUMDISPATCHER_H

#include "scheduledobject.h"

class MediumDispatcher;

class ScheduledMediumDispatcher:
        public ScheduledObject
{
public:
    ScheduledMediumDispatcher(MediumDispatcher *medDispatcher);

public:
    virtual StepResult execStep();

private:
    MediumDispatcher *_medDispatcher;
};

#endif // SCHEDULEDMEDIUMDISPATCHER_H
