// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"
#include "jkg_threading.h"

// this file is only included when building a dll
// g_syscalls.asm is included instead when building a qvm

static int (QDECL *syscall)( int arg, ... ) = (int (QDECL *)( int, ...))-1;

#include "../namespace_begin.h"
/*
#ifdef __linux__
extern "C" {
#endif
*/

#ifdef _WIN32
// Define engine-side com_error
void (*iCom_Error)(int level, const char *error) = (void (*)(int, const char*))0x437290;
#endif

void dllEntry( int (QDECL *syscallptr)( int arg,... ) ) {
	syscall = syscallptr;
}
/*
#ifdef __linux__
}
#endif
*/

int PASSFLOAT( float x ) {
	float	floatTemp;
	floatTemp = x;
	return *(int *)&floatTemp;
}

void	trap_Printf( const char *fmt ) {
	JKG_Printf (syscall, fmt);
}

void	trap_Error( const char *fmt ) {
	syscall( G_ERROR, fmt );
}

int		trap_Milliseconds( void ) {
	return syscall( G_MILLISECONDS ); 
}


//rww - precision timer funcs... -ALWAYS- call end after start with supplied ptr, or you'll get a nasty memory leak.
//not that you should be using these outside of debug anyway.. because you shouldn't be. So don't.

//Start should be suppled with a pointer to an empty pointer (e.g. void *blah; trap_PrecisionTimer_Start(&blah);),
//the empty pointer will be filled with an exe address to our timer (this address means nothing in vm land however).
//You must pass this pointer back unmodified to the timer end func.
void trap_PrecisionTimer_Start(void **theNewTimer)
{
	syscall(G_PRECISIONTIMER_START, theNewTimer);
}

//If you're using the above example, the appropriate call for this is int result = trap_PrecisionTimer_End(blah);
int trap_PrecisionTimer_End(void *theTimer)
{
	return syscall(G_PRECISIONTIMER_END, theTimer);
}

void	trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags ) {
	syscall( G_CVAR_REGISTER, cvar, var_name, value, flags );
}

void	trap_Cvar_Update( vmCvar_t *cvar ) {
	syscall( G_CVAR_UPDATE, cvar );
}

void trap_Cvar_Set( const char *var_name, const char *value ) {
	syscall( G_CVAR_SET, var_name, value );
}

int trap_Cvar_VariableIntegerValue( const char *var_name ) {
	return syscall( G_CVAR_VARIABLE_INTEGER_VALUE, var_name );
}

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	syscall( G_CVAR_VARIABLE_STRING_BUFFER, var_name, buffer, bufsize );
}

int		trap_Argc( void ) {
	return syscall( G_ARGC );
}

void	trap_Argv( int n, char *buffer, int bufferLength ) {
	syscall( G_ARGV, n, buffer, bufferLength );
}

int		trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return syscall( G_FS_FOPEN_FILE, qpath, f, mode );
}

void	trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	syscall( G_FS_READ, buffer, len, f );
}

void	trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	syscall( G_FS_WRITE, buffer, len, f );
}

void	trap_FS_FCloseFile( fileHandle_t f ) {
	syscall( G_FS_FCLOSE_FILE, f );
}

void	trap_SendConsoleCommand( int exec_when, const char *text ) {
	syscall( G_SEND_CONSOLE_COMMAND, exec_when, text );
}

void trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t,
						 playerState_t *clients, int sizeofGClient ) {
	syscall( G_LOCATE_GAME_DATA, gEnts, numGEntities, sizeofGEntity_t, clients, sizeofGClient );
}

void trap_DropClient( int clientNum, const char *reason ) {
	syscall( G_DROP_CLIENT, clientNum, reason );
}

void trap_SendServerCommand( int clientNum, const char *cmd ) {
	// Jedi Knight Galaxies overside command crash fix
	int i;
	char s;
	char *cmd2 = (char *)cmd;

#ifdef __MMO__
	G_Printf("clientNum %i sent %s.\n", clientNum, cmd);
#endif

	if ((i = strlen(cmd)) > 1022) {
		G_Printf("WARNING: Oversize cmd in trap_SendServerCommand(%i, len = %i), truncating!\n", clientNum, i);
		// The command is oversize, so cut if off after 1022 bytes
		// Since cmd is const char, we'll be inserting a null and restoring it afterwards
		s = cmd[1021];
		cmd2[1021] = 0;
		syscall( G_SEND_SERVER_COMMAND, clientNum, cmd );
		cmd2[1021] = s;

	} else {
		syscall( G_SEND_SERVER_COMMAND, clientNum, cmd );
	}
}

void trap_SetConfigstring( int num, const char *string ) {
	syscall( G_SET_CONFIGSTRING, num, string );
}

void trap_GetConfigstring( int num, char *buffer, int bufferSize ) {
	syscall( G_GET_CONFIGSTRING, num, buffer, bufferSize );
}

void trap_GetUserinfo( int num, char *buffer, int bufferSize ) {
	syscall( G_GET_USERINFO, num, buffer, bufferSize );
}

