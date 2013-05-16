//       ____ ___________________   ___           ____  __ _______   ___  ________  ___ ______________
//      |    |\_   _____/\______ \ |   |         |    |/ _|\      \ |   |/  _____/ /   |   \__    ___/
//      |    | |    __)_  |    |  \|   |         |      <  /   |   \|   /   \  ___/    ~    \|    |   
//  /\__|    | |        \ |    `   \   |         |    |  \/    |    \   \    \_\  \    Y    /|    |   
//  \________|/_______  //_______  /___|         |____|__ \____|__  /___|\______  /\___|_  / |____|   
//                    \/         \/                      \/       \/            \/       \/           
//                         ________    _____   ____       _____  ____  ___ ______________ _________   
//                        /  _____/   /  _  \ |    |     /  _  \ \   \/  /|   \_   _____//   _____/   
//                       /   \  ___  /  /_\  \|    |    /  /_\  \ \     / |   ||    __)_ \_____  \    
//                       \    \_\  \/    |    \    |___/    |    \/     \ |   ||        \/        \   
//                        \______  /\____|__  /_______ \____|__  /___/\  \|___/_______  /_______  /   
//                               \/         \/        \/	   \/	   \_/			  \/        \/ (c)
// jkg_libcurl.h
// JKG Libcurl Communications System
// (c) 2013 Jedi Knight Galaxies

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