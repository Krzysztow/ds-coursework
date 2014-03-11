#ifndef SCHEDULEDOBJECT_H
#define SCHEDULEDOBJECT_H

/**
 * @brief The ScheduledObject class - object being scheduled by Scheduler class.
 */

class ScheduledObject {
public:
    enum StepResult {
        MayFinish,
        NotFinished
    };
    /**
     * @brief execStep executes one action.
     * Don't block here. It should execute only atomic action.
     * @return MayFinish if finished and allows for program execution at that time.
     */
    virtual StepResult execStep() = 0;
};

#endif // SCHEDULEDOBJECT_H
