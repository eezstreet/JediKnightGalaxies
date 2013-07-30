// Copyright (C) 1999-2000 Id Software, Inc.
//

#ifndef G_PUBLIC_H

// g_public.h -- game module information visible to server
#define G_PUBLIC_H

#include "bg_vehicles.h"

// Jedi Knight Galaxies - Added a raised Q3_INFINITE to the absolute maximum time possible (server will auto-restart way before it reaches this)
// The old max caused fuckups after 279 minutes of gameplay (as level.time will go over 0x01000000 or 16777216
// The new limit will take over 24 days to reach (and the wrap around happens in about 23, so this'll never be reached
#define Q3_INFINITE			16777216
#define Q3_TIMEINFINITE		0x7FFFFFFF

#define	GAME_API_VERSION	9

// entity->svFlags
// the server does not know how to interpret most of the values
// in entityStates (level eType), so the game must explicitly flag
// special server behaviors
#define	SVF_NOCLIENT			0x00000001	// don't send entity to clients, even if it has effects
#define SVF_BOT					0x00000008	// set if the entity is a bot
#define SVF_PLAYER_USABLE		0x00000010	// player can use this with the use button
#define	SVF_BROADCAST			0x00000020	// send to all connected clients
#define	SVF_PORTAL				0x00000040	// merge a second pvs at origin2 into snapshots
#define	SVF_USE_CURRENT_ORIGIN	0x00000080	// entity->r.currentOrigin instead of entity->s.origin
											// for link position (missiles and movers)
#define SVF_SINGLECLIENT		0x00000100	// only send to a single client (entityShared_t->singleClient)
#define SVF_NOSERVERINFO		0x00000200	// don't send CS_SERVERINFO updates to this client
											// so that it can be updated for ping tools without
											// lagging clients
#define SVF_CAPSULE				0x00000400	// use capsule for collision detection instead of bbox
#define SVF_NOTSINGLECLIENT		0x00000800	// send entity to everyone but one client
											// (entityShared_t->singleClient)

#define SVF_OWNERNOTSHARED		0x00001000	// If it's owned by something and another thing owned by that something
											// hits it, it will still touch

#define	SVF_ICARUS_FREEZE		0x00008000	// NPCs are frozen, ents don't execute ICARUS commands

#define SVF_GLASS_BRUSH			0x08000000	// Ent is a glass brush

#define SVF_NO_BASIC_SOUNDS		0x10000000	// No basic sounds
#define SVF_NO_COMBAT_SOUNDS	0x20000000	// No combat sounds
#define SVF_NO_EXTRA_SOUNDS		0x40000000	// No extra or jedi sounds

//rww - ghoul2 trace flags
#define G2TRFLAG_DOGHOULTRACE	0x00000001 //do the ghoul2 trace
#define G2TRFLAG_HITCORPSES		0x00000002 //will try g2 collision on the ent even if it's EF_DEAD
#define G2TRFLAG_GETSURFINDEX	0x00000004 //will replace surfaceFlags with the ghoul2 surface index that was hit, if any.
#define G2TRFLAG_THICK			0x00000008 //assures that the trace radius will be significantly large regardless of the trace box size.

//===============================================================

//this structure is shared by gameside and in-engine NPC nav routines.
typedef struct failedEdge_e
{
	int	startID;
	int	endID;
	int checkTime;
	int	entID;
} failedEdge_t;

typedef struct {
	qboolean	linked;				// qfalse if not in any good cluster
	int			linkcount;

	int			svFlags;			// SVF_NOCLIENT, SVF_BROADCAST, etc
	int			singleClient;		// only send to this client when SVF_SINGLECLIENT is set

	qboolean	bmodel;				// if false, assume an explicit mins / maxs bounding box
									// only set by trap_SetBrushModel
	vec3_t		mins, maxs;
	int			contents;			// CONTENTS_TRIGGER, CONTENTS_SOLID, CONTENTS_BODY, etc
									// a non-solid entity should set to 0

	vec3_t		absmin, absmax;		// derived from mins/maxs and origin + rotation

	// currentOrigin will be used for all collision detection and world linking.
	// it will not necessarily be the same as the trajectory evaluation for the current
	// time, because each entity must be moved one at a time after time is advanced
	// to avoid simultanious collision issues
	vec3_t		currentOrigin;
	vec3_t		currentAngles;
	qboolean	mIsRoffing;			// set to qtrue when the entity is being roffed

	// when a trace call is made and passEntityNum != ENTITYNUM_NONE,
	// an ent will be excluded from testing if:
	// ent->s.number == passEntityNum	(don't interact with self)
	// ent->s.ownerNum = passEntityNum	(don't interact with your own missiles)
	// entity[ent->s.ownerNum].ownerNum = passEntityNum	(don't interact with other missiles from owner)
	int			ownerNum;

	// mask of clients that this entity should be broadcast too.  The first 32 clients
	// are represented by the first array index and the latter 32 clients are represented
	// by the second array index.
	int			broadcastClients[2];

} entityShared_t;

