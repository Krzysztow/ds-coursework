#ifndef MEDIUMMESSAGE_H
#define MEDIUMMESSAGE_H

#include <cstring>

//smart pointers would be better
class MediumMessage {
public:
    MediumMessage(int sender, int receiver, uint8_t data[], int size):
        size(size),
        sender(sender),
        receiver(receiver)
    {
        buffer = new uint8_t[size * sizeof(data)];
        memcpy(buffer, data, size * sizeof(data));
    }

    ~MediumMessage() {
        delete []buffer;
    }

public:
    uint8_t *data() {
        return buffer;
    }

    int dataSize() {
        return size;
    }

    int senderAddress() {
        return sender;
    }

    int receiverAddress() {
        return receiver;
    }

private:
    //no copy constructor, undefined
    MediumMessage(const MediumMessage &other);

    int size;
    int sender;
    int receiver;

    uint8_t *buffer;
};

#endif // MEDIUMMESSAGE_H
