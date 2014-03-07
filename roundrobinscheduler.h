#ifndef ROUNDROBINSCHEDULER_H
#define ROUNDROBINSCHEDULER_H

#include <map>

#include "scheduler.h"

class ScheduledObject;

class RoundRobinScheduler:
        public Scheduler
{
public:
    RoundRobinScheduler();

    virtual void registerObject(ScheduledObject *object);
    virtual void deregisterObject(ScheduledObject *object);

    virtual int exec();

private:
    std::map<ScheduledObject*, ScheduledObject*> _objects;
};

#endif // ROUNDROBINSCHEDULER_H
