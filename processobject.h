#ifndef PROCESSOBJECT_H
#define PROCESSOBJECT_H

#include <list>
#include <string>

#include "mediumparticipant.h"
#include "mutexhandler.h"
#include "scheduledobject.h"
#include "lamportclock.h"
#include "lamportclockhandler.h"

class Operation;
class ProcessObjectPrivate;

/**
 * @brief The ProcessObject class - class that represents the process being simulated.
 */
class ProcessObject:
        public ScheduledObject,
        public MessageReceiver
{
public:
    ProcessObject(MediumParticipant *mediumParticipant, int totalProcessesNo, int procId);
    ~ProcessObject();

    /**
     * @brief buildPlan - creates a list of actions, that will be invoked every @sa execStep() is called.
     * @param operations - operations list to be translated into "atomic" actions.
     */
    void buildPlan(const std::list<Operation *> *operations);

public://inherited from ScheduledObject
    /**
     * @brief execStep - executes atomic process action (send, recv, acquire/release lock).
     * @return returns NotFinished if it hasn't done all the operations. MayFinish if all operations were done.
     */
    virtual StepResult execStep();

public://inherited from MessageReceiver
    /**
     * @brief receive - receives messages from the environment and processes them.
     * If the received message is of:
     * - trans type - it's queued to be collected, when Recv operation will be invoked;
     * - mutex type - it's directly dispatched to the mutex handler (in private object), to be taken care of.
     * @warning: the message should be piggy-backed with Lamport clock. Here we strip last few bytes and interpret them as lamport clock,
     * which updates our internal clock.
     * @param srcAddress - source address of the process (equivalent with process id);
     * @param data - serialized message (here message is finished with Lamport clock);
     * @param size - size of the message.
     */
    virtual void receive(int srcAddress, uint8_t data[], int size);

private:
    ProcessObjectPrivate *_procObjPriv;
    std::string _procName;
};

#endif // PROCESSOBJECT_H
