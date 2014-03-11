#ifndef MUTEXOBJECT_H
#define MUTEXOBJECT_H

#include <list>
#include <map>
#include <stdint.h>
#include <string>

#include "lamportclockhandler.h"
#include "mediumparticipant.h"

class MutexResourceData;
namespace AppMessages {
class MutexMsg;
}

typedef std::map<int, MutexResourceData*> MutexesData;

/**
 * @brief The MutexHandler class that takes care of acquiring resources in a distributed system.
 * It implements Ricart and Agrawala algorithm. It can handle many resources (identified with resourceId).
 */

class MutexHandler
{
public:
    enum MutexState {
        Released,
        Wanted,
        Held
    };

    MutexHandler(MediumParticipant *medAccess, int processesId, int allProcsNo, LamportClock *clock, const std::string &dbgMsg = std::string());

    /**
     * @brief state returns actual state of the resource.
     * @param resourceId identification of the resource.
     * @return MutexState, depending on that if we have requested that already and gained access to it or not.
     */
    MutexState state(int resourceId);
    /**
     * @brief acquire starts acquisition of mutex for a given resource.
     */
    void acquire(int resourceId);
    /**
     * @brief release releases the mutex held for a given resource.
     */
    void release(int resourceId);

    /**
     * @brief handleMessage handle mutex messages for a process.
     * @param srcAddress process identifier of the requesting process,
     * @param mutexMsg message body,
     * @param msgClock Lamport clock of the message.
     */
    void handleMessage(int srcAddress, AppMessages::MutexMsg *mutexMsg, LamportClock::LamportClockType msgClock);

    void setDebuggMessage(const std::string &dbgMsg);

private:
    void _sendMutexMsg(int destAddr, bool isAcquire, int resourceId);
    void _handleMutexMsg(int srcAddress, AppMessages::MutexMsg *mutexMessage, LamportClock msgClock);

private:
    //access to medium to send requests/responses
    MediumParticipant *_mediumAccess;
    //this process identifier
    int _procId;
    //number of other processes
    int _otherProcsNo;
    //data for access for exclusion
    MutexesData _mutexesData;
    LamportClock *_clock;

    std::string _dbgMessage;
};

#endif // MUTEXOBJECT_H
