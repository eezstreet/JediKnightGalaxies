// ==========================================================================================================================
//
// jkg_clientsidenetwork.cpp : Defines the entry point for the new secondary networking system.
//
// source & usage info: http://www.codeproject.com/Articles/412511/Simple-client-server-network-using-Cplusplus-and-W
//
// ==========================================================================================================================

#include "../game/z_global_defines.h"

#ifdef __SECONDARY_NETWORK__

// may need #include "stdafx.h" in visual studio
#include "stdafx.h"
#include "ClientGame.h"
// used for multi-threading
#include <process.h>

ClientGame * clientNet;

char SERVER_IP[256];
char SERVER_PORT[256];

uintptr_t	CLIENT_THREAD = 0;
bool		CLIENT_SHUTDOWN = false;

extern "C"
{
	#include "../cgame/cg_local.h"

	vmCvar_t net_hostip;
	vmCvar_t net_hostport;

	qboolean CLIENT_FORCED_SHUTDOWN = qfalse;

	void clientLoop(void * arg) 
	{
		while(!CLIENT_SHUTDOWN)
		{
			//clientNet->sendEventPacket(PACKETEVENT_TEST_TEXT, "Hello world!", sizeof("Hello world!"));

			//do game stuff
			clientNet->update();
			Sleep(1);
		}
	}

	void jkg_netclientbegin()
	{
		int PORT = 0;

		//sv_hostname
		if (clientNet) return;

		trap_Cvar_Register( &net_hostip, "net_hostip", "", CVAR_ROM );
		trap_Cvar_Register( &net_hostport, "net_hostport", "", CVAR_ROM );

		if (!Q_stricmp(net_hostip.string, "") || !Q_stricmp(net_hostip.string, "0")) 
			return; // CGAME not initialized yet!

		CLIENT_FORCED_SHUTDOWN = qfalse;

		if (net_hostport.integer <= 0)
		{// Default JKA port...
			PORT = 29070 + 2000; // Secondary connection is + 2000...
		}
		else
		{
			PORT = net_hostport.integer + 2000; // Secondary connection is + 2000...
		}

		// Copy the IP to our c++ struct...
		strcpy(SERVER_IP, net_hostip.string);

		// Copy the port to our c++ struct...
		strcpy(SERVER_PORT, va("%i", PORT));

		// initialize the client 
		clientNet = new ClientGame();

		// create thread with arbitrary argument for the run function
		CLIENT_THREAD = _beginthread( clientLoop, 0, (void*)12);
	}

	void jkg_netclientshutdown()
	{
		printf(">>>>> Secondary network server connection shutting down <<<<<\n");

		CLIENT_SHUTDOWN = true;
		CLIENT_FORCED_SHUTDOWN = qfalse;

		clientNet->network->Shutdown();

		Sleep(100);

		if (clientNet)
			delete clientNet;

		CLIENT_SHUTDOWN = false;

		printf(">>>>> Secondary network server connection shutdown completed <<<<<\n");
	}

	// Pipe for sending data from normal game code...
	void jkg_net_send_packet( int eventID, char *eventData, int eventDataSize )
	{
		clientNet->sendEventPacket(eventID, eventData, eventDataSize);
	}
}

#endif //__SECONDARY_NETWORK__