void trap_SetUserinfo( int num, const char *buffer ) {
	syscall( G_SET_USERINFO, num, buffer );
}

void trap_GetServerinfo( char *buffer, int bufferSize ) {
	syscall( G_GET_SERVERINFO, buffer, bufferSize );
}

//server culling to reduce traffic on open maps -rww
void trap_SetServerCull(float cullDistance)
{
	syscall(G_SET_SERVER_CULL, PASSFLOAT(cullDistance));
}

void trap_SetBrushModel( gentity_t *ent, const char *name ) {
	syscall( G_SET_BRUSH_MODEL, ent, name );
}

void trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	syscall( G_TRACE, results, start, mins, maxs, end, passEntityNum, contentmask, 0, 10 );
}

//g2TraceType 0 is no g2 col, 1 is collision against anything not EF_DEAD, 2 is collision against all.
void trap_G2Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask, int g2TraceType, int traceLod ) {
	syscall( G_G2TRACE, results, start, mins, maxs, end, passEntityNum, contentmask, g2TraceType, traceLod );
}

int trap_PointContents( const vec3_t point, int passEntityNum ) {
	return syscall( G_POINT_CONTENTS, point, passEntityNum );
}


qboolean trap_InPVS( const vec3_t p1, const vec3_t p2 ) {
	return syscall( G_IN_PVS, p1, p2 );
}

qboolean trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 ) {
	return syscall( G_IN_PVS_IGNORE_PORTALS, p1, p2 );
}

void trap_AdjustAreaPortalState( gentity_t *ent, qboolean open ) {
	syscall( G_ADJUST_AREA_PORTAL_STATE, ent, open );
}

qboolean trap_AreasConnected( int area1, int area2 ) {
	return syscall( G_AREAS_CONNECTED, area1, area2 );
}

void trap_LinkEntity( gentity_t *ent ) {
	if (!ent->isLogical) {	// Can't link logical entities
		syscall( G_LINKENTITY, ent );
	}
}

void trap_UnlinkEntity( gentity_t *ent ) {
	if (!ent->isLogical) {	// Can't unlink logical entities
		syscall( G_UNLINKENTITY, ent );
	}
}

int trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *list, int maxcount ) {
	return syscall( G_ENTITIES_IN_BOX, mins, maxs, list, maxcount );
}

qboolean trap_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ) {
	return syscall( G_ENTITY_CONTACT, mins, maxs, ent );
}

int trap_BotAllocateClient( void ) {
	return syscall( G_BOT_ALLOCATE_CLIENT );
}

void trap_BotFreeClient( int clientNum ) {
	syscall( G_BOT_FREE_CLIENT, clientNum );
}

void trap_GetUsercmd( int clientNum, usercmd_t *cmd ) {
	syscall( G_GET_USERCMD, clientNum, cmd );
}

qboolean trap_GetEntityToken( char *buffer, int bufferSize ) {
	return syscall( G_GET_ENTITY_TOKEN, buffer, bufferSize );
}

void trap_SiegePersSet(siegePers_t *pers)
{
	syscall(G_SIEGEPERSSET, pers);
}
void trap_SiegePersGet(siegePers_t *pers)
{
	syscall(G_SIEGEPERSGET, pers);
}

int trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize ) {
	return syscall( G_FS_GETFILELIST, path, extension, listbuf, bufsize );
}

int trap_DebugPolygonCreate(int color, int numPoints, vec3_t *points) {
	return syscall( G_DEBUG_POLYGON_CREATE, color, numPoints, points );
}

void trap_DebugPolygonDelete(int id) {
	syscall( G_DEBUG_POLYGON_DELETE, id );
}

int trap_RealTime( qtime_t *qtime ) {
	return syscall( G_REAL_TIME, qtime );
}

void trap_SnapVector( float *v ) {
	syscall( G_SNAPVECTOR, v );
}

void trap_TraceCapsule( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	syscall( G_TRACECAPSULE, results, start, mins, maxs, end, passEntityNum, contentmask, 0, 10 );
}

qboolean trap_EntityContactCapsule( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ) {
	return syscall( G_ENTITY_CONTACTCAPSULE, mins, maxs, ent );
}

//qboolean trap_SP_RegisterServer( const char *package ) 
//{
//	return syscall( SP_REGISTER_SERVER_CMD, package );
//}

int trap_SP_GetStringTextString(const char *text, char *buffer, int bufferLength)
{
	return syscall( SP_GETSTRINGTEXTSTRING, text, buffer, bufferLength );
}

qboolean trap_ROFF_Clean( void ) 
{
	return syscall( G_ROFF_CLEAN );
}

void trap_ROFF_UpdateEntities( void ) 
{
	syscall( G_ROFF_UPDATE_ENTITIES );
}

int trap_ROFF_Cache( char *file ) 
{
	return syscall( G_ROFF_CACHE, file );
}

qboolean trap_ROFF_Play( int entID, int roffID, qboolean doTranslation ) 
{
	return syscall( G_ROFF_PLAY, entID, roffID, doTranslation );
}

