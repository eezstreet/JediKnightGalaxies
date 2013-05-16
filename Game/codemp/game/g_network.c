#ifdef QAGAME
#include "g_local.h"
//#include "server.h"
#endif

networkState_t networkStates[MAX_CLIENTS];
networkState_t old_networkStates[MAX_CLIENTS];



// Jampded
//void (*MSG_ReadBits)(msg_t *msg, int bits) = (void(*)(msg_t *msg, int bits))0x4186D0;
void (*MSG_WriteByte)(msg_t *msg, int c) = (void(*)(msg_t *msg, int c))0x00418810;
void (*MSG_WriteBits)(msg_t *msg, int value, int bits) = (void(*)(msg_t *msg, int value, int bits))0x00418530;

int **svs_clients = (int**)0x00606224; // size of client_t = 332920
const int size = 332920/sizeof(int);

int *sv_snapshotCounter = (int*)0x00567F44;

void N_EmitExtraStates(msg_t *msg, clientSnapshot_t *from, clientSnapshot_t *to, qboolean lastClient);
void N_WriteDeltaExtrastate( msg_t *msg, extraState_t *from, extraState_t *to, qboolean force, int newnum, qboolean lastClient );

static int counter = 0;

//int oldsize = 0;
//======================================================================================================
// FUNCS NEEDED FOR SECOND ENTITYSTATE

void N_WriteDeltaNetworkstate( msg_t *msg, int *client, clientSnapshot_t *fromFrame, clientSnapshot_t *toFrame )
{
	int			i, lc;
	int			*fromF, *toF;
	netField_t	*field;
	int			numFields;
	float		fullFloat;
	int			trunc;
	int			clientNum = (client - (*svs_clients))/size;
	networkState_t *to = &level.clients[clientNum].ns;
	networkState_t *from = &old_networkStates[clientNum];

	// this is not C++ --eez
	static int counter = 0;

	//Com_Printf("Writing new to: %i", clientNum);


	numFields = numNetworkStateFields;

	lc = 0;
	for ( i = 0, field = networkStateFields ; i < numFields ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );
		if ( *fromF != *toF ) {
			lc = i+1;
		}
	}

	MSG_WriteByte( msg, lc );	// # of changes

	//oldsize += numFields - lc;


	for ( i = 0, field = networkStateFields ; i < lc ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );

		if ( *fromF == *toF ) {
			MSG_WriteBits( msg, 0, 1 );	// no change
			continue;
		}

		MSG_WriteBits( msg, 1, 1 );	// changed
//		pcount[i]++;

		// Update old one too, it's just being used here anyways.
		*fromF = *toF;

		if ( field->bits == 0 ) {
			// float
			fullFloat = *(float *)toF;
			trunc = (int)fullFloat;

			if ( trunc == fullFloat && trunc + FLOAT_INT_BIAS >= 0 && 
				trunc + FLOAT_INT_BIAS < ( 1 << FLOAT_INT_BITS ) ) {
				// send as small integer
				MSG_WriteBits( msg, 0, 1 );
				MSG_WriteBits( msg, trunc + FLOAT_INT_BIAS, FLOAT_INT_BITS );
			} else {
				// send as full floating point value
				MSG_WriteBits( msg, 1, 1 );
				MSG_WriteBits( msg, *toF, 32 );
			}
		} else {
			// integer
			MSG_WriteBits( msg, *toF, field->bits );
		}
	}
	if(level.clients[clientNum].pers.connected == CON_CONNECTED)
		counter++;

	N_EmitExtraStates(msg, fromFrame, toFrame, counter == level.numConnectedClients);

	if(counter == level.numConnectedClients)
		counter = 0;

}


// Hooks
#include <windows.h>
#define WRITE_POS 0x00444C3B
#define WRITE_ORIG_CALL 0x004449C0
#define WRITE_RETURN 0x00444C40



// jampded
// hook location: 0x00444C3B
// 00444C3B: E8 80 FD FF FF    call    SV_EmitPacketEntities
byte write_restore[] = { 0xe8, 0x80, 0xfd, 0xff, 0xff };
void __declspec(naked) _Hook_NetworkState_Write(void)
{ 
	__asm 
	{ 
		add esp, 4
		pushad
		pushfd

		push ebx	// frame
		push ebp	// oldframe
		push edi	// client
		push esi	// msg
		call N_WriteDeltaNetworkstate
		add esp, 16

		popfd
		popad
		
		mov	eax, WRITE_ORIG_CALL
		call eax
		mov ecx, WRITE_RETURN
		jmp ecx
	} 
}

