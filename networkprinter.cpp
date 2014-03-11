#include "networkprinter.h"

#include <assert.h>

#include "applicationmessages.h"
#include "klogger.h"

NetworkPrinter::NetworkPrinter(MediumParticipant *mediumParticipant)
{
    mediumParticipant->registerReceiver(this);
}

void NetworkPrinter::receive(int srcAddress, uint8_t data[], int size)
{
    (void)size;
    AppMessages::AppMessage *appMsg = (AppMessages::AppMessage*)data;
    if (AppMessages::AppMsgPrint == appMsg->type) {
        std::string message((char*)(appMsg->printMsg.data), appMsg->printMsg.header.dataLength);
        klogger(klogger::Info) << "PRINTER: " << klogger::end();
        std::cout << message << std::endl;
    }
    else if (AppMessages::AppMsgMutex == appMsg->type) {
        klogger(klogger::Info) << "printer drops mutex request" << klogger::end();
    }
    else {
        assert(false);
        klogger(klogger::Errors) << "unknown msg type " << appMsg->type << " from " << srcAddress << klogger::end();
    }
}
