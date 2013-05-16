// TODO: put this file in UI
#include "../ui/ui_local.h"
#include "../ui/ui_shared.h"
#include "../ui/ui_crossover.h"

//=========================================================================
//
// SECOND PLAYERSTATE
// Runs a second, extendable playerstate called a networkstate
//
//=========================================================================

int MSG_ReadBits(msg_t *msg, int numBits)
{ 
	int returnVal;
	__asm 
	{ 
		push	numBits
		mov		esi, msg
		mov		eax, 0x004405C0
		call	eax
		add		esp, 4
		mov		returnVal, eax
	}
	return returnVal;
}

// Client
networkState_t networkState;

void	trap_SendConsoleCommand( const char *text );
extern networkState_t *CO_GetNetworkState(void);
extern extraState_t *CO_GetExtraState(int entNum);
extern extraState_t *CO_GetOldExtraState(int entNum);
void N_DeltaEntity (msg_t *msg, clSnapshot_t *frame, int newnum, qboolean unchanged);
void N_ReadDeltaExtrastate( msg_t *msg, extraState_t *from, extraState_t *to, int number);
void N_ParseExtraStates( msg_t *msg, clSnapshot_t *oldframe, clSnapshot_t *newframe);

void N_ReadDeltaNetworkstate( msg_t *msg, clSnapshot_t *oldframe, clSnapshot_t *newframe )
{
	int			i, lc;
	int			*toF;
	netField_t	*field;
	int			numFields;
	int			trunc;
	networkState_t *targetState = CO_GetNetworkState();

	numFields = sizeof( networkStateFields ) / sizeof( networkStateFields[0] );
	lc = MSG_ReadBits(msg, 8);

	for ( i = 0, field = networkStateFields ; i < lc ; i++, field++ ) {
		toF = (int *)( (byte *)targetState + field->offset );

		if ( ! MSG_ReadBits( msg, 1 ) ) {
			// Do nothing
		} else {
			if ( field->bits == 0 ) {
				// float
				if ( MSG_ReadBits( msg, 1 ) == 0 ) {
					// integral float
					trunc = MSG_ReadBits( msg, FLOAT_INT_BITS );
					// bias to allow equal parts positive and negative
					trunc -= FLOAT_INT_BIAS;
					*(float *)toF = trunc; 
				} else {
					// full floating point value
					*toF = MSG_ReadBits( msg, 32 );
				}
			} else {
				// integer
				*toF = MSG_ReadBits( msg, field->bits );
			}
		}
	}
	//for ( i=lc,field = &networkStateFields[lc];i<numFields; i++, field++) {
	//	fromF = (int *)( (byte *)from + field->offset );
	//	toF = (int *)( (byte *)to + field->offset );
	//	// no change
	//	*toF = *fromF;
	//}

	N_ParseExtraStates(msg, oldframe, newframe);
}

#define READ_POS 0x00421AB7
#define READ_ORIG_CALL 0x00421560
#define READ_RETURN 0x00421ABC

// jamp
// hook location: 0x00421AB7
// 00421AB7: E8 A4 FA FF FF    call    CL_ParsePacketEntities
byte read_restore[] = { 0xe8, 0xa4, 0xfa, 0xff, 0xff };
void __declspec(naked) _Hook_NetworkState_Read(void)
{ 
	__asm 
	{ 
		add esp, 4

		pushad
		pushfd

		push eax
		push edi
		push esi
		call N_ReadDeltaNetworkstate
		add esp, 12

		popfd
		popad

		mov eax, READ_ORIG_CALL
		call eax
		mov eax, READ_RETURN
		jmp eax
	} 
}

#define WIN32_LEAN_AND_MEAN
#ifndef Rectangle
#include <windows.h>
#endif

void PatchAddress( unsigned int address, unsigned int destination)
{
	int ret, dummy;

	ret = VirtualProtect((LPVOID)address, 5, PAGE_EXECUTE_READWRITE, &dummy);
	*(unsigned char *)address = 0xE8;
	*(unsigned int *)(address+1) = destination - (address + 5);
	ret = VirtualProtect((LPVOID)address, 5, PAGE_EXECUTE_READ, NULL);
}


void N_CL_Init()
{
	PatchAddress( READ_POS, (unsigned int)_Hook_NetworkState_Read );
}

void N_CL_Clear()
{
	int ret, dummy;

	ret = VirtualProtect((LPVOID)READ_POS, 5, PAGE_EXECUTE_READWRITE, &dummy);
	memcpy((LPVOID)READ_POS, read_restore, 5);
	ret = VirtualProtect((LPVOID)READ_POS, 5, PAGE_EXECUTE_READ, NULL);
}

