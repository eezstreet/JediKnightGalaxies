#include "../game/z_global_defines.h"

#ifdef __SECONDARY_NETWORK__

#include "StdAfx.h"
#include "ClientNetwork.h"
#include "ClientGame.h"

extern "C"
{
	//#include "../game/q_shared.h"

	extern void QDECL CG_Printf( const char *msg, ... );

	extern qboolean CLIENT_FORCED_SHUTDOWN;
}

extern ClientGame * clientNet;

extern char SERVER_IP[256];
extern char SERVER_PORT[256];

ClientNetwork::ClientNetwork(void)
{
    // create WSADATA object
    WSADATA wsaData;

    // socket
    ConnectSocket = INVALID_SOCKET;

    // holds address info for socket to connect to
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

    if (iResult != 0) {
        CG_Printf("WSAStartup failed with error: %d\n", iResult);
        //exit(1);
		CLIENT_FORCED_SHUTDOWN = qtrue;
		return;
    }



    // set address info
    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;  // UQ1: UDP connection??? - Just cant get UDP to bind a socket on server...

	
    //resolve server address and port 
    iResult = getaddrinfo(SERVER_IP, SERVER_PORT, &hints, &result);

    if( iResult != 0 ) 
    {
        CG_Printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        //exit(1);
		CLIENT_FORCED_SHUTDOWN = qtrue;
		return;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);

        if (ConnectSocket == INVALID_SOCKET) {
            CG_Printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            //exit(1);
			CLIENT_FORCED_SHUTDOWN = qtrue;
			return;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

        if (iResult == SOCKET_ERROR)
        {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            CG_Printf ("The server is down... did not connect. (IP %s. Port: %s)\n", SERVER_IP, SERVER_PORT);
        }
    }



    // no longer need address info for server
    freeaddrinfo(result);



    // if connection failed
    if (ConnectSocket == INVALID_SOCKET) 
    {
        CG_Printf("Unable to connect to server!\n");
        WSACleanup();
        //exit(1);
		CLIENT_FORCED_SHUTDOWN = qtrue;
		return;
    }

	// Set the mode of the socket to be nonblocking
    u_long iMode = 1;

    iResult = ioctlsocket(ConnectSocket, FIONBIO, &iMode);
    if (iResult == SOCKET_ERROR)
    {
        CG_Printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        //exit(1);    
		CLIENT_FORCED_SHUTDOWN = qtrue;
		return;
    }

	//disable nagle
    char value = 1;
    setsockopt( ConnectSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof( value ) );

	CG_Printf(">>>>> Secondary TCP network connection open to server %s on port %s <<<<<\n", SERVER_IP, SERVER_PORT);
}


ClientNetwork::~ClientNetwork(void)
{
}

void ClientNetwork::Shutdown(void)
{
	if (ConnectSocket)
		closesocket(ConnectSocket);

    WSACleanup();
}

int ClientNetwork::receivePackets(char * recvbuf) 
{
    iResult = NetworkServices::receiveMessage(ConnectSocket, recvbuf, MAX_PACKET_SIZE);

    if ( iResult == 0 )
    {
        CG_Printf("Connection closed\n");
        closesocket(ConnectSocket);
        WSACleanup();
        exit(1);
    }

    return iResult;
}

#endif //__SECONDARY_NETWORK__
