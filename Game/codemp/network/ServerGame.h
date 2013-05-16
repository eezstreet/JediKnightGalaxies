#include "../game/z_global_defines.h"

#ifdef __SECONDARY_NETWORK__

#pragma once
#include "ServerNetwork.h"
#include "NetworkData.h"

extern "C"
{
	#include "../game/q_shared.h"
}

class ServerGame
{

public:

    ServerGame(void);
    ~ServerGame(void);

    void update();

	void receiveFromClients();

	void sendEventPacket( int eventID, char *eventData, int eventDataSize, int entityNum );
	void sendHelloPackets();

	ServerNetwork* GetNetwork() { return network; }
private:

   // IDs for the clients connecting for table in ServerNetwork 
    static unsigned int client_id;

   // The ServerNetwork object 
    ServerNetwork* network;

	// data buffer
   char network_data[MAX_PACKET_SIZE];
};

#endif //__SECONDARY_NETWORK__
