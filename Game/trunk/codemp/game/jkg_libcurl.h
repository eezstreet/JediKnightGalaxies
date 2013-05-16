//////////////////////////////////////////////////
//
//  JKG Libcurl Communications System
//  Version 1.0 (Multi-platform)
//  Server-side
//
//  $Id$
//
//////////////////////////////////////////////////

#include "jkg_threading.h"

extern vmCvar_t		jkg_masterServerUrl;
extern vmCvar_t		jkg_planetId;

// For libcurl downloads
typedef struct {
	char *writeBuffer;
	int bufSize;
} jkgMemoryChunk_t;

typedef enum {
	LCMETHOD_TEST = 0,			// Test query (debugging purposes)
	LCMETHOD_SVSTARTUP,			// Server Startup
	LCMETHOD_SVSHUTDOWN,		// Server Shutdown
	LCMETHOD_SVHEARTBEAT,		// Periodical heartbeats to the master server
	LCMETHOD_CLCONNECT,			// Client Connections
	LCMETHOD_CLDISCONNECT,		// Client Disconnections
	LCMETHOD_NETWORKRCON,		// Didz approves of a network rcon system :o
} lcMethod_t;

#define CURL_MAX_HANDLES 16

asyncTask_t *JKG_NewNetworkTask ( lcMethod_t method, void (*finalCallback)(asyncTask_t *taskPointer), ... );
char *JKG_Libcurl_Perform ( int method, void *args );
int JKG_Libcurl_Init ( void );
void JKG_Libcurl_Shutdown ( void );
void JKG_Libcurl_Poller( void );