#define MAX_PARSE_ENTITIES 2048
entityState_t	*cl_parseEntities = (entityState_t*)0x00A1BC98;

cvar_t **cvar_handles = (cvar_t **)0x00B44228;


void N_ParseExtraStates( msg_t *msg, clSnapshot_t *oldframe, clSnapshot_t *newframe)
{
	int			newnum;
	entityState_t	*oldstate;
	int			oldindex, oldnum;

	oldnum = 0;
	while(1)
	{
		newnum = MSG_ReadBits( msg, GENTITYNUM_BITS );
		if ( newnum == (MAX_GENTITIES-1) ) {
			break;
		}

		while(oldnum < newnum)
        {
			N_DeltaEntity(msg, newframe, oldnum, qtrue);
			oldnum++;
		}

		N_DeltaEntity( msg, newframe, newnum, qfalse );

		oldnum = newnum;
	}

	// delta from the entities present in oldframe
	oldindex = 0;
	oldstate = NULL;
	if (!oldframe) {
		oldnum = 99999;
	} else {
		if ( oldindex >= oldframe->numEntities ) {
			oldnum = 99999;
		} else {
			oldstate = &cl_parseEntities[
				(oldframe->parseEntitiesNum + oldindex) & (MAX_PARSE_ENTITIES-1)];
			oldnum = oldstate->number;
		}
	}

	while ( 1 ) {
		// read the entity index number
		newnum = MSG_ReadBits( msg, GENTITYNUM_BITS );

		if ( newnum == (MAX_GENTITIES-1) ) {
			break;
		}

		if ( msg->readcount > msg->cursize ) {
			Com_Error (ERR_DROP,"N_ParseExtraStates: end of message");
		}

		while ( oldnum < newnum ) {
			// one or more entities from the old packet are unchanged
			N_DeltaEntity( msg, newframe, oldnum, qtrue );
			
			oldindex++;

			if ( oldindex >= oldframe->numEntities ) {
				oldnum = 99999;
			} else {
				oldstate = &cl_parseEntities[
					(oldframe->parseEntitiesNum + oldindex) & (MAX_PARSE_ENTITIES-1)];
				oldnum = oldstate->number;
			}
		}
		if (oldnum == newnum) {
			// delta from previous state
			N_DeltaEntity( msg, newframe, newnum, qfalse );

			oldindex++;

			if ( oldindex >= oldframe->numEntities ) {
				oldnum = 99999;
			} else {
				oldstate = &cl_parseEntities[
					(oldframe->parseEntitiesNum + oldindex) & (MAX_PARSE_ENTITIES-1)];
				oldnum = oldstate->number;
			}
			continue;
		}

		if ( oldnum > newnum ) {
			// delta from baseline
			//N_DeltaEntity( msg, newframe, newnum, &cl.entityBaselines[newnum], qfalse );
			N_DeltaEntity( msg, newframe, newnum, qfalse );
			continue;
		}

	}

	// any remaining entities in the old frame are copied over
	while ( oldnum != 99999 ) {
		// one or more entities from the old packet are unchanged
		N_DeltaEntity( msg, newframe, oldnum, qtrue );
		
		oldindex++;

		if ( oldindex >= oldframe->numEntities ) {
			oldnum = 99999;
		} else {
			oldstate = &cl_parseEntities[
				(oldframe->parseEntitiesNum + oldindex) & (MAX_PARSE_ENTITIES-1)];
			oldnum = oldstate->number;
		}
	}
}

extraState_t	new_parseExtras[MAX_GENTITIES];
extraState_t	old_parseExtras[MAX_GENTITIES];

void N_DeltaEntity (msg_t *msg, clSnapshot_t *frame, int newnum, qboolean unchanged)
{
	//entityState_t	*state;

	// save the parsed entity state into the big circular buffer so
	// it can be used as the source for a later delta
	//state = &cl_parseEntities[cl.parseEntitiesNum & (MAX_PARSE_ENTITIES-1)];

	if ( unchanged ) {
		//*state = *old;
		//new_parseExtras[newnum] = old_parseExtras[newnum];
		extraState_t *state = CO_GetExtraState(newnum);
		if(state != NULL) // well this here kinda makes it not get used i assume. mostly it will use CO_Get... at least
		{
			memcpy(state, &old_parseExtras[newnum], sizeof(extraState_t));
		}
		else
		{
			new_parseExtras[newnum] = old_parseExtras[newnum];
		}
	} else {
		//MSG_ReadDeltaEntity( msg, old, state, newnum );
		extraState_t *state = CO_GetExtraState(newnum);
		if(state != NULL)
		{
			N_ReadDeltaExtrastate( msg, &old_parseExtras[newnum], state, newnum );
		}
		else
		{
			N_ReadDeltaExtrastate( msg, &old_parseExtras[newnum], &new_parseExtras[newnum], newnum );
		}
	}

	//if ( state->number == (MAX_GENTITIES-1) ) {
	//	return;		// entity was delta removed
	//}
	//cl.parseEntitiesNum++;
	//frame->numEntities++;
}


