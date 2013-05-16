//////////////////////////////////////////////////
//
//  JKG Account Interface System
//  Version 1.0
//
//  $Id$
//
//////////////////////////////////////////////////

#ifndef JKG_ACCOUNTS
#define JKG_ACCOUNTS

/*
define ('ACC_SUCCESS', 0);			// Successful login, proceed with connection. (Expect jkgCharacter_t)
define ('ACC_BADLOGIN', 1);			// Invalid account login. (Print generic error: Bad username/password)(Expect nothing)
define ('ACC_BADCHARSLOT', 2);		// Invalid character slot requested. (This shouldn't happen really, but it can.)(Expect nothing)
define ('ACC_IPBANNED', 3);			// IP is banned from the system. (Expect expire=>UNIX_TIMESTAMP, reason=>char[256])
define ('ACC_CHARBANNED', 4);		// Character is banned from the system. (Expect expire=>UNIX_TIMESTAMP, reason=>char[256])
define ('ACC_ACCOUNTBANNED', 5);	// Account is banned from the system. (Expect expire=>UNIX_TIMESTAMP, reason=>char[256])
define ('ACC_WRONGSERVER', 6);		// Client doesn't belong on this server. (Expect name=>char[64], address=>char[22])
define ('ACC_GENERALFAIL', 7);		// Generic error string give by auth server. (Expect message=>char[256])
*/

typedef enum {
	// Auth server directly transmitted error codes
	ACC_SUCCESS,			// Successful login, proceed with connection. (Expect jkgCharacter_t)
	ACC_BADLOGIN,			// Invalid account login. (Print generic error: Bad username/password)
	ACC_BADCHARSLOT,		// Invalid character slot requested. (This shouldn't happen really, but it can.)
	ACC_IPBANNED,			// IP is banned from the system. (Expect jkgBan_t)
	ACC_CHARBANNED,			// Character is banned from the system. (Expect jkgBan_t)
	ACC_ACCOUNTBANNED,		// Account is banned from the system. (Expect jkgBan_t)
	ACC_WRONGSERVER,		// Client doesn't belong on this server. (Expect /something/ here)
	ACC_GENERALFAIL,		// Generic error string give by auth server. (Expect char[256])
} authResponse_t;

typedef struct {
	int ololol;
} jkgCharacter_t;

void JKG_InitAccounts ( void );
char *JKG_AccountLogin ( const char *username, const char *hashedpass, int charslot, int clientNum, jkgCharacter_t **character );

#endif