//bstate.h
typedef enum //# bState_e
{//These take over only if script allows them to be autonomous
	BS_DEFAULT = 0,//# default behavior for that NPC
	BS_ADVANCE_FIGHT,//# Advance to captureGoal and shoot enemies if you can
	BS_SLEEP,//# Play awake script when startled by sound
	BS_FOLLOW_LEADER,//# Follow your leader and shoot any enemies you come across
	BS_JUMP,//# Face navgoal and jump to it.
	BS_SEARCH,//# Using current waypoint as a base, search the immediate branches of waypoints for enemies
	BS_WANDER,//# Wander down random waypoint paths
	BS_NOCLIP,//# Moves through walls, etc.
	BS_REMOVE,//# Waits for player to leave PVS then removes itself
	BS_CINEMATIC,//# Does nothing but face it's angles and move to a goal if it has one
	//# #eol
	//internal bStates only
	BS_WAIT,//# Does nothing but face it's angles
	BS_STAND_GUARD,
	BS_PATROL,
	BS_INVESTIGATE,//# head towards temp goal and look for enemies and listen for sounds
	BS_STAND_AND_SHOOT,
	BS_HUNT_AND_KILL,
	BS_FLEE,//# Run away!
	NUM_BSTATES
} bState_t;

enum
{
	EDGE_NORMAL,
	EDGE_PATH,
	EDGE_BLOCKED,
	EDGE_FAILED,
	EDGE_MOVEDIR
};

enum
{
	NODE_NORMAL,
	NODE_START,
	NODE_GOAL,
	NODE_NAVGOAL,
};

typedef enum //# taskID_e
{
	TID_CHAN_VOICE = 0,	// Waiting for a voice sound to complete
	TID_ANIM_UPPER,		// Waiting to finish a lower anim holdtime
	TID_ANIM_LOWER,		// Waiting to finish a lower anim holdtime
	TID_ANIM_BOTH,		// Waiting to finish lower and upper anim holdtimes or normal md3 animating
	TID_MOVE_NAV,		// Trying to get to a navgoal or For ET_MOVERS
	TID_ANGLE_FACE,		// Turning to an angle or facing
	TID_BSTATE,			// Waiting for a certain bState to finish
	TID_LOCATION,		// Waiting for ent to enter a specific trigger_location
//	TID_MISSIONSTATUS,	// Waiting for player to finish reading MISSION STATUS SCREEN
	TID_RESIZE,			// Waiting for clear bbox to inflate size
	TID_SHOOT,			// Waiting for fire event
	NUM_TIDS,			// for def of taskID array
} taskID_t;

typedef enum //# bSet_e
{//This should check to matching a behavior state name first, then look for a script
	BSET_INVALID = -1,
	BSET_FIRST = 0,
	BSET_SPAWN = 0,//# script to use when first spawned
	BSET_USE,//# script to use when used
	BSET_AWAKE,//# script to use when awoken/startled
	BSET_ANGER,//# script to use when aquire an enemy
	BSET_ATTACK,//# script to run when you attack
	BSET_VICTORY,//# script to run when you kill someone
	BSET_LOSTENEMY,//# script to run when you can't find your enemy
	BSET_PAIN,//# script to use when take pain
	BSET_FLEE,//# script to use when take pain below 50% of health
	BSET_DEATH,//# script to use when killed
	BSET_DELAYED,//# script to run when self->delayScriptTime is reached
	BSET_BLOCKED,//# script to run when blocked by a friendly NPC or player
	BSET_BUMPED,//# script to run when bumped into a friendly NPC or player (can set bumpRadius)
	BSET_STUCK,//# script to run when blocked by a wall
	BSET_FFIRE,//# script to run when player shoots their own teammates
	BSET_FFDEATH,//# script to run when player kills a teammate
	BSET_MINDTRICK,//# script to run when player does a mind trick on this NPC

	NUM_BSETS
} bSet_t;

