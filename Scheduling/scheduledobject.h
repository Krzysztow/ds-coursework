#ifndef SCHEDULEDOBJECT_H
#define SCHEDULEDOBJECT_H

class ScheduledObject {
public:
    enum StepResult {
        MayFinish,
        NotFinished
    };
    /**
     * @brief execStep executes one action.
     * @return MayFinish if finished and allows for program execution at that time.
     */
    virtual StepResult execStep() = 0;
};

#endif // SCHEDULEDOBJECT_H