void N_ReadDeltaExtrastate( msg_t *msg, extraState_t *from, extraState_t *to, int number)
{
	int			i, lc;
	int			numFields;
	netField_t	*field;
	int			*fromF, *toF;
	int			trunc;
	int			startBit;

	if ( number < 0 || number >= MAX_GENTITIES) {
		Com_Error( ERR_DROP, "Bad delta entity number: %i", number );
	}

	if ( msg->bit == 0 ) {
		startBit = msg->readcount * 8 - GENTITYNUM_BITS;
	} else {
		startBit = ( msg->readcount - 1 ) * 8 + msg->bit - GENTITYNUM_BITS;
	}

	// check for a remove
	if ( MSG_ReadBits( msg, 1 ) == 1 ) {
		//Com_Memset( to, 0, sizeof( *to ) );	
		//to->number = MAX_GENTITIES - 1;
		return;
	}

	// check for no delta
	if ( MSG_ReadBits( msg, 1 ) == 0 ) {
		//*to = *from;
		//to->number = number;
		extraState_t *state = CO_GetExtraState(number);
		if(state != NULL)
		{
			memcpy(state, &old_parseExtras[number], sizeof(extraState_t));
			//new_parseExtras[number] = old_parseExtras[number];
		}
		else
		{
			new_parseExtras[number] = old_parseExtras[number];
		}
		return;
	}

	numFields = sizeof(extraStateFields)/sizeof(extraStateFields[0]);
	lc = MSG_ReadBits(msg, 8);

	// shownet 2/3 will interleave with other printed info, -1 will
	// just print the delta records`
	//if ( cl_shownet->integer >= 2 || cl_shownet->integer == -1 ) {
	//	print = 1;
	//	Com_Printf( "%3i: #%-3i ", msg->readcount, to->number );
	//} else {
	//	print = 0;
	//}

	to->number = number;

	for ( i = 0, field = extraStateFields ; i < lc ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );

		if ( ! MSG_ReadBits( msg, 1 ) ) {
			// no change
			*toF = *fromF;
		} else {
			if ( field->bits == 0 ) {
				// float
				if ( MSG_ReadBits( msg, 1 ) == 0 ) {
						*(float *)toF = 0.0f; 
				} else {
					if ( MSG_ReadBits( msg, 1 ) == 0 ) {
						// integral float
						trunc = MSG_ReadBits( msg, FLOAT_INT_BITS );
						// bias to allow equal parts positive and negative
						trunc -= FLOAT_INT_BIAS;
						*(float *)toF = trunc; 
						//if ( print ) {
						//	Com_Printf( "%s:%i ", field->name, trunc );
						//}
					} else {
						// full floating point value
						*toF = MSG_ReadBits( msg, 32 );
						//if ( print ) {
						//	Com_Printf( "%s:%f ", field->name, *(float *)toF );
						//}
					}
				}
			} else {
				if ( MSG_ReadBits( msg, 1 ) == 0 ) {
					*toF = 0;
				} else {
					// integer
					*toF = MSG_ReadBits( msg, field->bits );
					//if ( print ) {
					//	Com_Printf( "%s:%i ", field->name, *toF );
					//}
				}
			}
//			pcount[i]++;
			// At this point toF is now set so
			*fromF = *toF; // this the point it crashed on? // i think so, lemme see if i can pull up a log
		}
	}
	for ( i = lc, field = &extraStateFields[lc] ; i < numFields ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );
		// no change
		*toF = *fromF;
	}

	//if ( print ) {
	//	if ( msg->bit == 0 ) {
	//		endBit = msg->readcount * 8 - GENTITYNUM_BITS;
	//	} else {
	//		endBit = ( msg->readcount - 1 ) * 8 + msg->bit - GENTITYNUM_BITS;
	//	}
	//	Com_Printf( " (%i bits)\n", endBit - startBit  );
	//}
}