qboolean trap_ROFF_Purge_Ent( int entID ) 
{
	return syscall( G_ROFF_PURGE_ENT, entID );
}

//rww - dynamic vm memory allocation!
void trap_TrueMalloc(void **ptr, int size)
{
	syscall(G_TRUEMALLOC, ptr, size);
}

void trap_TrueFree(void **ptr)
{
	syscall(G_TRUEFREE, ptr);
}

//rww - icarus traps (modified to properly handle logical entities
int trap_ICARUS_RunScript( gentity_t *ent, const char *name )
{
	if (!ent->isLogical) // Disable icarus for logical entities
		return syscall(G_ICARUS_RUNSCRIPT, ent, name);
	return 0;
}

qboolean trap_ICARUS_RegisterScript( const char *name, qboolean bCalledDuringInterrogate)
{
	return syscall(G_ICARUS_REGISTERSCRIPT, name, bCalledDuringInterrogate);
}

void trap_ICARUS_Init( void )
{
	syscall(G_ICARUS_INIT);
}

qboolean trap_ICARUS_ValidEnt( gentity_t *ent )
{
	if (!ent->isLogical) // Disable icarus for logical entities
		return syscall(G_ICARUS_VALIDENT, ent);
	return qfalse;
}

qboolean trap_ICARUS_IsInitialized( int entID )
{
	if (entID < 1023) // Disable icarus for logical entities
		return syscall(G_ICARUS_ISINITIALIZED, entID);
	return qfalse;
}

qboolean trap_ICARUS_MaintainTaskManager( int entID )
{
	if (entID < 1023) // Disable icarus for logical entities
		return syscall(G_ICARUS_MAINTAINTASKMANAGER, entID);
	return qfalse;
}

qboolean trap_ICARUS_IsRunning( int entID )
{
	if (entID < 1023) // Disable icarus for logical entities
		return syscall(G_ICARUS_ISRUNNING, entID);
	return qfalse;
}

qboolean trap_ICARUS_TaskIDPending(gentity_t *ent, int taskID)
{
	if (!ent->isLogical) // Disable icarus for logical entities
		return syscall(G_ICARUS_TASKIDPENDING, ent, taskID);
	return qfalse;
}

void trap_ICARUS_InitEnt( gentity_t *ent )
{
	if (!ent->isLogical) // Disable icarus for logical entities
		syscall(G_ICARUS_INITENT, ent);
}

void trap_ICARUS_FreeEnt( gentity_t *ent )
{
	if (!ent->isLogical) // Disable icarus for logical entities
		syscall(G_ICARUS_FREEENT, ent);
}

void trap_ICARUS_AssociateEnt( gentity_t *ent )
{
	if (!ent->isLogical) // Disable icarus for logical entities
		syscall(G_ICARUS_ASSOCIATEENT, ent);
}

void trap_ICARUS_Shutdown( void )
{
	syscall(G_ICARUS_SHUTDOWN);
}

void trap_ICARUS_TaskIDSet(gentity_t *ent, int taskType, int taskID)
{
	if (!ent->isLogical) // Disable icarus for logical entities
		syscall(G_ICARUS_TASKIDSET, ent, taskType, taskID);
}

void trap_ICARUS_TaskIDComplete(gentity_t *ent, int taskType)
{
	if (!ent->isLogical) // Disable icarus for logical entities
		syscall(G_ICARUS_TASKIDCOMPLETE, ent, taskType);
}

void trap_ICARUS_SetVar(int taskID, int entID, const char *type_name, const char *data)
{
	syscall(G_ICARUS_SETVAR, taskID, entID, type_name, data);
}

int trap_ICARUS_VariableDeclared(const char *type_name)
{
	return syscall(G_ICARUS_VARIABLEDECLARED, type_name);
}

int trap_ICARUS_GetFloatVariable( const char *name, float *value )
{
	return syscall(G_ICARUS_GETFLOATVARIABLE, name, value);
}

int trap_ICARUS_GetStringVariable( const char *name, const char *value )
{
	return syscall(G_ICARUS_GETSTRINGVARIABLE, name, value);
}

int trap_ICARUS_GetVectorVariable( const char *name, const vec3_t value )
{
	return syscall(G_ICARUS_GETVECTORVARIABLE, name, value);
}

//rww - BEGIN NPC NAV TRAPS
void trap_Nav_Init( void )
{
	syscall(G_NAV_INIT);
}

void trap_Nav_Free( void )
{
	syscall(G_NAV_FREE);
}

qboolean trap_Nav_Load( const char *filename, int checksum )
{
	return syscall(G_NAV_LOAD, filename, checksum);
}

qboolean trap_Nav_Save( const char *filename, int checksum )
{
	return syscall(G_NAV_SAVE, filename, checksum);
}

int trap_Nav_AddRawPoint( vec3_t point, int flags, int radius )
{
	return syscall(G_NAV_ADDRAWPOINT, point, flags, radius);
}

