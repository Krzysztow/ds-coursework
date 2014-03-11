#ifndef SCHEDULER_H
#define SCHEDULER_H

class ScheduledObject;

/**
 * @brief The Scheduler class - interface for a scheduler (for more information, see RoundRobinScheduler).
 */

class Scheduler {
public:
    virtual void registerObject(ScheduledObject *object) = 0;
    virtual void deregisterObject(ScheduledObject *object) = 0;

    virtual int exec() = 0;
};

#endif // SCHEDULER_H