#define	MAX_PARMS	16
#define	MAX_PARM_STRING_LENGTH	MAX_QPATH//was 16, had to lengthen it so they could take a valid file path
typedef struct
{	
	char	parm[MAX_PARMS][MAX_PARM_STRING_LENGTH];
} parms_t;

#define MAX_FAILED_NODES 8

#if __WIN32 && !defined(__GNUC__)
typedef struct Vehicle_s Vehicle_t;
#endif

// the server looks at a sharedEntity, which is the start of the game's gentity_t structure
//mod authors should not touch this struct
typedef struct {
	entityState_t	s;				// communicated by server to clients
	playerState_t	*playerState;	//needs to be in the gentity for bg entity access
									//if you want to actually see the contents I guess
									//you will have to be sure to VMA it first.
	Vehicle_t		*m_pVehicle; //vehicle data
	void			*ghoul2; //g2 instance
	int				localAnimIndex; //index locally (game/cgame) to anim data for this skel
	vec3_t			modelScale; //needed for g2 collision

	//from here up must also be unified with bgEntity/centity

	entityShared_t	r;				// shared by both the server system and game

	//Script/ICARUS-related fields
	int				taskID[NUM_TIDS];
	parms_t			*parms;
	char			*behaviorSet[NUM_BSETS];
	char			*script_targetname;
	int				delayScriptTime;
	char			*fullName;

	//rww - targetname and classname are now shared as well. ICARUS needs access to them.
	char			*targetname;
	char			*classname;			// set in QuakeEd

	//rww - and yet more things to share. This is because the nav code is in the exe because it's all C++.
	int				waypoint;			//Set once per frame, if you've moved, and if someone asks
	int				lastWaypoint;		//To make sure you don't double-back
	int				lastValidWaypoint;	//ALWAYS valid -used for tracking someone you lost
	int				noWaypointTime;		//Debouncer - so don't keep checking every waypoint in existance every frame that you can't find one
	int				combatPoint;
	int				failedWaypoints[MAX_FAILED_NODES];
	int				failedWaypointCheckTime;

	int				next_roff_time; //rww - npc's need to know when they're getting roff'd
} sharedEntity_t;

#ifdef ENGINE
#ifdef __cplusplus
class CSequencer;
class CTaskManager;

//I suppose this could be in another in-engine header or something. But we never want to
//include an icarus file before sharedentity_t is declared.
extern CSequencer	*gSequencers[MAX_GENTITIES];
extern CTaskManager	*gTaskManagers[MAX_GENTITIES];

#include "../icarus/icarus.h"
#include "../icarus/sequencer.h"
#include "../icarus/taskmanager.h"
#include "../icarus/blockstream.h"
#endif
#endif