void PatchAddress( unsigned int address, unsigned int destination)
{
	int ret, dummy;

	ret = VirtualProtect((LPVOID)address, 5, PAGE_EXECUTE_READWRITE, &dummy);
	*(unsigned char *)address = 0xE8;
	*(unsigned int *)(address+1) = destination - (address + 5);
	ret = VirtualProtect((LPVOID)address, 5, PAGE_EXECUTE_READ, NULL);
	
}


#define	MAX_PARSE_ENTITIES	2048
extraState_t	new_parseExtras[MAX_GENTITIES];
extraState_t	old_parseExtras[MAX_GENTITIES];

entityState_t **svs_snapshotEntities = (entityState_t**)0x00606230;
int *svs_numSnapshotEntities		= (int*)0x00606228;

void N_EmitExtraStates(msg_t *msg, clientSnapshot_t *from, clientSnapshot_t *to, qboolean lastClient)
{
	entityState_t	*oldent, *newent;
	extraState_t	*extraOld, *extraNew;
	int		oldindex, newindex;
	int		oldnum, newnum;
	int		from_num_entities;
	int i;

	//clientSnapshot_t *to = NULL;
	//clientSnapshot_t *from = NULL;


	// Basically here you can put it
	// for <ents>
	// if active
	//N_WriteDeltaExtrastate(msg, old, new, qfalse(iguess)); No change needed on clientside.

	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if(level.clients[i].pers.connected != CON_CONNECTED)
			continue;

		//extraOld = &old_parseExtras[newnum];

		N_WriteDeltaExtrastate(msg, &old_parseExtras[i], &g_entities[i].x, qfalse, i, lastClient);
	}
	//I meant outside :p
	MSG_WriteBits( msg, (MAX_GENTITIES-1), GENTITYNUM_BITS );	// end of packetentities
	// generate the delta update
	if ( !from ) {
		from_num_entities = 0;
	} else {
		from_num_entities = from->num_entities;
	}

	newent = NULL;
	oldent = NULL;
	extraNew = NULL;
	extraOld = NULL;
	newindex = 0;
	oldindex = 0;
	while ( newindex < to->num_entities || oldindex < from_num_entities )
	{
		if ( newindex >= to->num_entities ) {
			newnum = 9999;
		} else {
			// And the reason it doesn't do it for players is that this array here doesn't contain the numbers for players.
			// So i guess it ignores players when it is built. In Sv_BuildClientSnapshot. You can check it out in q3 to see if there is a reason.
			newent = &(*svs_snapshotEntities)[(to->first_entity+newindex) % *svs_numSnapshotEntities];
			extraNew = &g_entities[newent->number].x;
			newnum = newent->number;
		}

		if ( oldindex >= from_num_entities ) {
			oldnum = 9999;
		} else {
			oldent = &(*svs_snapshotEntities)[(from->first_entity+oldindex) % *svs_numSnapshotEntities];
			extraOld = &old_parseExtras[oldent->number];
			oldnum = oldent->number;
		}

		if ( newnum == oldnum ) {
			// delta update from old position
			// because the force parm is qfalse, this will not result
			// in any bytes being emited if the entity has not changed at all
			N_WriteDeltaExtrastate (msg, extraOld, extraNew, qfalse, newnum, lastClient );
			oldindex++;
			newindex++;
			continue;
		}

		if ( newnum < oldnum ) {
			// this is a new entity, send it from the baseline
			//N_WriteDeltaExtrastate (msg, &sv.svEntities[newnum].baseline, extraNew, qtrue );
			extraOld = &old_parseExtras[newnum];
			N_WriteDeltaExtrastate (msg, extraOld, extraNew, qtrue, newnum, lastClient );
			newindex++;
			continue;
		}

		if ( newnum > oldnum ) {
			// the old entity isn't present in the new message
			N_WriteDeltaExtrastate (msg, extraOld, NULL, qtrue, newnum, lastClient );
			oldindex++;
			continue;
		}
	}

	MSG_WriteBits( msg, (MAX_GENTITIES-1), GENTITYNUM_BITS );	// end of packetentities
}


