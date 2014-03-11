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

class MutexHandler
{
public:
    enum MutexState {
        Released,
        Wanted,
        Held
    };

    MutexHandler(MediumParticipant *medAccess, int processesId, int allProcsNo, LamportClock *clock, const std::string &dbgMsg = std::string());

    MutexState state(int resourceId);
    void acquire(int resourceId);
    void release(int resourceId);

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
