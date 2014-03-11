#include "roundrobinscheduler.h"

#include <cstdlib>
#include <time.h>
#include <unistd.h>

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
    timespec t;
    clock_gettime(CLOCK_MONOTONIC_COARSE, &t);
    srand(t.tv_nsec + t.tv_sec);

    ObjectsIterator it = _objects.begin();
    bool finish = false;
    bool isFirstLoop = true;//needed if we want to randomize start

    //randomize the first object being scheduled
    const int startObject = rand() % _objects.size();
    std::advance(it, startObject);

    //loop until all objects decide they are done
    while (!finish) {
        finish = true;
        for (; _objects.end() != it; ++it) {
            ScheduledObject::StepResult res = it->second->execStep();
            finish &= (ScheduledObject::MayFinish == res);
        }

        //don't finish after first loop
        if (isFirstLoop) {
            isFirstLoop = false;
            finish = false;
        }

        it = _objects.begin();

        usleep(10000);
    }

    return 0;
}