void trap_Nav_CalculatePaths( qboolean recalc ) //recalc = qfalse
{
	syscall(G_NAV_CALCULATEPATHS, recalc);
}

void trap_Nav_HardConnect( int first, int second )
{
	syscall(G_NAV_HARDCONNECT, first, second);
}

void trap_Nav_ShowNodes( void )
{
	syscall(G_NAV_SHOWNODES);
}

void trap_Nav_ShowEdges( void )
{
	syscall(G_NAV_SHOWEDGES);
}

void trap_Nav_ShowPath( int start, int end )
{
	syscall(G_NAV_SHOWPATH, start, end);
}

int trap_Nav_GetNearestNode( gentity_t *ent, int lastID, int flags, int targetID )
{
	return syscall(G_NAV_GETNEARESTNODE, ent, lastID, flags, targetID);
}

int trap_Nav_GetBestNode( int startID, int endID, int rejectID ) //rejectID = NODE_NONE
{
	return syscall(G_NAV_GETBESTNODE, startID, endID, rejectID);
}

int trap_Nav_GetNodePosition( int nodeID, vec3_t out )
{
	return syscall(G_NAV_GETNODEPOSITION, nodeID, out);
}

int trap_Nav_GetNodeNumEdges( int nodeID )
{
	return syscall(G_NAV_GETNODENUMEDGES, nodeID);
}

int trap_Nav_GetNodeEdge( int nodeID, int edge )
{
	return syscall(G_NAV_GETNODEEDGE, nodeID, edge);
}

int trap_Nav_GetNumNodes( void )
{
	return syscall(G_NAV_GETNUMNODES);
}
	
qboolean trap_Nav_Connected( int startID, int endID )
{
	return syscall(G_NAV_CONNECTED, startID, endID);
}

int trap_Nav_GetPathCost( int startID, int endID )
{
	return syscall(G_NAV_GETPATHCOST, startID, endID);
}

int trap_Nav_GetEdgeCost( int startID, int endID )
{
	return syscall(G_NAV_GETEDGECOST, startID, endID);
}

int trap_Nav_GetProjectedNode( vec3_t origin, int nodeID )
{
	return syscall(G_NAV_GETPROJECTEDNODE, origin, nodeID);
}

void trap_Nav_CheckFailedNodes( gentity_t *ent )
{
	syscall(G_NAV_CHECKFAILEDNODES, ent);
}

void trap_Nav_AddFailedNode( gentity_t *ent, int nodeID )
{
	syscall(G_NAV_ADDFAILEDNODE, ent, nodeID);
}

qboolean trap_Nav_NodeFailed( gentity_t *ent, int nodeID )
{
	return syscall(G_NAV_NODEFAILED, ent, nodeID);
}

qboolean trap_Nav_NodesAreNeighbors( int startID, int endID )
{
	return syscall(G_NAV_NODESARENEIGHBORS, startID, endID);
}

void trap_Nav_ClearFailedEdge( failedEdge_t	*failedEdge )
{
	syscall(G_NAV_CLEARFAILEDEDGE, failedEdge);
}

void trap_Nav_ClearAllFailedEdges( void )
{
	syscall(G_NAV_CLEARALLFAILEDEDGES);
}

int trap_Nav_EdgeFailed( int startID, int endID )
{
	return syscall(G_NAV_EDGEFAILED, startID, endID);
}

void trap_Nav_AddFailedEdge( int entID, int startID, int endID )
{
	syscall(G_NAV_ADDFAILEDEDGE, entID, startID, endID);
}

qboolean trap_Nav_CheckFailedEdge( failedEdge_t *failedEdge )
{
	return syscall(G_NAV_CHECKFAILEDEDGE, failedEdge);
}

void trap_Nav_CheckAllFailedEdges( void )
{
	syscall(G_NAV_CHECKALLFAILEDEDGES);
}

qboolean trap_Nav_RouteBlocked( int startID, int testEdgeID, int endID, int rejectRank )
{
	return syscall(G_NAV_ROUTEBLOCKED, startID, testEdgeID, endID, rejectRank);
}

int trap_Nav_GetBestNodeAltRoute( int startID, int endID, int *pathCost, int rejectID ) //rejectID = NODE_NONE
{
	return syscall(G_NAV_GETBESTNODEALTROUTE, startID, endID, pathCost, rejectID);
}

int trap_Nav_GetBestNodeAltRoute2( int startID, int endID, int rejectID ) //rejectID = NODE_NONE
{
	return syscall(G_NAV_GETBESTNODEALT2, startID, endID, rejectID);
}
	
int trap_Nav_GetBestPathBetweenEnts( gentity_t *ent, gentity_t *goal, int flags )
{
	return syscall(G_NAV_GETBESTPATHBETWEENENTS, ent, goal, flags);
}

int	trap_Nav_GetNodeRadius( int nodeID )
{
	return syscall(G_NAV_GETNODERADIUS, nodeID);
}

void trap_Nav_CheckBlockedEdges( void )
{
	syscall(G_NAV_CHECKBLOCKEDEDGES);
}

