#include "../game/z_global_defines.h"

#ifdef __SECONDARY_NETWORK__

#include "StdAfx.h"
#include "NetworkServices.h"

int NetworkServices::sendMessage(SOCKET curSocket, char * message, int messageSize)
{
    return send(curSocket, message, messageSize, 0);
}

int NetworkServices::receiveMessage(SOCKET curSocket, char * buffer, int bufSize)
{
    return recv(curSocket, buffer, bufSize, 0);
}

#endif //__SECONDARY_NETWORK__
