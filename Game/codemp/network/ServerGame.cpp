#include "../game/z_global_defines.h"

#ifdef __SECONDARY_NETWORK__

#include "StdAfx.h"
#include "ServerGame.h"

unsigned int ServerGame::client_id; 


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
	gentity_t	*ent = NULL;

	// get stuff - Format (int ID, int dataSize, char *data)
	packet >> int32(eventID);

	while (eventID != 0)
	{
		// Server always recieves entityNum from the packet...
		packet >> int32(entityNum);
		ent = &g_entities[entityNum];

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
				printf("Client entity %i sent text %s.\n", ent->s.number, eventData);
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

ServerGame::ServerGame(void)
{
    // id's to assign clients for our table
    client_id = 0;

    // set up the server network to listen 
    network = new ServerNetwork(); 
}

ServerGame::~ServerGame(void)
{
}

void ServerGame::update()
{
    // get new clients
   if(network->acceptNewClient(client_id))
   {
        printf(">>>>> client %d has connected to the server <<<<<\n", client_id);

        client_id++;
   }

   receiveFromClients();
}

void ServerGame::receiveFromClients()
{
    Packet packet;

	if (!network) return;

    // go through all clients
    std::map<unsigned int, SOCKET>::iterator iter;

    for(iter = network->sessions.begin(); iter != network->sessions.end(); iter++)
    {
		int data_length = network->receiveData(iter->first, network_data);

        if (data_length <= 0) 
        {
            //no data recieved
            continue;
        }

		packet.clear();
		packet.append(network_data, data_length);
		GetDataFromPacket(packet);
		packet.clear();
    }

	//sendHelloPackets(); // test hello world...
}

void ServerGame::sendHelloPackets()
{
	// UQ1: send hello packet... This is an example of how to send a packet to the server...
	
	// Our packet struct...
	Packet packet;
	// Init the packet...
    packet.Initialize();
	// Add some data... You can do as many as these as needed...
	AddDataToPacket(packet, PACKETEVENT_TEST_TEXT, "Hello world!", sizeof("Hello world!") , 0);
	// Add ending to the packet...
	AddPacketEnding(packet);
	// Send it...
	network->sendToAll((char *)packet.contents(), packet.size());
}

void ServerGame::sendEventPacket( int eventID, char *eventData, int eventDataSize, int entityNum )
{
	// UQ1: send a packet...
	
	// Our packet struct...
	Packet packet;
	// Init the packet...
    packet.Initialize();
	// Add some data... You can do as many as these as needed...
	AddDataToPacket(packet, eventID, eventData, eventDataSize, entityNum);
	// Add ending to the packet...
	AddPacketEnding(packet);
	// Send it...
	network->sendToAll((char *)packet.contents(), packet.size());
}

#endif //__SECONDARY_NETWORK__
