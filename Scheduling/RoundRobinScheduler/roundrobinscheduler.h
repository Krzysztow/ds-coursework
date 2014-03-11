#ifndef ROUNDROBINSCHEDULER_H
#define ROUNDROBINSCHEDULER_H

#include <map>

#include "scheduler.h"

class ScheduledObject;

/**
 * @brief The RoundRobinScheduler class - class that is to failry schedule the ScheduledObject instances
 * in signle-threaded application.
 */

class RoundRobinScheduler:
        public Scheduler
{
public:
    RoundRobinScheduler();

    /**
     * @brief registerObject addds object to be scheduled, in @sa RoundRobinScheduler::exec() method.
     * @param object - object to be added.
     */
    virtual void registerObject(ScheduledObject *object);
    /**
     * @brief deregisterObject removes object from being scheduled.
     * @param object - object to be removed.
     */
    virtual void deregisterObject(ScheduledObject *object);

    /**
     * @brief exec enters the loop for object scheduling. Finished, if all the objects allow for that
     * in the same loop run.
     * @return
     */
    virtual int exec();

private:
    std::map<ScheduledObject*, ScheduledObject*> _objects;
};

#endif // ROUNDROBINSCHEDULER_H