void trap_Nav_ClearCheckedNodes( void )
{
	syscall(G_NAV_CLEARCHECKEDNODES);
}

int trap_Nav_CheckedNode(int wayPoint, int ent) //return int was byte
{
	return syscall(G_NAV_CHECKEDNODE, wayPoint, ent);
}

void trap_Nav_SetCheckedNode(int wayPoint, int ent, int value) //int value was byte value
{
	syscall(G_NAV_SETCHECKEDNODE, wayPoint, ent, value);
}

void trap_Nav_FlagAllNodes( int newFlag )
{
	syscall(G_NAV_FLAGALLNODES, newFlag);
}

qboolean trap_Nav_GetPathsCalculated(void)
{
	return syscall(G_NAV_GETPATHSCALCULATED);
}

void trap_Nav_SetPathsCalculated(qboolean newVal)
{
	syscall(G_NAV_SETPATHSCALCULATED, newVal);
}
//rww - END NPC NAV TRAPS

void trap_SV_RegisterSharedMemory(char *memory)
{
	syscall(G_SET_SHARED_BUFFER, memory);
}

// BotLib traps start here
int trap_BotLibSetup( void ) {
	return syscall( BOTLIB_SETUP );
}

int trap_BotLibShutdown( void ) {
	return syscall( BOTLIB_SHUTDOWN );
}

int trap_BotGetSnapshotEntity( int clientNum, int sequence ) {
	return syscall( BOTLIB_GET_SNAPSHOT_ENTITY, clientNum, sequence );
}

int trap_BotGetServerCommand(int clientNum, char *message, int size) {
	return syscall( BOTLIB_GET_CONSOLE_MESSAGE, clientNum, message, size );
}

void trap_BotUserCommand(int clientNum, usercmd_t *ucmd) {
	syscall( BOTLIB_USER_COMMAND, clientNum, ucmd );
}

void trap_AAS_EntityInfo(int entnum, void /* struct aas_entityinfo_s */ *info) {
	syscall( BOTLIB_AAS_ENTITY_INFO, entnum, info );
}

void trap_EA_Attack(int client) {
	syscall( BOTLIB_EA_ATTACK, client );
}

void trap_EA_Alt_Attack(int client) {
	syscall( BOTLIB_EA_ALT_ATTACK, client );
}

void trap_EA_ForcePower(int client) {
	syscall( BOTLIB_EA_FORCEPOWER, client );
}

void trap_EA_Use(int client) {
	syscall( BOTLIB_EA_USE, client );
}

void trap_EA_Crouch(int client) {
	syscall( BOTLIB_EA_CROUCH, client );
}

void trap_EA_MoveUp(int client) {
	syscall( BOTLIB_EA_MOVE_UP, client );
}

void trap_EA_MoveDown(int client) {
	syscall( BOTLIB_EA_MOVE_DOWN, client );
}

void trap_EA_MoveForward(int client) {
	syscall( BOTLIB_EA_MOVE_FORWARD, client );
}

void trap_EA_MoveBack(int client) {
	syscall( BOTLIB_EA_MOVE_BACK, client );
}

void trap_EA_MoveLeft(int client) {
	syscall( BOTLIB_EA_MOVE_LEFT, client );
}

void trap_EA_MoveRight(int client) {
	syscall( BOTLIB_EA_MOVE_RIGHT, client );
}

void trap_EA_SelectWeapon(int client, int weapon) {
	syscall( BOTLIB_EA_SELECT_WEAPON, client, weapon );
}

void trap_EA_Jump(int client) {
	syscall( BOTLIB_EA_JUMP, client );
}

void trap_EA_DelayedJump(int client) {
	syscall( BOTLIB_EA_DELAYED_JUMP, client );
}

void trap_EA_Move(int client, vec3_t dir, float speed) {
	syscall( BOTLIB_EA_MOVE, client, dir, PASSFLOAT(speed) );
}

void trap_EA_View(int client, vec3_t viewangles) {
	syscall( BOTLIB_EA_VIEW, client, viewangles );
}

void trap_EA_GetInput(int client, float thinktime, void /* struct bot_input_s */ *input) {
	syscall( BOTLIB_EA_GET_INPUT, client, PASSFLOAT(thinktime), input );
}

void trap_EA_ResetInput(int client) {
	syscall( BOTLIB_EA_RESET_INPUT, client );
}

void trap_BotResetGoalState(int goalstate) {
	syscall( BOTLIB_AI_RESET_GOAL_STATE, goalstate );
}

void trap_BotResetAvoidGoals(int goalstate) {
	syscall( BOTLIB_AI_RESET_AVOID_GOALS, goalstate );
}

void trap_BotUpdateEntityItems(void) {
	syscall( BOTLIB_AI_UPDATE_ENTITY_ITEMS );
}

int trap_BotAllocGoalState(int state) {
	return syscall( BOTLIB_AI_ALLOC_GOAL_STATE, state );
}

void trap_BotFreeGoalState(int handle) {
	syscall( BOTLIB_AI_FREE_GOAL_STATE, handle );
}

