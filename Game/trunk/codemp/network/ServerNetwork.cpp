#include "../game/z_global_defines.h"

#ifdef __SECONDARY_NETWORK__

#include "StdAfx.h"
#include "ServerNetwork.h"
#include "ServerGame.h"

extern ServerGame *serverNet;

extern char SERVER_PORT[255];

ServerNetwork::ServerNetwork(void)
{
	// create WSADATA object
    WSADATA wsaData;

    // our sockets for the server
    ListenSocket = INVALID_SOCKET;
    ClientSocket = INVALID_SOCKET;



    // address info for the server to listen to
    struct addrinfo *result = NULL;
    struct addrinfo hints;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        //exit(1);
		return;
    }

    // set address information
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;    // UQ1: UDP connection??? - I can't get it to bind...
    hints.ai_flags = AI_PASSIVE;

	/*
            0,           // int ai_flags;
            AF_INET,     // int ai_family;
            SOCK_DGRAM,  // int ai_socktype;
            IPPROTO_UDP, // int ai_protocol;
            0,           // size_t ai_addrlen;
            NULL,        // char* ai_canonname;
            NULL,        // struct sockaddr* ai_addr;
            NULL         // struct addrinfo* ai_next;
	*/

	    // Resolve the server address and port
    iResult = getaddrinfo(NULL, SERVER_PORT, &hints, &result);

    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        //exit(1);
		return;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        //exit(1);
		return;
    }

    // Set the mode of the socket to be nonblocking
    u_long iMode = 1;
    iResult = ioctlsocket(ListenSocket, FIONBIO, &iMode);

    if (iResult == SOCKET_ERROR) {
        printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        //exit(1);
		return;
    }

    // Setup the TCP listening socket
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);

    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        //exit(1);
		return;
    }

    // no longer need address information
    freeaddrinfo(result);

    // start listening for new clients attempting to connect
    iResult = listen(ListenSocket, SOMAXCONN);

    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        //exit(1);
		return;
    }

	printf(">>>>> Secondary network server open on TCP port %s <<<<<\n", SERVER_PORT);
}


ServerNetwork::~ServerNetwork(void)
{
}

void ServerNetwork::Shutdown(void)
{
	closesocket(ListenSocket);
    WSACleanup();
}

// accept new connections
bool ServerNetwork::acceptNewClient(unsigned int & id)
{
    // if client waiting, accept the connection and save the socket
    ClientSocket = accept(ListenSocket,NULL,NULL);

    if (ClientSocket != INVALID_SOCKET) 
    {
        //disable nagle on the client's socket
        char value = 1;
        setsockopt( ClientSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof( value ) );

        // insert new client into session id table
        sessions.insert( pair<unsigned int, SOCKET>(id, ClientSocket) );

        return true;
    }

    return false;
}

// receive incoming data
int ServerNetwork::receiveData(unsigned int client_id, char * recvbuf)
{
    if( sessions.find(client_id) != sessions.end() )
    {
        SOCKET currentSocket = sessions[client_id];
        iResult = NetworkServices::receiveMessage(currentSocket, recvbuf, MAX_PACKET_SIZE);

        if (iResult == 0)
        {
            printf("Connection closed\n");
            closesocket(currentSocket);
        }

        return iResult;
    }

    return 0;
}

// send data to all clients
void ServerNetwork::sendToAll(char * packets, int totalSize)
{
    SOCKET currentSocket;
    std::map<unsigned int, SOCKET>::iterator iter;
    int iSendResult;

    for (iter = sessions.begin(); iter != sessions.end(); iter++)
    {
        currentSocket = iter->second;
        iSendResult = NetworkServices::sendMessage(currentSocket, packets, totalSize);

        if (iSendResult == SOCKET_ERROR) 
        {
            //printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(currentSocket);
        }
    }
}

#endif //__SECONDARY_NETWORK__
