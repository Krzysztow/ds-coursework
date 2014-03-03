#ifndef MEDIUMPARTICIPANT_H
#define MEDIUMPARTICIPANT_H

#include <stdint.h>
#include <stdlib.h>

class MessageReceiver {
public:
    virtual void receive(int srcAddress, uint8_t data[], int size) = 0;
};

class MediumParticipant {
public:
    /**

     */
    virtual void registerReceiver(MessageReceiver *receiver) = 0;

    /**
     * @brief send sends data with broadcast to all processes;
     * @param data - data to be sent;
     * @param size - size of the data;
     * @return - number of bytes sent.
     */
    virtual int send(uint8_t data[], int size) = 0;

    /**
     * @brief sendto sends data to the process with unicast.
     * @param data - data to be sent;
     * @param size - size of the data;
     * @param destAddr - identifier of destination process;
     * @return number of bytes sent.
     */
    virtual int sendto(uint8_t data[], int size, int destAddr) = 0;\

    virtual ~MediumParticipant() {}
};

#endif // MEDIUMPARTICIPANT_H
