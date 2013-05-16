// ==========================================================================================================================
//
// jkg_serversidenetwork.cpp : Defines the entry point for the new secondary networking system.
//
// source & usage info: http://www.codeproject.com/Articles/412511/Simple-client-server-network-using-Cplusplus-and-W
//
// ==========================================================================================================================

#include "../game/z_global_defines.h"

#ifdef __SECONDARY_NETWORK__

// may need #include "stdafx.h" in visual studio
#include "stdafx.h"
#include "ServerGame.h"
// used for multi-threading
#include <process.h>

ServerGame	*serverNet;

char		SERVER_PORT[255];

uintptr_t	SERVER_THREAD = 0;
bool		SERVER_SHUTDOWN = false;
bool		SERVER_SHUTTING_DOWN = false;

int REAL_ENTITYNUM[MAX_GENTITIES];

extern "C"
{
	#include "../game/g_local.h"

	extern vmCvar_t net_ip;
	extern vmCvar_t net_port;

	void serverLoop(void * arg) 
	{
		while(!SERVER_SHUTDOWN)
		{
			//serverNet->sendEventPacket(PACKETEVENT_TEST_TEXT, "Hello world!", sizeof("Hello world!"), 0);

			//do game stuff
			serverNet->update();
			Sleep(1);
		}
	}

	void jkg_netserverbegin()
	{
		int PORT = 0;

		//sv_hostname
		if (serverNet) return;

		if (!Q_stricmp(net_port.string, "0") || !Q_stricmp(net_port.string, ""))
		{// Default JKA port...
			//PORT = 29070 + 2000; // Secondary connection is + 2000...
			return; // try again after init...
		}
		else
		{
			PORT = net_port.integer + 2000; // Secondary connection is + 2000...
		}

		// Copy the port to our c++ struct...
		strcpy(SERVER_PORT, va("%i", PORT));

		// initialize the server
		serverNet = new ServerGame();

		// create thread with arbitrary argument for the run function
		_beginthread( serverLoop, 0, (void*)12);
	}

	void jkg_netservershutdown()
	{
		printf(">>>>> Secondary network server shutting down <<<<<\n");

		SERVER_SHUTDOWN = true;
		serverNet->GetNetwork()->Shutdown();
		delete serverNet;

		printf(">>>>> Secondary network server shutdown completed <<<<<\n");
	}

	// Pipe for sending data from normal game code...
	void jkg_net_send_packet( int eventID, char *eventData, int eventDataSize, int entityNum )
	{
		serverNet->sendEventPacket(eventID, eventData, eventDataSize, entityNum);
	}
}

#endif //__SECONDARY_NETWORK__
