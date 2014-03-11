#ifndef NETWORKPRINTER_H
#define NETWORKPRINTER_H

#include "mediumparticipant.h"

/**
 * @brief The NetworkPrinter class - class simulating the network printer.
 * It doesn't take part int the mutex acquisition process (it could, but then
 * all the processes would have to take it into account (number of processes + 1)
 * and it would always respond with true).
 */

class NetworkPrinter:
        public MessageReceiver
{
public:
    NetworkPrinter(MediumParticipant *mediumParticipant);

public://MessageReceiver overrides
    virtual void receive(int srcAddress, uint8_t data[], int size);
};

#endif // NETWORKPRINTER_H
