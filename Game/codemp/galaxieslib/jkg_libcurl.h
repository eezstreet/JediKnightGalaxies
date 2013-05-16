//////////////////////////////////////////////////
//
//  JKG Libcurl Communications System
//  Version 1.0 (Multi-platform)
//  Client-side
//
//  $Id$
//
//////////////////////////////////////////////////

#include "jkg_threading.h"

// For libcurl downloads
typedef struct {
	unsigned char *writeBuffer;
	int bufSize;
} jkgMemoryChunk_t;

typedef enum {
	LCMETHOD_TEST = 0,			// Test query (debugging purposes)
	LCMETHOD_GETTERMSOFUSE,		// Obtain terms of use (for registration)
	
	// Extra Arguments: const char *username, int rawpassword, const char *password, const char *email
	LCMETHOD_REGISTERUSER,		// Register user

	// Extra Arguments: const char *username, int rawpassword, const char *password
	LCMETHOD_LOGIN,				// Log in
} lcMethod_t;

#define CURL_MAX_HANDLES 2

asyncTask_t *JKG_NewNetworkTask ( lcMethod_t method, void (*finalCallback)(asyncTask_t *taskPointer), int ui, ... );
char *JKG_Libcurl_Perform ( int method, void *args );
int JKG_Libcurl_Init ( void );
void JKG_Libcurl_Shutdown ( void );
void JKG_Libcurl_Poller( void );