void trap_BotResetMoveState(int movestate) {
	syscall( BOTLIB_AI_RESET_MOVE_STATE, movestate );
}

void trap_BotResetAvoidReach(int movestate) {
	syscall( BOTLIB_AI_RESET_AVOID_REACH, movestate );
}

int trap_BotAllocMoveState(void) {
	return syscall( BOTLIB_AI_ALLOC_MOVE_STATE );
}

void trap_BotFreeMoveState(int handle) {
	syscall( BOTLIB_AI_FREE_MOVE_STATE, handle );
}

int trap_BotAllocWeaponState(void) {
	return syscall( BOTLIB_AI_ALLOC_WEAPON_STATE );
}

void trap_BotFreeWeaponState(int weaponstate) {
	syscall( BOTLIB_AI_FREE_WEAPON_STATE, weaponstate );
}

void trap_BotResetWeaponState(int weaponstate) {
	syscall( BOTLIB_AI_RESET_WEAPON_STATE, weaponstate );
}

int trap_PC_LoadSource( const char *filename ) {
	return syscall( BOTLIB_PC_LOAD_SOURCE, filename );
}

int trap_PC_FreeSource( int handle ) {
	return syscall( BOTLIB_PC_FREE_SOURCE, handle );
}

int trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	return syscall( BOTLIB_PC_READ_TOKEN, handle, pc_token );
}

int trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	return syscall( BOTLIB_PC_SOURCE_FILE_AND_LINE, handle, filename, line );
}


/*
Ghoul2 Insert Start
*/
qhandle_t trap_R_RegisterSkin( const char *name )
{
	return syscall( G_R_REGISTERSKIN, name );
}

// CG Specific API calls
void trap_G2_ListModelBones(void *ghlInfo, int frame)
{
	syscall( G_G2_LISTBONES, ghlInfo, frame);
}

void trap_G2_ListModelSurfaces(void *ghlInfo)
{
	syscall( G_G2_LISTSURFACES, ghlInfo);
}

qboolean trap_G2_HaveWeGhoul2Models(void *ghoul2)
{
	return (qboolean)(syscall(G_G2_HAVEWEGHOULMODELS, ghoul2));
}

void trap_G2_SetGhoul2ModelIndexes(void *ghoul2, qhandle_t *modelList, qhandle_t *skinList)
{
	syscall( G_G2_SETMODELS, ghoul2, modelList, skinList);
}

qboolean trap_G2API_GetBoltMatrix(void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix,
								const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale)
{
	return (qboolean)(syscall(G_G2_GETBOLT, ghoul2, modelIndex, boltIndex, matrix, angles, position, frameNum, modelList, scale));
}

qboolean trap_G2API_GetBoltMatrix_NoReconstruct(void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix,
								const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale)
{ //Same as above but force it to not reconstruct the skeleton before getting the bolt position
	return (qboolean)(syscall(G_G2_GETBOLT_NOREC, ghoul2, modelIndex, boltIndex, matrix, angles, position, frameNum, modelList, scale));
}

qboolean trap_G2API_GetBoltMatrix_NoRecNoRot(void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix,
								const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale)
{ //Same as above but force it to not reconstruct the skeleton before getting the bolt position
	return (qboolean)(syscall(G_G2_GETBOLT_NOREC_NOROT, ghoul2, modelIndex, boltIndex, matrix, angles, position, frameNum, modelList, scale));
}

int trap_G2API_InitGhoul2Model(void **ghoul2Ptr, const char *fileName, int modelIndex, qhandle_t customSkin,
						  qhandle_t customShader, int modelFlags, int lodBias)
{
	return syscall(G_G2_INITGHOUL2MODEL, ghoul2Ptr, fileName, modelIndex, customSkin, customShader, modelFlags, lodBias);
}

qboolean trap_G2API_SetSkin(void *ghoul2, int modelIndex, qhandle_t customSkin, qhandle_t renderSkin)
{
	return syscall(G_G2_SETSKIN, ghoul2, modelIndex, customSkin, renderSkin);
}

int trap_G2API_Ghoul2Size ( void* ghlInfo )
{
	return syscall(G_G2_SIZE, ghlInfo );
}

int trap_G2API_AddBolt(void *ghoul2, int modelIndex, const char *boneName)
{
	return syscall(G_G2_ADDBOLT, ghoul2, modelIndex, boneName);
}

void trap_G2API_SetBoltInfo(void *ghoul2, int modelIndex, int boltInfo)
{
	syscall(G_G2_SETBOLTINFO, ghoul2, modelIndex, boltInfo);
}

qboolean trap_G2API_SetBoneAngles(void *ghoul2, int modelIndex, const char *boneName, const vec3_t angles, const int flags,
								const int up, const int right, const int forward, qhandle_t *modelList,
								int blendTime , int currentTime )
{
	return (syscall(G_G2_ANGLEOVERRIDE, ghoul2, modelIndex, boneName, angles, flags, up, right, forward, modelList, blendTime, currentTime));
}