//=========================================================================
//
// Multiple Master Servers (sources)
// Lets you: 
// a) connect to any master server, not just the forced defaults and
// b) lets you have up to five at any time.
//
//=========================================================================

typedef enum {
	NA_BOT,
	NA_BAD,					// an address lookup failed
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
	NA_IPX,
	NA_BROADCAST_IPX
} netadrtype_t;

typedef enum {
	NS_CLIENT,
	NS_SERVER
} netsrc_t;

typedef struct {
	netadrtype_t	type;

	byte	ip[4];
	byte	ipx[10];

	unsigned short	port;
} netadr_t;

#define CDECL_FUNCTION( R, N, A ) typedef R (__cdecl *N) A
CDECL_FUNCTION(void, NET_OOBPrint, (netsrc_t, netadr_t, const char *, ...));

static NET_OOBPrint NET_OutOfBandPrint = (NET_OOBPrint)0x00443100;

int		trap_Argc( void );
void	trap_Argv( int n, char *buffer, int bufferLength );
void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void	trap_RemoveCommand( const char *cmdName );

typedef void (*xcommand_t) (void);
void Cmd_AddCommand( const char *cmdName, xcommand_t function )
{
	// THIS IS USERCALL! can vary on mac systems
	__asm
	{
		pushad
		mov ebx, function
		mov eax, cmdName
		mov edx, 0x00436DA0
		call edx
		popad
	}
}

void Cmd_RemoveCommand( const char *cmdName )
{
	// THIS IS USERCALL! can vary on mac systems
	__asm
	{
		pushad
		mov edx, cmdName
		mov ecx, 0x00436E30
		call ecx
		popad
	}
}

extern char	*Cmd_Argv( int arg );
void (*NET_OutOfBandPrint)(netsrc_t source, netadr_t adr, const char *format, ...);
void CL_GlobalServers_f ( void )
{
	char command[1024];
	char *buffptr;
	int	count, i;
	netadr_t to;
	int *cls_nummplayerservers = (int *)0x008B4430;
	int *cls_pingUpdateSource = (int *)0x00913840;
	int *masterNum = (int*)(0x00913844);
	if(trap_Argc() < 3)
	{
		Com_Printf( "usage: globalservers <master# 0-1> <protocol> [keywords]\n");
		return;	
	}

	*masterNum = atoi(Cmd_Argv(1));

	Com_Printf( "Requesting servers from the master...\n");

	// This function appears totally nerfed from Q3.
	// In Q3, this had a AS_MPLAYERS. This is not
	// used in JA, rather it's forced to AS_GLOBAL (gj raven etc)
	// --eez
	{
		// THIS IS USERCALL! can vary on mac systems
		char *cvarBuffer = (char *)malloc(64);
		trap_Cvar_VariableStringBuffer( va("sv_master%i", (*masterNum > 0) ? *masterNum : 1), cvarBuffer, 64 );
		__asm
		{
			pushad
			pushfd
			push cvarBuffer
			lea ebx, to
			mov eax, 0x00443210
			call eax
			add esp,4
			popfd
			popad
		}
		*cls_nummplayerservers = -1;
		*cls_pingUpdateSource = 1;
		free(cvarBuffer);
	}

	to.type = NA_IP;
	to.port = 33905;

	// Construct the command
	sprintf(command, "getservers %s", Cmd_Argv(2));

	// tack on keywords
	buffptr = command + strlen( command );
	count   = trap_Argc();

	for (i=3; i<count; i++)
	{
		buffptr += sprintf( buffptr, " %s", Cmd_Argv(i) );
	}
	// if we are a demo, automatically add a "demo" keyword
	{
		char cvarBuffer[8];
		trap_Cvar_VariableStringBuffer( "fs_restrict", cvarBuffer, 8 );
		if(atoi(cvarBuffer))
		{
			buffptr += sprintf( buffptr, " demo" );
		}
	}
	// send it already!
	NET_OutOfBandPrint(NS_SERVER, to, command);

	*masterNum = 0;	// hackery, but necessary hackery
}

void CL_InitMultiMasterServer(void)
{
	// You need to call this upon UI_Init
	Cmd_RemoveCommand("globalservers");					// get rid of the old command so we can add it ourselves
	Cmd_AddCommand("globalservers", CL_GlobalServers_f);
}

void CL_ShutdownMultiMasterServer(void)
{
	Cmd_RemoveCommand("globalservers");
	Cmd_AddCommand("globalservers", (xcommand_t)0x00420B30);
}