//
// functions exported by the game subsystem
//
typedef enum {
	GAME_INIT,	// ( int levelTime, int randomSeed, int restart );
	// init and shutdown will be called every single level
	// The game should call G_GET_ENTITY_TOKEN to parse through all the
	// entity configuration text and spawn gentities.

	GAME_SHUTDOWN,	// (void);

	GAME_CLIENT_CONNECT,	// ( int clientNum, qboolean firstTime, qboolean isBot );
	// return NULL if the client is allowed to connect, otherwise return
	// a text string with the reason for denial

	GAME_CLIENT_BEGIN,				// ( int clientNum );

	GAME_CLIENT_USERINFO_CHANGED,	// ( int clientNum );

	GAME_CLIENT_DISCONNECT,			// ( int clientNum );

	GAME_CLIENT_COMMAND,			// ( int clientNum );

	GAME_CLIENT_THINK,				// ( int clientNum );

	GAME_RUN_FRAME,					// ( int levelTime );

	GAME_CONSOLE_COMMAND,			// ( void );
	// ConsoleCommand will be called when a command has been issued
	// that is not recognized as a builtin function.
	// The game can issue trap_argc() / trap_argv() commands to get the command
	// and parameters.  Return qfalse if the game doesn't recognize it as a command.

	BOTAI_START_FRAME,				// ( int time );

	GAME_ROFF_NOTETRACK_CALLBACK,	// int entnum, char *notetrack

	GAME_SPAWN_RMG_ENTITY, //rwwRMG - added

	//rww - icarus callbacks
	GAME_ICARUS_PLAYSOUND,
	GAME_ICARUS_SET,
	GAME_ICARUS_LERP2POS,
	GAME_ICARUS_LERP2ORIGIN,
	GAME_ICARUS_LERP2ANGLES,
	GAME_ICARUS_GETTAG,
	GAME_ICARUS_LERP2START,
	GAME_ICARUS_LERP2END,
	GAME_ICARUS_USE,
	GAME_ICARUS_KILL,
	GAME_ICARUS_REMOVE,
	GAME_ICARUS_PLAY,
	GAME_ICARUS_GETFLOAT,
	GAME_ICARUS_GETVECTOR,
	GAME_ICARUS_GETSTRING,
	GAME_ICARUS_SOUNDINDEX,
	GAME_ICARUS_GETSETIDFORSTRING,
	GAME_NAV_CLEARPATHTOPOINT,
	GAME_NAV_CLEARLOS,
	GAME_NAV_CLEARPATHBETWEENPOINTS,
	GAME_NAV_CHECKNODEFAILEDFORENT,
	GAME_NAV_ENTISUNLOCKEDDOOR,
	GAME_NAV_ENTISDOOR,
	GAME_NAV_ENTISBREAKABLE,
	GAME_NAV_ENTISREMOVABLEUSABLE,
	GAME_NAV_FINDCOMBATPOINTWAYPOINTS,
	
	GAME_GETITEMINDEXBYTAG
} gameExport_t;

typedef struct
{
	int taskID;
	int entID;
	char name[2048];
	char channel[2048];
} T_G_ICARUS_PLAYSOUND;


typedef struct
{
	int taskID;
	int entID;
	char type_name[2048];
	char data[2048];
} T_G_ICARUS_SET;

typedef struct
{
	int taskID;
	int entID; 
	vec3_t origin;
	vec3_t angles;
	float duration;
	qboolean nullAngles; //special case
} T_G_ICARUS_LERP2POS;

typedef struct
{
	int taskID;
	int entID;
	vec3_t origin;
	float duration;
} T_G_ICARUS_LERP2ORIGIN;

typedef struct
{
	int taskID;
	int entID;
	vec3_t angles;
	float duration;
} T_G_ICARUS_LERP2ANGLES;

typedef struct
{
	int entID;
	char name[2048];
	int lookup;
	vec3_t info;
} T_G_ICARUS_GETTAG;

typedef struct
{
	int entID;
	int taskID;
	float duration;
} T_G_ICARUS_LERP2START;

typedef struct
{
	int entID;
	int taskID;
	float duration;
} T_G_ICARUS_LERP2END;

typedef struct
{
	int entID;
	char target[2048];
} T_G_ICARUS_USE;

typedef struct
{
	int entID;
	char name[2048];
} T_G_ICARUS_KILL;

typedef struct
{
	int entID;
	char name[2048];
} T_G_ICARUS_REMOVE;

typedef struct
{
	int taskID;
	int entID;
	char type[2048];
	char name[2048];
} T_G_ICARUS_PLAY;

typedef struct
{
	int entID;
	int type;
	char name[2048];
	float value;
} T_G_ICARUS_GETFLOAT;

typedef struct
{
	int entID;
	int type;
	char name[2048];
	vec3_t value;
} T_G_ICARUS_GETVECTOR;

typedef struct
{
	int entID;
	int type;
	char name[2048];
	char value[2048];
} T_G_ICARUS_GETSTRING;

typedef struct
{
	char filename[2048];
} T_G_ICARUS_SOUNDINDEX;
typedef struct
{
	char string[2048];
} T_G_ICARUS_GETSETIDFORSTRING;


