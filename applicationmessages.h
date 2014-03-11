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

/**
 * @brief Protocol messages that transport a text message from one to second process.
 * They are responsivle for SEND/RECV operations.
 */
struct TransmissionMsgHeader {
    MsgType type;
    int dataLength;
};

struct TransmissionMsg {
    TransmissionMsgHeader header;
    uint8_t data[APP_MSG_MAX_DATA_LENGTH];
};

/**
 * @brief Protocol Print messssages that transport appropriate print text to the
 * network printer.
 */

struct PrintMsgHeader {
    MsgType type;
    int dataLength;
};

struct PrintMsg {
    PrintMsgHeader header;
    uint8_t data[APP_MSG_MAX_DATA_LENGTH];
};

/**
 * @brief Protocol mutex messages that take care of requesting for permission and granting
 * permission over the mutual exclusion.
 */

enum MutexMessageType {
    MutexRequired,
    MutexGranted
};

struct MutexMsg {
    MsgType type;
    int procId;
    int resourceId;
    MutexMessageType requestType;
};

union AppMessage {
    MsgType type;

    TransmissionMsg transMsg;
    PrintMsg printMsg;
    MutexMsg mutexMsg;
};

}
#endif // APPLICATIONMESSAGES_H
