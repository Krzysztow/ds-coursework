#include "roundrobinscheduler.h"

#include "scheduledobject.h"

typedef std::map<ScheduledObject*, ScheduledObject*>::iterator ObjectsIterator;

RoundRobinScheduler::RoundRobinScheduler()
{
}

void RoundRobinScheduler::registerObject(ScheduledObject *object)
{
    //don't duplicate
    ObjectsIterator it = _objects.find(object);
    if (_objects.end() != it)
        return;

    _objects[object] = object;
}

void RoundRobinScheduler::deregisterObject(ScheduledObject *object)
{
    //unregister only if it's there
    ObjectsIterator it = _objects.find(object);
    if (_objects.end() == it)
        _objects.erase(it);
}

int RoundRobinScheduler::exec()
{
    ObjectsIterator it;
    bool finish = false;
    //loop until all objects decide they are done
    while (!finish) {
        finish = true;
        it = _objects.begin();
        for (; _objects.end() != it; ++it) {
            ScheduledObject::StepResult res = it->second->execStep();
            finish &= (ScheduledObject::MayFinish == res);
        }
    }

    return 0;
}
