#ifndef SCHEDULEDMEDIUMDISPATCHER_H
#define SCHEDULEDMEDIUMDISPATCHER_H

#include "scheduledobject.h"

class MediumDispatcher;

/**
 * @brief The ScheduledMediumDispatcher class - wrapper for a MediumDispatcher so that
 * it can be scheduled by a Scheduler class.
 */

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
