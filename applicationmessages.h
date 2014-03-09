#ifndef APPLICATIONMESSAGES_H
#define APPLICATIONMESSAGES_H

#include <stdint.h>

#define APP_MSG_MAX_DATA_LENGTH 128

enum AppMsgType {
    AppMsgTrans,
    AppMsgPrint,
    AppMsgMutex
};

struct TransmissionAppMsg {
    AppMsgType type;
    int dataLength;
    uint8_t data[APP_MSG_MAX_DATA_LENGTH];
};

struct PrintAppMsg {
    AppMsgType type;
    int dataLength;
    uint8_t data[APP_MSG_MAX_DATA_LENGTH];
};

struct MutexAppMsg {
    AppMsgType type;

    int procId;
    int resourceId;
    bool isAcquire;
};

union AppMessage {
    AppMsgType type;

    TransmissionAppMsg transMsg;
    PrintAppMsg printMsg;
    MutexAppMsg mutexMsg;
};

#endif // APPLICATIONMESSAGES_H
