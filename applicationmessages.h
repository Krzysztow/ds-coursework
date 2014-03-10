#ifndef APPLICATIONMESSAGES_H
#define APPLICATIONMESSAGES_H

#include <stdint.h>

#define APP_MSG_MAX_DATA_LENGTH 128

namespace AppMessages {

enum MsgType {
    AppMsgTrans,
    AppMsgPrint,
    AppMsgMutex
};

struct TransmissionMsgHeader {
    MsgType type;
    int dataLength;
};

struct TransmissionMsg {
    TransmissionMsgHeader header;
    uint8_t data[APP_MSG_MAX_DATA_LENGTH];
};

struct PrintMsgHeader {
    MsgType type;
    int dataLength;
};

struct PrintMsg {
    PrintMsgHeader header;
    uint8_t data[APP_MSG_MAX_DATA_LENGTH];
};

struct MutexMsg {
    MsgType type;
    int procId;
    int resourceId;
    bool isAcquire;
};

union AppMessage {
    MsgType type;

    TransmissionMsg transMsg;
    PrintMsg printMsg;
    MutexMsg mutexMsg;
};

}
#endif // APPLICATIONMESSAGES_H
