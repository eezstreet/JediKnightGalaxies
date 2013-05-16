//////////////////////////////////////////////////
//
//  JKG Account Interface System
//  Version 1.0
//
//  $Id$
//
//////////////////////////////////////////////////

/*
	I've thoroughly commented the accounts system since it could take a while
	to get your heads around how the code works.

	This system is a good example of how the threading system is utilized.

	- Didz
*/

#include "g_local.h"
#include "g_engine.h"		// Access to the svs struct
#include "jkg_threading.h"
#include "jkg_libcurl.h"
#include "json/cJSON.h"
#include "jkg_dynarrays.h"

typedef enum {
	CONNSTATE_NEW = 0,		// New unused connection slot
	CONNSTATE_AUTHORIZING,	// A task has been created to authorize the client but is still running
	CONNSTATE_RESPONDED		// Client authorization finished, awaiting validation
} jkgConnState_t;

typedef struct {
	jkgConnState_t	state;		// Connection state to avoid new connection problems while still authorizing an old client
	int				time;		// Time that the connection request was first made
	int				challenge;	// Challenge key
	int				qport;		// QPort
	byte			ip[4];		// IP as signed bytes
	asyncTask_t		*task;		// Async task pointer for an authorizing connection
	cJSON			*reply;		// Pointer to an allocated cJSON structure, the task callback fills this in
} jkgConnection_t;

static jkgConnection_t jkgConnectingClients[MAX_CLIENTS];

char *sAuthorizing = "Authorizing...";			// Not const since function returns char* and
char *sBusy = "Server busy, please wait...";	// typecasting (char*) for each instance of this is ugly.
char *sError = "^1Unexpected Error.\n\n"JKG_ERRMSG;

// -- Called from G_InitGame()
void JKG_InitAccounts ( void )
{
	memset( jkgConnectingClients, 0, sizeof(jkgConnectingClients) );
}

// -- Assigned as a final callback function to authorize clients from the network task function.
// -- It's responsible for matching up the task to the correct connection in jkgConnectingClients.
// asyncTask_t taskPointer	=	The threading system passes the pointer to the newly finished task here
static void JKG_AccountTaskFinal ( asyncTask_t *taskPointer )
{
	int i;
	jkgConnection_t *correctConn;

	for ( i = 0; i < sizeof(jkgConnectingClients) / sizeof(jkgConnection_t); i++ ) {
		correctConn = &jkgConnectingClients[i];

		if ( correctConn->task == taskPointer ) {
			correctConn->reply = taskPointer->finalData;
			correctConn->state = CONNSTATE_RESPONDED;

			// HACKHACKHACK, Boba: TASKSTATE_DATACOLLECT is/was there for a reason o.o
			// You're putting tasks that are in DATACOLLECT state into the Finished tasks queue.
			// The finished tasks queue should only have TASKSTATE_FINISHED tasks in there.
			// cJSON was deleting my response before I got a chance to use it :\
			// I don't want auto-deletion on the response cJSON struct, the accounts code
			// handles the deletion. I don't want to memcpy taskPointer->finalData into
			// correctConn->reply :P
			taskPointer->flags &= ~TASKFLAG_JSON_FINALDATA;
			// So the dilemna, live with this hack to stop the system from auto-deleting cJSON data..
			// or do something different?
			// If we're only using queues now, we probably need to alter the TASKSTATEs.

			return;
		}
	}

	// The finished task didn't match any connections, wtf?!
	G_Printf( "^1JKG_AccountTaskFinal: Unmatched authorization reply! (Was created at %i, returned error code %i)\n", taskPointer->createTime, taskPointer->errorCode );

	return;
}