qboolean trap_G2API_SetBoneAnim(void *ghoul2, const int modelIndex, const char *boneName, const int startFrame, const int endFrame,
							  const int flags, const float animSpeed, const int currentTime, const float setFrame , const int blendTime )
{
	return syscall(G_G2_PLAYANIM, ghoul2, modelIndex, boneName, startFrame, endFrame, flags, PASSFLOAT(animSpeed), currentTime, PASSFLOAT(setFrame), blendTime);
}

qboolean trap_G2API_GetBoneAnim(void *ghoul2, const char *boneName, const int currentTime, float *currentFrame,
						   int *startFrame, int *endFrame, int *flags, float *animSpeed, int *modelList, const int modelIndex)
{
	return syscall(G_G2_GETBONEANIM, ghoul2, boneName, currentTime, currentFrame, startFrame, endFrame, flags, animSpeed, modelList, modelIndex);
}

void trap_G2API_GetGLAName(void *ghoul2, int modelIndex, char *fillBuf)
{
	syscall(G_G2_GETGLANAME, ghoul2, modelIndex, fillBuf);
}

int trap_G2API_CopyGhoul2Instance(void *g2From, void *g2To, int modelIndex)
{
	return syscall(G_G2_COPYGHOUL2INSTANCE, g2From, g2To, modelIndex);
}

void trap_G2API_CopySpecificGhoul2Model(void *g2From, int modelFrom, void *g2To, int modelTo)
{
	syscall(G_G2_COPYSPECIFICGHOUL2MODEL, g2From, modelFrom, g2To, modelTo);
}

void trap_G2API_DuplicateGhoul2Instance(void *g2From, void **g2To)
{
	syscall(G_G2_DUPLICATEGHOUL2INSTANCE, g2From, g2To);
}

qboolean trap_G2API_HasGhoul2ModelOnIndex(void *ghlInfo, int modelIndex)
{
	return syscall(G_G2_HASGHOUL2MODELONINDEX, ghlInfo, modelIndex);
}

qboolean trap_G2API_RemoveGhoul2Model(void *ghlInfo, int modelIndex)
{
	return syscall(G_G2_REMOVEGHOUL2MODEL, ghlInfo, modelIndex);
}

qboolean trap_G2API_RemoveGhoul2Models(void *ghlInfo)
{
	return syscall(G_G2_REMOVEGHOUL2MODELS, ghlInfo);
}

void trap_G2API_CleanGhoul2Models(void **ghoul2Ptr)
{
	syscall(G_G2_CLEANMODELS, ghoul2Ptr);
}

void trap_G2API_CollisionDetect ( 
	CollisionRecord_t *collRecMap, 
	void* ghoul2, 
	const vec3_t angles, 
	const vec3_t position,
	int frameNumber, 
	int entNum, 
	vec3_t rayStart, 
	vec3_t rayEnd, 
	vec3_t scale, 
	int traceFlags, 
	int useLod,
	float fRadius
	)
{
	syscall ( G_G2_COLLISIONDETECT, collRecMap, ghoul2, angles, position, frameNumber, entNum, rayStart, rayEnd, scale, traceFlags, useLod, PASSFLOAT(fRadius) );
}

void trap_G2API_CollisionDetectCache ( 
	CollisionRecord_t *collRecMap, 
	void* ghoul2, 
	const vec3_t angles, 
	const vec3_t position,
	int frameNumber, 
	int entNum, 
	vec3_t rayStart, 
	vec3_t rayEnd, 
	vec3_t scale, 
	int traceFlags, 
	int useLod,
	float fRadius
	)
{
	syscall ( G_G2_COLLISIONDETECTCACHE, collRecMap, ghoul2, angles, position, frameNumber, entNum, rayStart, rayEnd, scale, traceFlags, useLod, PASSFLOAT(fRadius) );
}

void trap_G2API_GetSurfaceName(void *ghoul2, int surfNumber, int modelIndex, char *fillBuf)
{
	syscall(G_G2_GETSURFACENAME, ghoul2, surfNumber, modelIndex, fillBuf);
}

qboolean trap_G2API_SetRootSurface(void *ghoul2, const int modelIndex, const char *surfaceName)
{
	return syscall(G_G2_SETROOTSURFACE, ghoul2, modelIndex, surfaceName);
}

qboolean trap_G2API_SetSurfaceOnOff(void *ghoul2, const char *surfaceName, const int flags)
{
	return syscall(G_G2_SETSURFACEONOFF, ghoul2, surfaceName, flags);
}

qboolean trap_G2API_SetNewOrigin(void *ghoul2, const int boltIndex)
{
	return syscall(G_G2_SETNEWORIGIN, ghoul2, boltIndex);
}

//check if a bone exists on skeleton without actually adding to the bone list -rww
qboolean trap_G2API_DoesBoneExist(void *ghoul2, int modelIndex, const char *boneName)
{
	return syscall(G_G2_DOESBONEEXIST, ghoul2, modelIndex, boneName);
}