void N_WriteDeltaExtrastate( msg_t *msg, extraState_t *from, extraState_t *to, qboolean force, int newnum, qboolean lastClient )
{
	int			i, lc;
	int			numFields;
	netField_t	*field;
	int			trunc;
	float		fullFloat;
	int			*fromF, *toF;

	numFields = sizeof(extraStateFields)/sizeof(extraStateFields[0]);

	// a NULL to is a delta remove message
	if ( to == NULL ) {
		if ( from == NULL ) {
			return;
		}
		MSG_WriteBits( msg, newnum, GENTITYNUM_BITS );
		MSG_WriteBits( msg, 1, 1 );
		return;
	}

	if ( newnum < 0 || newnum >= MAX_GENTITIES ) {
		Com_Error (ERR_FATAL, "N_WriteDeltaExtrastate: Bad entity number: %i", newnum );
	}

	lc = 0;
	// build the change vector as bytes so it is endien independent
	for ( i = 0, field = extraStateFields ; i < numFields ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );
		if ( *fromF != *toF ) {
			lc = i+1;
		}
	}

	if ( lc == 0 ) {
		// nothing at all changed
		if ( !force ) {
			return;		// nothing at all
		}
		// write two bits for no change
		MSG_WriteBits( msg, newnum, GENTITYNUM_BITS );
		MSG_WriteBits( msg, 0, 1 );		// not removed
		MSG_WriteBits( msg, 0, 1 );		// no delta
		return;
	}

	MSG_WriteBits( msg, newnum, GENTITYNUM_BITS );
	MSG_WriteBits( msg, 0, 1 );			// not removed
	MSG_WriteBits( msg, 1, 1 );			// we have a delta

	MSG_WriteByte( msg, lc );	// # of changes


	for ( i = 0, field = extraStateFields ; i < lc ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );

		if ( *fromF == *toF ) {
			MSG_WriteBits( msg, 0, 1 );	// no change
			*fromF = *toF;

			continue;
		}

		MSG_WriteBits( msg, 1, 1 );	// changed

		if(lastClient)
			*fromF = *toF; // Update old to have new settings. This is all the change needed on server.

		if ( field->bits == 0 ) {
			// float
			fullFloat = *(float *)toF;
			trunc = (int)fullFloat;

			if (fullFloat == 0.0f) {
					MSG_WriteBits( msg, 0, 1 );
					//oldsize += FLOAT_INT_BITS;
			} else {
				MSG_WriteBits( msg, 1, 1 );
				if ( trunc == fullFloat && trunc + FLOAT_INT_BIAS >= 0 && 
					trunc + FLOAT_INT_BIAS < ( 1 << FLOAT_INT_BITS ) ) {
					// send as small integer
					MSG_WriteBits( msg, 0, 1 );
					MSG_WriteBits( msg, trunc + FLOAT_INT_BIAS, FLOAT_INT_BITS );
				} else {
					// send as full floating point value
					MSG_WriteBits( msg, 1, 1 );
					MSG_WriteBits( msg, *toF, 32 );
				}
			}
		} else {
			if (*toF == 0) {
				MSG_WriteBits( msg, 0, 1 );
			} else {
				MSG_WriteBits( msg, 1, 1 );
				// integer
				MSG_WriteBits( msg, *toF, field->bits );
			}
		}
	}
}

void N_Init()
{
	int i;
	PatchAddress( WRITE_POS, (unsigned int)_Hook_NetworkState_Write );
//	PatchAddress( READ_POS, (unsigned int)_Hook_NetworkState_Read );
	for(i = 0; i < MAX_GENTITIES; i++)
		old_parseExtras[i].number = g_entities[i].x.number = i;
}

void N_Clear()
{
	int ret, dummy;

	ret = VirtualProtect((LPVOID)WRITE_POS, 5, PAGE_EXECUTE_READWRITE, &dummy);
	memcpy((LPVOID)WRITE_POS, write_restore, 5);
	ret = VirtualProtect((LPVOID)WRITE_POS, 5, PAGE_EXECUTE_READ, NULL);
}

void N_Change1(int clientNum, float num)
{
	//new_parseExtras[clientNum].testFloat = num;
	g_entities[clientNum].x.testFloat = num;
}

void N_Change2(int clientNum, int num)
{
	//new_parseExtras[clientNum].testInt = num;
	g_entities[clientNum].x.testInt = num;
}