// -- Called from ClientConnect(), returns a string if auth failed, otherwise *character is filled in and a null string is returned.
// const char *username			=	Account username
// const char *hashedPass		=	Hashed account password
// int charslot					=	Character slot the player chose to play with
// int clientNum				=	Client slot number
// jkgCharacter_t *character	=	Function fills this in if auth was successful and a character is assigned to the client.
char *JKG_AccountLogin ( const char *username, const char *hashedpass, int charslot, int clientNum, jkgCharacter_t **character )
{
	int challenge, qport;
	byte ip[4];
	jkgConnection_t *conn;
	asyncTask_t *netTask;
	// cJSON newly authorized client stuff:
	cJSON *root;
	authResponse_t errorCode;
	cJSON *data;
	static char staticResponseBuf[MAX_STRING_CHARS];

	return NULL;

	// Genuine data checks
	if ( !username[0] ) return "No username given.";
	if ( !hashedpass[0] ) return "No password given.";
	// TODO: Additional length check for the password hash
	if ( charslot < 1 || charslot > 5 ) return "Bad character slot.";

	// Copy client info from svs into the local variables for less typing :p
	challenge = svs->clients[clientNum].challenge;
	qport = svs->clients[clientNum].netchan.qport;
	memcpy( ip, svs->clients[clientNum].netchan.remoteAddress.ip, sizeof(ip) );

	// Less typing is good.
	conn = &jkgConnectingClients[clientNum];

	if ( conn->challenge == challenge && conn->qport == qport &&
		conn->ip[0] == ip[0] && conn->ip[1] == ip[1] &&
		conn->ip[2] == ip[2] && conn->ip[3] == ip[3] )
	{
			// We have a previously connecting client here!

			if ( conn->state != CONNSTATE_RESPONDED ) {
				// No response from the master server yet.
				return sAuthorizing;
			}

			if ( !conn->reply ) { // No reply
				return sError;
			}

			////////////////////////////////
			//   cJSON response parsing   //
			////////////////////////////////
			root = conn->reply;
			memset( staticResponseBuf, 0, sizeof(staticResponseBuf) );

			errorCode = cJSON_ToIntegerOpt( cJSON_GetObjectItem(root, "errorCode"), -1 );

			switch ( errorCode )
			{
				case ACC_SUCCESS:
					data = cJSON_GetObjectItem(root, "data");

					cJSON_Delete( root );
					memset( conn, 0, sizeof( jkgConnection_t ) );
					return NULL; // Connection Granted!
				case ACC_BADLOGIN:
					Q_strncpyz( staticResponseBuf, "Invalid username or password.", sizeof(staticResponseBuf) );
					break;
				case ACC_BADCHARSLOT:
					Q_strncpyz( staticResponseBuf, "Bad character slot.", sizeof(staticResponseBuf) );
					break;
				/*case ACC_IPBANNED:
					break;
				case ACC_CHARBANNED:
					break;
				case ACC_ACCOUNTBANNED:
					break;*/
				case ACC_WRONGSERVER:
					data = cJSON_GetObjectItem(root, "redirect");
					// TODO: Ensure new server address exists
					Q_strncpyz( staticResponseBuf, va( "@svr %s", cJSON_ToString( data ) ), sizeof(staticResponseBuf) );
					break;
				case ACC_GENERALFAIL:
					data = cJSON_GetObjectItem(root, "message");
					Q_strncpyz( staticResponseBuf, va("^1Authorization Error: %s\n\n"JKG_ERRMSG, cJSON_ToStringOpt( data, "N/A" )), sizeof(staticResponseBuf) );
					break;
				default:
					Q_strncpyz( staticResponseBuf, sError, sizeof(staticResponseBuf) );
					break;
			}

			// Error from the PHP Script falls here...

			// Note: We don't free the client's 'conn' here because
			// the client will attempt to connect again and we don't
			// want to try to re-authenticate them.

			return staticResponseBuf; // Connection Rejected
	}

	if ( conn->state == CONNSTATE_AUTHORIZING ) {
		// This is not very good, this client slot is authorizing but
		// it's not the right person. We must just wait for the task to finish
		// and print the generic "Authorizing..." message to the client.
		return sBusy;
	}

	if ( conn->state == CONNSTATE_RESPONDED ) {
		// An old authorized connection that was never handled
		// (Client disconnected before auth finished)
		//assert( conn->reply ); // Should NEVER happen, but check anyway.. maybe :d
		cJSON_Delete( conn->reply );
	}

	// New client connection! Create a new network task to authorize them
	memset( conn, 0, sizeof( jkgConnection_t ) );
	conn->challenge = challenge;
	conn->qport = qport;
	memcpy( &conn->ip, &ip, sizeof(conn->ip) );


	conn->state = CONNSTATE_AUTHORIZING;
	netTask = JKG_NewNetworkTask( LCMETHOD_CLCONNECT, JKG_AccountTaskFinal, username, hashedpass, charslot ); // This will create the new network task if there's a free task slot


	if ( !netTask ) {
		// Bail out, clean and free everything we changed
		memset( conn, 0, sizeof( jkgConnection_t ) );
		return sBusy; // No task slots free at the moment, deny connection
	}

	// We need to keep this for the final callback function
	conn->task = netTask;

	return sAuthorizing;
}