// QVM stripped imports -- JKG
#include "g_local.h"

typedef struct
{
	int APIversion;

	
	void		(*Printf)( const char *fmt );
	void		(*Error)( const char *fmt );
	int			(*Milliseconds)( void );

	//rww - precision timer funcs... -ALWAYS- call end after start with supplied ptr, or you'll get a nasty memory leak.
	//not that you should be using these outside of debug anyway.. because you shouldn't be. So don't.

	//Start should be suppled with a pointer to an empty pointer (e.g. void *blah; trap_PrecisionTimer_Start(&blah);),
	//the empty pointer will be filled with an exe address to our timer (this address means nothing in vm land however).
	//You must pass this pointer back unmodified to the timer end func.
	void		(*PrecisionTimer_Start)( void **theNewTimer );
	int			(*PrecisionTimer_End)( void *theTimer );
	//If you're using the above example, the appropriate call for this is int result = trap_PrecisionTimer_End(blah);

	void		(*Cvar_Register)( vmCvar_t *cvar, const char *var_name, const char *value, int flags );
	void		(*Cvar_Update)( vmCvar_t *cvar );
	void		(*Cvar_Set)( const char *var_name, const char *value );
	int			(*Cvar_VariableIntegerValue)( const char *var_name );
	void		(*Cvar_VariableStringBuffer)( const char *var_name, char *buffer, int bufsize );
	
	int			(*Argc)( void );
	void		(*Argv)( int n, char *buffer, int bufferLength );

	int			(*FS_FOpenFile)( const char *qpath, fileHandle_t *f, fsMode_t mode );
	void		(*FS_Read)( void *buffer, int len, fileHandle_t f );
	void		(*FS_Write)( const void *buffer, int len, fileHandle_t f );
	void		(*FS_FCloseFile)( fileHandle_t f );
	int			(*FS_GetFileList)( const char *path, const char *extension, char *listbuf, int bufsize );

	void		(*SendConsoleCommand)( int exec_when, const char *text );
	void		(*LocateGameData)( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t,
						 playerState_t *clients, int sizeofGClient );
	void		(*DropClient)( int clientNum, const char *reason );
	void		(*SendServerCommand)( int clientNum, const char *cmd ); // TODO: Fix a la JKG's custom function
	void		(*GetUsercmd)( int clientNum, usercmd_t *cmd );
	bool		(*GetEntityToken)( char *buffer, int bufferSize );

	void		(*SetConfigstring)( int num, const char *string );
	void		(*GetConfigstring)( int num, char *buffer, int bufferSize );
	void		(*GetUserinfo)( int num, char *buffer, int bufferSize );
	void		(*SetUserinfo)( int num, const char *buffer );
	void		(*GetServerinfo)( char *buffer, int bufferSize );

	void		(*SetServerCull)( float cullDistance );
	void		(*SetBrushModel)( gentity_t *ent, const char *name );
	void		(*Trace)( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
	void		(*G2Trace)( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask, int g2TraceType, int traceLod );
	int			(*PointContents)( const vec3_t point, int passEntityNum );
	bool		(*InPVS)( const vec3_t p1, const vec3_t p2 ); // TODO: merge InPVS and InPVSIgnorePortals into one func
	bool		(*InPVSIgnorePortals)( const vec3_t p1, const vec3_t p2 );
	void		(*AdjustAreaPortalState)( gentity_t *ent, bool open );
	bool		(*AreasConnected)( int area1, int area2 );
	void		(*TraceCapsule)( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
	bool		(*EntityContactCapsule)( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );

	void		(*LinkEntity)( gentity_t *ent );
	void		(*UnlinkEntity)( gentity_t *ent );

	int			(*EntitiesInBox)( const vec3_t mins, const vec3_t maxs, int *list, int maxcount );
	bool		(*EntityContact)( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );

	// TODO: remove
	int			(*BotAllocateClient)( void );
	void		(*BotFreeClient)( int clientNum );

	int			(*RealTime)( qtime_t *qtime );
	void		(*SnapVector)( float *v );

	bool		(*ROFF_Clean)( void );
	void		(*ROFF_UpdateEntities)( void );
	int			(*ROFF_Cache)( char *file );
	bool		(*ROFF_Play)( int entID, int roffID, bool doTranslation );
	bool		(*ROFF_Purge_Ent)( int entID );

	// TODO: remove
	void		(*TrueMalloc)( void **ptr, int size );
	void		(*TrueFree)( void **ptr );

	// TODO: remove and replace with Lua?
	int			(*ICARUS_RunScript)( gentity_t *ent, const char *name );
	bool		(*ICARUS_RegisterScript)( const char *name, bool bCalledDuringInterrogate );
	void		(*ICARUS_Init)( void );
	bool		(*ICARUS_ValidEnt)( gentity_t *ent );
	bool		(*ICARUS_IsInitialized)( int entID );
	bool		(*ICARUS_MaintainTaskManager)( int entID );
	bool		(*ICARUS_IsRunning)( int entID );
	bool		(*ICARUS_TaskIDPending)( gentity_t *ent, int taskID );
	void		(*ICARUS_InitEnt)( gentity_t *ent );
	void		(*ICARUS_FreeEnt)( gentity_t *ent );
	void		(*ICARUS_AssociateEnt)( gentity_t *ent );
	void		(*ICARUS_Shutdown)( void );
	void		(*ICARUS_TaskIDSet)( gentity_t *ent, int taskType, int taskID );
	void		(*ICARUS_TaskIDComplete)( gentity_t *ent, int taskType );
	void		(*ICARUS_SetVar)( int taskID, int entID, const char *type_name, const char *data );
	int			(*ICARUS_VariableDeclared)( const char *type_name );
	int			(*ICARUS_GetFloatVariable)( const char *name, float *value );
	int			(*ICARUS_GetStringVariable)( const char *name, const char *value );
	int			(*ICARUS_GetVectorVariable)( const char *name, const vec3_t value );

	// TODO: remove
	void		(*NAV_Init)( void );
	void		(*NAV_Free)( void );
	bool		(*NAV_Load)( const char *filename, int checksum );
	bool		(*NAV_Save)( const char *filename, int checksum );
	int			(*NAV_AddRawPoint)( vec3_t point, int flags, int radius );
	void		(*NAV_CalculatePatsh)( bool recalc );
	void		(*NAV_HardConnect)( int first, int second );
	void		(*NAV_ShowNodes)( void );
	void		(*NAV_ShowEdges)( void );
	void		(*NAV_ShowPath)( int start, int end );
	int			(*NAV_GetNearestNode)( gentity_t *ent, int lastID, int flags, int targetID );
	int			(*NAV_GetBestNode)( int startID, int endID, int rejectID );
	int			(*NAV_GetNodePosition)( int nodeID, vec3_t out );
	int			(*NAV_GetNodeNumEdges)( int nodeID );
	int			(*NAV_GetNodeEdge)( int nodeID, int edge );
	int			(*NAV_GetNumNodes)( void );
	bool		(*NAV_Connected)( int startID, int endID );
	int			(*NAV_GetPathCost)( int startID, int endID );
	void		(*NAV_CheckFailedNodes)( gentity_t *ent);
	void		(*NAV_AddFailedNode)( gentity_t *ent, int nodeID );
	bool		(*NAV_NodesAreNeighbors)( int startID, int endID );
	void		(*NAV_ClearAllFailedEdges)( void );
	void		(*NAV_AddFailedEdge)( int entID, int startID, int endID );
	void		(*NAV_CheckAllFailedEdges)( void );
	int			(*NAV_GetBestNodeAltRoute)( int startID, int endID, int rejectID );
	int			(*NAV_GetBestPathBetweenEnts)( gentity_t *ent, gentity_t *goal, int flags );
	void		(*NAV_CheckBlockedEdges)( void );
	void		(*NAV_ClearCheckedNodes)( void );
	void		(*NAV_SetPathsCalculated)( bool newVal );

	void		(*SV_RegisterSharedMemory)( char *memory );

	// TODO: remove
	int			(*BotLibSetup)( void );
	int			(*BotLibShutdown)( void );
	int			(*BotGetSnapshotEntity)( int clientNum, int sequence );
	int			(*BotGetServerCommand)( int clientNum, char *message, int size );
	void		(*BotUserCommand)( int clientNum, usercmd_t *ucmd );
	void		(*AAS_EntityInfo)( int entnum, void /* struct aas_entityinfo_s */ *info );
	void		(*EA_Attack)( int client );
	void		(*EA_Alt_Attack)( int client );
	void		(*EA_ForcePower)( int client );
	void		(*EA_Use)( int client );
	void		(*EA_Crouch)( int client );
	void		(*EA_MoveForward)( int client );
	void		(*EA_SelectWeapon)( int client, int weapon );
	void		(*EA_Jump)( int client );
	void		(*EA_Move)( int client, vec3_t dir, float speed );
	void		(*EA_View)( int client, vec3_t viewangles );
	void		(*EA_GetInput)( int client, float thinktime, void /* struct bot_input_s */ *input );
	void		(*EA_ResetInput)( int client );
	void		(*BotResetGoalState)( int goalstate );
	void		(*BotResetAvoidGoals)( int goalstate );
	void		(*BotUpdateEntityItems)( void );
	int			(*BotAllocGoalState)( int state );
	void		(*BotFreeGoalState)( int handle );
	void		(*BotResetMoveState)( int movestate );
	void		(*BotResetAvoidReach)( int movestate );
	int			(*BotAllocMoveState)( void );
	void		(*BotFreeMoveState)( int handle );
	int			(*BotAllocWeaponState)( void );
	void		(*BotFreeWeaponState)( int weaponstate );
	void		(*BotResetWeaponState)( int weaponstate );
	void		(*Bot_UpdateWaypoints)( int wpnum, wpobject_t **wps );
	void		(*Bot_CalculatePaths)( int rmg );

	int			(*PC_LoadSource)( const char *filename );
	int			(*PC_FreeSource)( int handle );
	int			(*PC_ReadToken)( int handle, pc_token_t *pc_token );
	int			(*PC_SourceFileAndLine)( int handle, char *filename, int *line );

/*
Ghoul2 Insert Start
*/
	qhandle_t	(*R_RegisterSkin)( const char *name );
	void		(*G2_ListModelBones)( void *ghlInfo, int frame );
	void		(*G2_ListModelSurfaces)( void *ghlInfo );
	bool		(*G2_HaveWeGhoul2Models)( void *ghoul2 );
	void		(*G2_SetGhoul2ModelIndexes)( void *ghoul2, qhandle_t *modelList, qhandle_t *skinList );
	bool		(*G2API_GetBoltMatrix)( void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix,
								const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale );
	bool		(*G2API_GetBoltMatrix_NoReconstruct)( void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix,
								const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale );
	bool		(*G2API_GetBoltMatrix_NoRecNoRot)( void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix,
								const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale );
	int			(*G2API_InitGhoul2Model)( void **ghoul2Ptr, const char *fileName, int modelIndex, qhandle_t customSkin,
						  qhandle_t customShader, int modelFlags, int lodBias );
	bool		(*G2API_SetSkin)( void *ghoul2, int modelIndex, qhandle_t customSkin, qhandle_t renderSkin );
	int			(*G2API_Ghoul2Size)( void *ghlInfo );
	int			(*G2API_AddBolt)( void *ghoul2, int modelIndex, const char *boneName );
	void		(*G2API_SetBoltInfo)( void *ghoul2, int modelIndex, int boltInfo );
	bool		(*G2API_SetBoneAngles)( void *ghoul2, int modelIndex, const char *boneName, const vec3_t angles, const int flags,
								const int up, const int right, const int forward, qhandle_t *modelList,
								int blendTime , int currentTime );
	bool		(*G2API_SetBoneAnim)( void *ghoul2, const int modelIndex, const char *boneName, const int startFrame, const int endFrame,
							  const int flags, const float animSpeed, const int currentTime, const float setFrame , const int blendTime );
	bool		(*G2API_GetBoneAnim)( void *ghoul2, const char *boneName, const int currentTime, float *currentFrame,
						   int *startFrame, int *endFrame, int *flags, float *animSpeed, int *modelList, const int modelIndex );
	void		(*G2API_GetGLAName)( void *ghoul2, int modelIndex, char *fillBuf );
	int			(*G2API_CopyGhoul2Instance)( void *g2From, void *g2To, int modelIndex );
	void		(*G2API_CopySpecificGhoul2Model)( void *g2From, int modelFrom, void *g2To, int modelTo );
	void		(*G2API_DuplicateGhoul2Instance)( void *g2From, void **g2To );
	bool		(*G2API_HasGhoul2ModelOnIndex)( void *ghlInfo, int modelIndex );
	bool		(*G2API_RemoveGhoul2Model)( void *ghlInfo, int modelIndex );
	bool		(*G2API_RemoveGhoul2Models)( void *ghlInfo );
	void		(*G2API_CleanGhoul2Models)( void **ghoul2Ptr );
	void		(*G2API_CollisionDetect)( CollisionRecord_t *collRecMap, void *ghoul2, const vec3_t angles, const vec3_t position, int frameNumber,
							int entNum, vec3_t rayStart, vec3_t rayEnd, vec3_t scale, int traceFlags, int useLod, float fRadius );
	void		(*G2API_CollisionDetectCache)( CollisionRecord_t *collRecMap, void *ghoul2, const vec3_t angles, const vec3_t position, int frameNumber,
							int entNum, vec3_t rayStart, vec3_t rayEnd, vec3_t scale, int traceFlags, int useLod, float fRadius );
	void		(*G2API_GetSurfaceName)( void *ghoul2, int surfNumber, int modelIndex, char *fillBuf );
	bool		(*G2API_SetRootSurface)( void *ghoul2, const int modelIndex, const char *surfaceName );
	bool		(*G2API_SetSurfaceOnOff)( void *ghoul2, const char *surfaceName, const int flags );
	bool		(*G2API_SetNewOrigin)( void *ghoul2, const int boltIndex );
	bool		(*G2API_DoesBoneExist)( void *ghoul2, int modelIndex, const char *boneName );
	int			(*G2API_GetSurfaceRenderStatus)( void *ghoul2, const int modelIndex, const char *surfaceName );
	void		(*G2API_AbsurdSmoothing)( void *ghoul2, bool status );
	void		(*G2API_SetRagDoll)( void *ghoul2, sharedRagDollParams_t *params );
	void		(*G2API_AnimateG2Models)( void *ghoul2, int time, sharedRagDollUpdateParams_t *params );
	bool		(*G2API_RagPCJConstraint)( void *ghoul2, const char *boneName, vec3_t min, vec3_t max );
	bool		(*G2API_RagPCJGradientSpeed)( void *ghoul2, const char *boneName, const float speed );
	bool		(*G2API_RagEffectorGoal)( void *ghoul2, const char *boneName, vec3_t pos); //override an effector bone's goal position (world coordinates)
	bool		(*G2API_GetRagBonePos)( void *ghoul2, const char *boneName, vec3_t pos, vec3_t entAngles, vec3_t entPos, vec3_t entScale);
	bool		(*G2API_RagEffectorKick)( void *ghoul2, const char *boneName, vec3_t velocity );
	bool		(*G2API_RagForceSolve)( void *ghoul2, bool force );
	bool		(*G2API_SetBoneIKState)( void *ghoul2, int time, const char *boneName, int ikState, sharedSetBoneIKStateParams_t *params );
	bool		(*G2API_IKMove)( void *ghoul2, int time, sharedIKMoveParams_t *params );
	bool		(*G2API_RemoveBone)( void *ghoul2, const char *boneName, int modelIndex );
//rww - Stuff to allow association of ghoul2 instances to entity numbers.
//This way, on listen servers when both the client and server are doing
//ghoul2 operations, we can copy relevant data off the client instance
//directly onto the server instance and slash the transforms and whatnot
//right in half.
	void		(*G2API_AttachInstanceToEntNum)( void *ghoul2, int entityNum, bool server );
	void		(*G2API_ClearAttachedInstance)( int entityNum );
	void		(*G2API_CleanEntAttachments)( void );
	bool		(*G2API_OverrideServer)( void *serverInstance );
/*
Ghoul2 Insert End
*/

	void		(*SetActiveSubBSP)( int index );
	int			(*CM_RegisterTerrain)( const char *config );
	void		(*RMG_Init)( int terrainID );
} gameImport_t;

#endif //G_PUBLIC_H
