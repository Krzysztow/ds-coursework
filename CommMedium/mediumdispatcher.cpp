#include "mediumdispatcher.h"

bool MediumDispatcher::isBcastAddress(int address)
{
    return (MD_BroadcastAddress == address);
}

int MediumDispatcher::bcastAddress()
{
    return MD_BroadcastAddress;
}
