#ifndef NETWORKPRINTER_H
#define NETWORKPRINTER_H

#include "mediumparticipant.h"

class NetworkPrinter:
        public MessageReceiver
{
public:
    NetworkPrinter(MediumParticipant *mediumParticipant);

public://MessageReceiver overrides
    virtual void receive(int srcAddress, uint8_t data[], int size);
};

#endif // NETWORKPRINTER_H