int trap_G2API_GetSurfaceRenderStatus(void *ghoul2, const int modelIndex, const char *surfaceName)
{
	return syscall(G_G2_GETSURFACERENDERSTATUS, ghoul2, modelIndex, surfaceName);
}

//hack for smoothing during ugly situations. forgive me.
void trap_G2API_AbsurdSmoothing(void *ghoul2, qboolean status)
{
	syscall(G_G2_ABSURDSMOOTHING, ghoul2, status);
}

//rww - RAGDOLL_BEGIN
void trap_G2API_SetRagDoll(void *ghoul2, sharedRagDollParams_t *params)
{
	syscall(G_G2_SETRAGDOLL, ghoul2, params);
}

void trap_G2API_AnimateG2Models(void *ghoul2, int time, sharedRagDollUpdateParams_t *params)
{
	syscall(G_G2_ANIMATEG2MODELS, ghoul2, time, params);
}
//rww - RAGDOLL_END

//additional ragdoll options -rww
qboolean trap_G2API_RagPCJConstraint(void *ghoul2, const char *boneName, vec3_t min, vec3_t max) //override default pcj bonee constraints
{
	return syscall(G_G2_RAGPCJCONSTRAINT, ghoul2, boneName, min, max);
}

qboolean trap_G2API_RagPCJGradientSpeed(void *ghoul2, const char *boneName, const float speed) //override the default gradient movespeed for a pcj bone
{
	return syscall(G_G2_RAGPCJGRADIENTSPEED, ghoul2, boneName, PASSFLOAT(speed));
}

qboolean trap_G2API_RagEffectorGoal(void *ghoul2, const char *boneName, vec3_t pos) //override an effector bone's goal position (world coordinates)
{
	return syscall(G_G2_RAGEFFECTORGOAL, ghoul2, boneName, pos);
}

qboolean trap_G2API_GetRagBonePos(void *ghoul2, const char *boneName, vec3_t pos, vec3_t entAngles, vec3_t entPos, vec3_t entScale) //current position of said bone is put into pos (world coordinates)
{
	return syscall(G_G2_GETRAGBONEPOS, ghoul2, boneName, pos, entAngles, entPos, entScale);
}

qboolean trap_G2API_RagEffectorKick(void *ghoul2, const char *boneName, vec3_t velocity) //add velocity to a rag bone
{
	return syscall(G_G2_RAGEFFECTORKICK, ghoul2, boneName, velocity);
}

qboolean trap_G2API_RagForceSolve(void *ghoul2, qboolean force) //make sure we are actively performing solve/settle routines, if desired
{
	return syscall(G_G2_RAGFORCESOLVE, ghoul2, force);
}

qboolean trap_G2API_SetBoneIKState(void *ghoul2, int time, const char *boneName, int ikState, sharedSetBoneIKStateParams_t *params)
{
	return syscall(G_G2_SETBONEIKSTATE, ghoul2, time, boneName, ikState, params);
}

qboolean trap_G2API_IKMove(void *ghoul2, int time, sharedIKMoveParams_t *params)
{
	return syscall(G_G2_IKMOVE, ghoul2, time, params);
}

qboolean trap_G2API_RemoveBone(void *ghoul2, const char *boneName, int modelIndex)
{
	return syscall(G_G2_REMOVEBONE, ghoul2, boneName, modelIndex);
}

//rww - Stuff to allow association of ghoul2 instances to entity numbers.
//This way, on listen servers when both the client and server are doing
//ghoul2 operations, we can copy relevant data off the client instance
//directly onto the server instance and slash the transforms and whatnot
//right in half.
void trap_G2API_AttachInstanceToEntNum(void *ghoul2, int entityNum, qboolean server)
{
	syscall(G_G2_ATTACHINSTANCETOENTNUM, ghoul2, entityNum, server);
}

void trap_G2API_ClearAttachedInstance(int entityNum)
{
	syscall(G_G2_CLEARATTACHEDINSTANCE, entityNum);
}

void trap_G2API_CleanEntAttachments(void)
{
	syscall(G_G2_CLEANENTATTACHMENTS);
}

qboolean trap_G2API_OverrideServer(void *serverInstance)
{
	return syscall(G_G2_OVERRIDESERVER, serverInstance);
}

/*
Ghoul2 Insert End
*/

void trap_SetActiveSubBSP(int index)
{ //rwwRMG - added [NEWTRAP]
	syscall( G_SET_ACTIVE_SUBBSP, index );
}

int	trap_CM_RegisterTerrain(const char *config)
{ //rwwRMG - added [NEWTRAP]
	return syscall(G_CM_REGISTER_TERRAIN, config);
}

void trap_RMG_Init(int terrainID)
{ //rwwRMG - added [NEWTRAP]
	syscall(G_RMG_INIT, terrainID);
}

void trap_Bot_UpdateWaypoints(int wpnum, wpobject_t **wps)
{
	syscall(G_BOT_UPDATEWAYPOINTS, wpnum, wps);
}

void trap_Bot_CalculatePaths(int rmg)
{
	syscall(G_BOT_CALCULATEPATHS, rmg);
}

#include "../namespace_end.h"
