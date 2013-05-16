#include "../game/z_global_defines.h"

#ifdef __SECONDARY_NETWORK__

#include "StdAfx.h"
#include "ClientGame.h"

extern "C"
{
	#include "../game/q_shared.h"
	#include "../cgame/cg_local.h"

	extern void QDECL CG_Printf( const char *msg, ... );

	extern qboolean CLIENT_FORCED_SHUTDOWN;
}

void AddPacketEnding(Packet& packet)
{
	// add EOP
	packet << int32(0);
}

void AddDataToPacket(Packet& packet, int eventID, char *eventData, int eventDataSize, int entityNum)
{
	// add stuff - Format (int ID, int dataSize, char *data)
	packet << int32(eventID);

	if (eventID == 0)
		return; // 0 Marks the end of packet...

	// Server always sends an entity number value... (required or not!)
	packet << int32(entityNum);
	packet << int32(eventDataSize);
	packet << eventData;
	//packet.append(eventData, strlen(eventData));
}

void GetDataFromPacket(Packet& packet)
{
	int			eventID = 0;
	int			eventDataSize = 0;
	char		*eventData;
	int			entityNum;
	centity_t	*cent = NULL;

	// get stuff - Format (int ID, int dataSize, char *data)
	packet >> int32(eventID);

	while (eventID != 0)
	{
		// Client always recieves entityNum from the packet...
		packet >> int32(entityNum);
		cent = &cg_entities[entityNum];

		packet >> int32(eventDataSize);

		eventData = (char *)malloc(eventDataSize);
		packet.read((uint8 *)eventData, eventDataSize);

		switch (eventID)
		{
		// ====================================================================================================================
		//
		// UQ1: Event Handling...
		//
		// Assign the incoming data to where it is meant to go in here...
		//
		// Place code for your new events in here. See q_shared.h for more information...
		//
		// ====================================================================================================================
		case PACKETEVENT_TEST_TEXT:
			{
				CG_Printf("Server entity %i sent text %s.\n", cent->currentState.number, eventData);
			}
			break;
		// ====================================================================================================================
		default:
			break;
		}

		free(eventData);

		// Get Next ID...
		packet >> int32(eventID);
	}
}

ClientGame::ClientGame(void)
{
    network = new ClientNetwork();
}


ClientGame::~ClientGame(void)
{
}

void ClientGame::sendHelloPacket()
{
	// UQ1: send hello packet... This is an example of how to send a packet to the server...
	
	// Our packet struct...
	Packet packet;
	// Init the packet...
    packet.Initialize();
	// Add some data... You can do as many as these as needed...
	AddDataToPacket(packet, PACKETEVENT_TEST_TEXT, "Hello world!", sizeof("Hello world!") , cg.clientNum);
	// Add ending to the packet...
	AddPacketEnding(packet);
	// Send it...
    NetworkServices::sendMessage(network->ConnectSocket, (char *)packet.contents(), packet.size());
}

void ClientGame::sendEventPacket( int eventID, char *eventData, int eventDataSize )
{
	// UQ1: send a packet...
	
	// Our packet struct...
	Packet packet;
	// Init the packet...
    packet.Initialize();
	// Add some data... You can do as many as these as needed...
	AddDataToPacket(packet, eventID, eventData, eventDataSize, cg.clientNum);
	// Add ending to the packet...
	AddPacketEnding(packet);
	// Send it...
	NetworkServices::sendMessage(network->ConnectSocket, (char *)packet.contents(), packet.size());
}

void ClientGame::update()
{
    Packet packet;

	if (!network) return;

    int data_length = network->receivePackets(network_data);

    if (data_length <= 0) 
    {
		//sendHelloPacket();

        //no data recieved
        return;
    }

	// UQ1: New...
	packet.clear();
	packet.append(network_data, data_length);
	GetDataFromPacket(packet);
	packet.clear();
}

#endif //__SECONDARY_NETWORK__
