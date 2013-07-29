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
// cg_public.h
// Copyright (C) 1999-2000 Id Software, Inc. (c) 2013 Jedi Knight Galaxies

#ifndef __CG_PUBLIC_H
#define __CG_PUBLIC_H

#define	CMD_BACKUP			64	
#define	CMD_MASK			(CMD_BACKUP - 1)
// allow a lot of command backups for very fast systems
// multiple commands may be combined into a single packet, so this
// needs to be larger than PACKET_BACKUP


// Jedi Knight Galaxies
// Goddamn morrons at Ravensoft.. 256 clientside, 1024 serverside? yea right.. fixin
// Note, this uses a engine patch to disable the limit and an offset change to ensure snap.serverCommandSequence is still set --boba

//#define	MAX_ENTITIES_IN_SNAPSHOT	256
#define	MAX_ENTITIES_IN_SNAPSHOT	1024

// snapshots are a view of the server at a given time

// Snapshots are generated at regular time intervals by the server,
// but they may not be sent if a client's rate level is exceeded, or
// they may be dropped by the network.
typedef struct {
	int				snapFlags;			// SNAPFLAG_RATE_DELAYED, etc
	int				ping;

	int				serverTime;		// server time the message is valid for (in msec)

	byte			areamask[MAX_MAP_AREA_BYTES];		// portalarea visibility bits

	playerState_t	ps;						// complete information about the current player at this time
	playerState_t	vps; //vehicle I'm riding's playerstate (if applicable) -rww

	int				numEntities;			// all of the entities that need to be presented
	entityState_t	entities[MAX_ENTITIES_IN_SNAPSHOT];	// at the time of this snapshot

	int				numServerCommands;		// text based server commands to execute when this
	int				serverCommandSequence;	// snapshot becomes current
} snapshot_t;

enum {
  CGAME_EVENT_NONE,
  CGAME_EVENT_TEAMMENU,
  CGAME_EVENT_SCOREBOARD,
  CGAME_EVENT_EDITHUD
};


/*
==================================================================

functions imported from the main executable

==================================================================
*/

#define	CGAME_IMPORT_API_VERSION	5

typedef enum {
	CG_PRINT = 0,
	CG_ERROR,
	CG_MILLISECONDS,

	// these have to match the definitions in ui_public.h ==eez
	CG_CO_SYSCALL_UI = 4,
	CG_CO_SYSCALL_CG = 5,

	//Also for profiling.. do not use for game related tasks.
	CG_PRECISIONTIMER_START,
	CG_PRECISIONTIMER_END,

	CG_CVAR_REGISTER,
	CG_CVAR_UPDATE,
	CG_CVAR_SET,
	CG_CVAR_VARIABLESTRINGBUFFER,
	CG_CVAR_GETHIDDENVALUE,
	CG_ARGC,								// DOES NOT WORK			
	CG_ARGV,
	CG_ARGS,
	CG_FS_FOPENFILE,
	CG_FS_READ,
	CG_FS_WRITE,
	CG_FS_FCLOSEFILE,
	CG_FS_GETFILELIST,
	CG_SENDCONSOLECOMMAND,
	CG_ADDCOMMAND,
	CG_REMOVECOMMAND,
	CG_SENDCLIENTCOMMAND,
	CG_UPDATESCREEN,
	CG_CM_LOADMAP,
	CG_CM_NUMINLINEMODELS,					// DOES NOT WORK
	CG_CM_INLINEMODEL,
	CG_CM_TEMPBOXMODEL,
	CG_CM_TEMPCAPSULEMODEL,
	CG_CM_POINTCONTENTS,
	CG_CM_TRANSFORMEDPOINTCONTENTS,
	CG_CM_BOXTRACE,
	CG_CM_CAPSULETRACE,
	CG_CM_TRANSFORMEDBOXTRACE,
	CG_CM_TRANSFORMEDCAPSULETRACE,
	CG_CM_MARKFRAGMENTS,
	CG_S_GETVOICEVOLUME,					// DOES NOT WORK
	CG_S_MUTESOUND,
	CG_S_STARTSOUND,
	CG_S_STARTLOCALSOUND,
	CG_S_CLEARLOOPINGSOUNDS,
	CG_S_ADDLOOPINGSOUND,
	CG_S_UPDATEENTITYPOSITION,
	CG_S_ADDREALLOOPINGSOUND,
	CG_S_STOPLOOPINGSOUND,
	CG_S_RESPATIALIZE,
	CG_S_SHUTUP,
	CG_S_REGISTERSOUND,
	CG_S_STARTBACKGROUNDTRACK,

	//rww - AS trap implem
	CG_S_UPDATEAMBIENTSET,
	CG_AS_PARSESETS,
	CG_AS_ADDPRECACHEENTRY,
	CG_S_ADDLOCALSET,
	CG_AS_GETBMODELSOUND,

	CG_R_LOADWORLDMAP,
	CG_R_REGISTERMODEL,
	CG_R_REGISTERSKIN,
	CG_R_REGISTERSHADER,
	CG_R_REGISTERSHADERNOMIP,
	CG_R_REGISTERFONT,
	CG_R_FONT_STRLENPIXELS,
	CG_R_FONT_STRLENCHARS,
	CG_R_FONT_STRHEIGHTPIXELS,
	CG_R_FONT_DRAWSTRING,
	CG_LANGUAGE_ISASIAN,
	CG_LANGUAGE_USESSPACES,
	CG_ANYLANGUAGE_READCHARFROMSTRING,

	CGAME_MEMSET = 100,
	CGAME_MEMCPY,
	CGAME_STRNCPY,
	CGAME_SIN,
	CGAME_COS,
	CGAME_ATAN2,
	CGAME_SQRT,
	CGAME_MATRIXMULTIPLY,
	CGAME_ANGLEVECTORS,
	CGAME_PERPENDICULARVECTOR,
	CGAME_FLOOR,
	CGAME_CEIL,

	CGAME_TESTPRINTINT,						// DOES NOT WORK
	CGAME_TESTPRINTFLOAT,					// DOES NOT WORK

	CGAME_ACOS,
	CGAME_ASIN,

	CG_R_CLEARSCENE = 200,
	CG_R_CLEARDECALS,
	CG_R_ADDREFENTITYTOSCENE,
	CG_R_ADDPOLYTOSCENE,
	CG_R_ADDPOLYSTOSCENE,
	CG_R_ADDDECALTOSCENE,
	CG_R_LIGHTFORPOINT,
	CG_R_ADDLIGHTTOSCENE,
	CG_R_ADDADDITIVELIGHTTOSCENE,
	CG_R_RENDERSCENE,
	CG_R_SETCOLOR,
	CG_R_DRAWSTRETCHPIC,
	CG_R_MODELBOUNDS,
	CG_R_LERPTAG,
	CG_R_DRAWROTATEPIC,
	CG_R_DRAWROTATEPIC2,
	CG_R_SETRANGEFOG, //linear fogging, with settable range -rww
	CG_R_SETREFRACTIONPROP, //set some properties for the draw layer for my refractive effect (here primarily for mod authors) -rww
	CG_R_REMAP_SHADER,
	CG_R_GET_LIGHT_STYLE,
	CG_R_SET_LIGHT_STYLE,
	CG_R_GET_BMODEL_VERTS,
	CG_R_GETDISTANCECULL,

	CG_R_GETREALRES,
	CG_R_AUTOMAPELEVADJ,
	CG_R_INITWIREFRAMEAUTO,

	CG_FX_ADDLINE,

	CG_GETGLCONFIG,
	CG_GETGAMESTATE,
	CG_GETCURRENTSNAPSHOTNUMBER,
	CG_GETSNAPSHOT,
	CG_GETDEFAULTSTATE,
	CG_GETSERVERCOMMAND,
	CG_GETCURRENTCMDNUMBER,					// DOES NOT WORK
	CG_GETUSERCMD,
	CG_SETUSERCMDVALUE,
	CG_SETCLIENTFORCEANGLE,
	CG_SETCLIENTTURNEXTENT,					// DOES NOT WORK
	CG_OPENUIMENU,
	CG_TESTPRINTINT,
	CG_TESTPRINTFLOAT,
	CG_MEMORY_REMAINING,					// DOES NOT WORK
	CG_KEY_ISDOWN,
	CG_KEY_GETCATCHER,						// DOES NOT WORK
	CG_KEY_SETCATCHER,
	CG_KEY_GETKEY,

 	CG_PC_ADD_GLOBAL_DEFINE,
	CG_PC_LOAD_SOURCE,
	CG_PC_FREE_SOURCE,
	CG_PC_READ_TOKEN,
	CG_PC_SOURCE_FILE_AND_LINE,
	CG_PC_LOAD_GLOBAL_DEFINES,
	CG_PC_REMOVE_ALL_GLOBAL_DEFINES,

	CG_S_STOPBACKGROUNDTRACK,
	CG_REAL_TIME,
	CG_SNAPVECTOR,
	CG_CIN_PLAYCINEMATIC,
	CG_CIN_STOPCINEMATIC,
	CG_CIN_RUNCINEMATIC,
	CG_CIN_DRAWCINEMATIC,
	CG_CIN_SETEXTENTS,

	CG_GET_ENTITY_TOKEN,
	CG_R_INPVS,

	CG_FX_REGISTER_EFFECT,
	CG_FX_PLAY_EFFECT,
	CG_FX_PLAY_ENTITY_EFFECT,				// DOES NOT WORK
	CG_FX_PLAY_EFFECT_ID,
	CG_FX_PLAY_PORTAL_EFFECT_ID,
	CG_FX_PLAY_ENTITY_EFFECT_ID,
	CG_FX_PLAY_BOLTED_EFFECT_ID,
	CG_FX_ADD_SCHEDULED_EFFECTS,
	CG_FX_INIT_SYSTEM,
	CG_FX_SET_REFDEF,
	CG_FX_FREE_SYSTEM,
	CG_FX_ADJUST_TIME,
	CG_FX_DRAW_2D_EFFECTS,
	CG_FX_RESET,
	CG_FX_ADDPOLY,
	CG_FX_ADDBEZIER,
	CG_FX_ADDPRIMITIVE,
	CG_FX_ADDSPRITE,
	CG_FX_ADDELECTRICITY,

//	CG_SP_PRINT,
	CG_SP_GETSTRINGTEXTSTRING,

	CG_ROFF_CLEAN,
	CG_ROFF_UPDATE_ENTITIES,
	CG_ROFF_CACHE,
	CG_ROFF_PLAY,
	CG_ROFF_PURGE_ENT,


	//rww - dynamic vm memory allocation!
	CG_TRUEMALLOC,
	CG_TRUEFREE,

/*
Ghoul2 Insert Start
*/
	CG_G2_LISTSURFACES,
	CG_G2_LISTBONES,
	CG_G2_SETMODELS,						// DOES NOT WORK
	CG_G2_HAVEWEGHOULMODELS,
	CG_G2_GETBOLT,
	CG_G2_GETBOLT_NOREC,
	CG_G2_GETBOLT_NOREC_NOROT,
	CG_G2_INITGHOUL2MODEL,
	CG_G2_SETSKIN,
	CG_G2_COLLISIONDETECT,
	CG_G2_COLLISIONDETECTCACHE,
	CG_G2_CLEANMODELS,
	CG_G2_ANGLEOVERRIDE,
	CG_G2_PLAYANIM,
	CG_G2_GETBONEANIM,
	CG_G2_GETBONEFRAME, //trimmed down version of GBA, so I don't have to pass all those unused args across the VM-exe border
	CG_G2_GETGLANAME,
	CG_G2_COPYGHOUL2INSTANCE,
	CG_G2_COPYSPECIFICGHOUL2MODEL,
	CG_G2_DUPLICATEGHOUL2INSTANCE,
	CG_G2_HASGHOUL2MODELONINDEX,
	CG_G2_REMOVEGHOUL2MODEL,
	CG_G2_SKINLESSMODEL,
	CG_G2_GETNUMGOREMARKS,
	CG_G2_ADDSKINGORE,
	CG_G2_CLEARSKINGORE,
	CG_G2_SIZE,
	CG_G2_ADDBOLT,
	CG_G2_ATTACHENT,
	CG_G2_SETBOLTON,
	CG_G2_SETROOTSURFACE,
	CG_G2_SETSURFACEONOFF,
	CG_G2_SETNEWORIGIN,
	CG_G2_DOESBONEEXIST,
	CG_G2_GETSURFACERENDERSTATUS,

	CG_G2_GETTIME,
	CG_G2_SETTIME,

	CG_G2_ABSURDSMOOTHING,

/*
	//rww - RAGDOLL_BEGIN
*/
	CG_G2_SETRAGDOLL,
	CG_G2_ANIMATEG2MODELS,
/*
	//rww - RAGDOLL_END
*/

	//additional ragdoll options -rww
	CG_G2_RAGPCJCONSTRAINT,
	CG_G2_RAGPCJGRADIENTSPEED,
	CG_G2_RAGEFFECTORGOAL,
	CG_G2_GETRAGBONEPOS,					// DOES NOT WORK
	CG_G2_RAGEFFECTORKICK,
	CG_G2_RAGFORCESOLVE,

	//rww - ik move method, allows you to specify a bone and move it to a world point (within joint constraints)
	//by using the majority of gil's existing bone angling stuff from the ragdoll code.
	CG_G2_SETBONEIKSTATE,
	CG_G2_IKMOVE,

	CG_G2_REMOVEBONE,

	CG_G2_ATTACHINSTANCETOENTNUM,			// DOES NOT WORK
	CG_G2_CLEARATTACHEDINSTANCE,			// DOES NOT WORK
	CG_G2_CLEANENTATTACHMENTS,				// DOES NOT WORK
	CG_G2_OVERRIDESERVER,

	CG_G2_GETSURFACENAME,

	CG_SET_SHARED_BUFFER,

	CG_CM_REGISTER_TERRAIN,
	CG_RMG_INIT,
	CG_RE_INIT_RENDERER_TERRAIN,
	CG_R_WEATHER_CONTENTS_OVERRIDE,			// DOES NOT WORK (but is called?)
	CG_R_WORLDEFFECTCOMMAND,
	//Adding trap to get weather working
	CG_WE_ADDWEATHERZONE,

/*
Ghoul2 Insert End
*/

	// JKG trap calls --eez
	CG_CO_INITCROSSOVER,
	CG_CO_SHUTDOWN,

	CG_JKG_OVERRIDESHADERFRAME,
	CG_JKG_GETCOLORTABLE,
	CG_JKG_GETVIEWANGLES,
	CG_JKG_SETVIEWANGLES,

	// FX system stuff for CPLUSPLUS --eez
	CG_FX_GETSHAREDMEM,
	CG_FX_ADDMINIREFENTITY,

	CG_FX_GETEFFECTCOPY1,
	CG_FX_GETEFFECTCOPY2,
	CG_FX_GETPRIMITIVECOPY,
} cgameImport_t;


/*
==================================================================

functions exported to the main executable

==================================================================
*/

typedef enum {
	CG_INIT,
//	void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum )
	// called when the level loads or when the renderer is restarted
	// all media should be registered at this time
	// cgame will display loading status by calling SCR_Update, which
	// will call CG_DrawInformation during the loading process
	// reliableCommandSequence will be 0 on fresh loads, but higher for
	// demos, tourney restarts, or vid_restarts

	CG_SHUTDOWN,
//	void (*CG_Shutdown)( void );
	// oportunity to flush and close any open files

	CG_CONSOLE_COMMAND,
//	qboolean (*CG_ConsoleCommand)( void );
	// a console command has been issued locally that is not recognized by the
	// main game system.
	// use Cmd_Argc() / Cmd_Argv() to read the command, return qfalse if the
	// command is not known to the game

	CG_DRAW_ACTIVE_FRAME,
//	void (*CG_DrawActiveFrame)( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback );
	// Generates and draws a game scene and status information at the given time.
	// If demoPlayback is set, local movement prediction will not be enabled

	CG_CROSSHAIR_PLAYER,
//	int (*CG_CrosshairPlayer)( void );

	CG_LAST_ATTACKER,
//	int (*CG_LastAttacker)( void );

	CG_KEY_EVENT, 
//	void	(*CG_KeyEvent)( int key, qboolean down );

	CG_MOUSE_EVENT,
//	void	(*CG_MouseEvent)( int dx, int dy );
	CG_EVENT_HANDLING,
//	void (*CG_EventHandling)(int type);

	CG_POINT_CONTENTS,
//	int	CG_PointContents( const vec3_t point, int passEntityNum );

	CG_GET_LERP_ORIGIN,
//	void CG_LerpOrigin(int num, vec3_t result);

	CG_GET_LERP_DATA,
	CG_GET_GHOUL2,
	CG_GET_MODEL_LIST,

	CG_CALC_LERP_POSITIONS,
//	void CG_CalcEntityLerpPositions(int num);

	CG_TRACE,
	CG_G2TRACE,
//void CG_Trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, 
//					 int skipNumber, int mask );

	CG_G2MARK,

	CG_RAG_CALLBACK,

	CG_INCOMING_CONSOLE_COMMAND,

	CG_GET_USEABLE_FORCE,

	CG_GET_ORIGIN,		// int entnum, vec3_t origin
	CG_GET_ANGLES,		// int entnum, vec3_t angle

	CG_GET_ORIGIN_TRAJECTORY,		// int entnum
	CG_GET_ANGLE_TRAJECTORY,		// int entnum

	CG_ROFF_NOTETRACK_CALLBACK,		// int entnum, char *notetrack

	CG_IMPACT_MARK,
//void CG_ImpactMark( qhandle_t markShader, const vec3_t origin, const vec3_t dir, 
//				   float orientation, float red, float green, float blue, float alpha,
//				   qboolean alphaFade, float radius, qboolean temporary )

	CG_MAP_CHANGE,

	CG_AUTOMAP_INPUT,

	CG_MISC_ENT, //rwwRMG - added

	CG_GET_SORTED_FORCE_POWER,

	CG_FX_CAMERASHAKE,//mcg post-gold added

	// Jedi Knight Galaxies
	CG_MESSAGEMODE,
} cgameExport_t;

typedef struct
{
	float		up;
	float		down;
	float		yaw;
	float		pitch;
	qboolean	goToDefaults;
} autoMapInput_t;

// CG_POINT_CONTENTS
typedef struct
{
	vec3_t		mPoint;			// input
	int			mPassEntityNum;	// input
} TCGPointContents;

// CG_GET_BOLT_POS
typedef struct
{
	vec3_t		mOrigin;		// output
	vec3_t		mAngles;		// output
	vec3_t		mScale;			// output
	int			mEntityNum;		// input
} TCGGetBoltData;

// CG_IMPACT_MARK
typedef struct
{
	int		mHandle;
	vec3_t	mPoint;
	vec3_t	mAngle;
	float	mRotation;
	float	mRed;
	float	mGreen;
	float	mBlue;
	float	mAlphaStart;
	float	mSizeStart;
} TCGImpactMark;

// CG_GET_LERP_ORIGIN
// CG_GET_LERP_ANGLES
// CG_GET_MODEL_SCALE
typedef struct
{
	int			mEntityNum;		// input
	vec3_t		mPoint;			// output
} TCGVectorData;

// CG_TRACE/CG_G2TRACE
typedef struct
{
	trace_t mResult;					// output
	vec3_t	mStart, mMins, mMaxs, mEnd;	// input
	int		mSkipNumber, mMask;			// input
} TCGTrace;

// CG_G2MARK
typedef struct
{
	int			shader;
	float		size;
	vec3_t		start, dir;
} TCGG2Mark;

// CG_INCOMING_CONSOLE_COMMAND
typedef struct
{
	char conCommand[1024];
} TCGIncomingConsoleCommand;

// CG_FX_CAMERASHAKE
typedef struct
{
	vec3_t	mOrigin;					// input
	float	mIntensity;					// input
	int		mRadius;					// input
	int		mTime;						// input
} TCGCameraShake;

// CG_MISC_ENT
typedef struct
{
	char	mModel[MAX_QPATH];			// input
	vec3_t	mOrigin, mAngles, mScale;	// input
} TCGMiscEnt;

typedef struct
{
	refEntity_t		ent;				// output
	void			*ghoul2;			// input
	int				modelIndex;			// input
	int				boltIndex;			// input
	vec3_t			origin;				// input
	vec3_t			angles;				// input
	vec3_t			modelScale;			// input
} TCGPositionOnBolt;

//ragdoll callback structs -rww
#define RAG_CALLBACK_NONE				0
#define RAG_CALLBACK_DEBUGBOX			1
typedef struct
{
	vec3_t			mins;
	vec3_t			maxs;
	int				duration;
} ragCallbackDebugBox_t;

#define RAG_CALLBACK_DEBUGLINE			2
typedef struct
{
	vec3_t			start;
	vec3_t			end;
	int				time;
	int				color;
	int				radius;
} ragCallbackDebugLine_t;

#define RAG_CALLBACK_BONESNAP			3
typedef struct
{
	char			boneName[128]; //name of the bone in question
	int				entNum; //index of entity who owns the bone in question
} ragCallbackBoneSnap_t;

#define RAG_CALLBACK_BONEIMPACT			4
typedef struct
{
	char			boneName[128]; //name of the bone in question
	int				entNum; //index of entity who owns the bone in question
} ragCallbackBoneImpact_t;

#define RAG_CALLBACK_BONEINSOLID		5
typedef struct
{
	vec3_t			bonePos; //world coordinate position of the bone
	int				entNum; //index of entity who owns the bone in question
	int				solidCount; //higher the count, the longer we've been in solid (the worse off we are)
} ragCallbackBoneInSolid_t;

#define RAG_CALLBACK_TRACELINE			6
typedef struct
{
	trace_t			tr;
	vec3_t			start;
	vec3_t			end;
	vec3_t			mins;
	vec3_t			maxs;
	int				ignore;
	int				mask;
} ragCallbackTraceLine_t;

#define	MAX_CG_SHARED_BUFFER_SIZE		2048

//----------------------------------------------

typedef struct
{
	int				APIversion;

	void			(*Print)( const char *fmt );
	void			(*Error)( const char *fmt );
	int				(*Milliseconds) ( void );
	int				(*RealTime)( qtime_t *qtime );

	void			(*PrecisionTimer_Start)( void **theNewTimer );
	int				(*PrecisionTimer_End)( void *theTimer );
	
	void			(*Cvar_Register)( vmCvar_t *cvar, const char *varName, const char *defaultValue, int flags );
	void			(*Cvar_Update)( vmCvar_t *cvar );
	void			(*Cvar_Set)( const char *var_name, const char *value );
	void			(*Cvar_VariableStringBuffer)( const char *var_name, char *buffer, int bufsize );
	void			(*Cvar_GetHiddenVarValue)( const char *name );

	int				(*Argc)( void );
	void			(*Argv)( int n, char *buffer, int bufferLength );
	void			(*Args)( char *buffer, int bufferLen );

	int				(*FS_FOpenFile)( const char *qpath, fileHandle_t *f, fsMode_t mode );
	void			(*FS_Read)( void *buffer, int len, fileHandle_t f );
	void			(*FS_Write)( const void *buffer, int len, fileHandle_t f );
	void			(*FS_FCloseFile)( fileHandle_t f );
	void			(*FS_GetFileList)( const char *path, const char *extension, char *listbuf, int bufsize );

	void			(*SendConsoleCommand)( const char *text );
	void			(*AddCommand)( const char *cmdName );
	void			(*RemoveCommand)( const char *cmdName );
	void			(*SendClientCommand)( const char *s );

	void			(*UpdateScreen)( void );

	void			(*CM_LoadMap)( const char *mapname, bool SubBSP );
	int				(*CM_NumInlineModels)( void );
	clipHandle_t	(*CM_InlineModel)( int index );
	clipHandle_t	(*CM_TempBoxModel)( const vec3_t mins, const vec3_t maxs );
	clipHandle_t	(*CM_TempCapsuleModel)( const vec3_t mins, const vec3_t maxs );
	int				(*CM_PointContents)( const vec3_t p, clipHandle_t model );
	int				(*CM_TransformedPointContents)( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles );
	void			(*CM_BoxTrace)( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs,
									clipHandle_t model, int brushmask );
	void			(*CM_CapsuleTrace)( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs,
										clipHandle_t model, int brushmask );
	void			(*CM_TransformedBoxTrace)( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs,
										clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles );
	void			(*CM_TransformedCapsuleTrace)( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs,
										clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles );
	int				(*CM_MarkFragments)( int numPoints, const vec3_t *points, const vec3_t projection, int maxPoints, vec3_t pointBuffer,
										int maxFragments, markFragment_t *fragmentBuffer );

	int				(*S_GetVoiceVolume)( int entityNum );
	void			(*S_MuteSound)( int entityNum, int entChannel );
	void			(*S_StartSound)( vec3_t origin, int entityNum, int entchannel, sfxHandle_ t sfx );
	void			(*S_StartLocalSound)( sfxHandle_t sfx, int channelNum );
	void			(*S_ClearLoopingSounds)( void );
	void			(*S_AddLoopingSound)( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
	void			(*S_UpdateEntityPosition)( int entityNum, const vec3_t origin );
	void			(*S_AddRealLoopingSound)( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
	void			(*S_StopLoopingSound)( int entityNum );
	void			(*S_Respatialize)( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater );
	void			(*S_ShutUp)( bool shutUpFactor );
	sfxHandle_t		(*S_RegisterSound)( const char *sample );
	void			(*S_StartBackgroundTrack)( const char *intro, const char *loop, bool bReturnWithoutStarting );
	void			(*S_UpdateAmbientSet)( const char *name, vec3_t origin );
	void			(*AS_ParseSets)( void );
	void			(*AS_AddPrecacheEntry)( const char *name );
	int				(*S_AddLocalSet)( const char *name, vec3_t listener_origin, vec3_t origin, int entID, int time );
	sfxHandle_t		(*AS_GetBModelSound)( const char *name, int stage );
	void			(*S_StopBackgroundTrack)( void );

	void			(*R_LoadWorldMap)( const char *mapname );
	qhandle_t		(*R_RegisterModel)( const char *name );
	qhandle_t		(*R_RegisterSkin)( const char *name );
	qhandle_t		(*R_RegisterShader)( const char *name );
	qhandle_t		(*R_RegisterShaderNoMip)( const char *name );
	qhandle_t		(*R_RegisterFont)( const char *fontName );
	int				(*R_Font_StrLenPixels)( const char *text, const int iFontIndex, const float scale );
	int				(*R_Font_StrLenChars)( const char *text );
	int				(*R_Font_HeightPixels)( const int iFontIndex, const float scale );
	void			(*R_Font_DrawString)( int ox, int oy, const char *text, const float *rgba, const int setIndex, int iCharLimit, const float scale );
	void			(*R_ClearScene)( void );
	void			(*R_ClearDecals)( void );
	void			(*R_AddRefEntityToScene)( const refEntity_t *re );
	void			(*R_AddPolyToScene)( qhandle_t hShader, int numVerts, const polyVert_t *verts );
	void			(*R_AddPolysToScene)( qhandle_t hShader , int numVertsPerPoly, const polyVert_t *verts, int num );
	void			(*R_AddDecalToScene)( qhandle_t shader, const vec3_t origin, const vec3_t dir, float orientation, float r, float g, float b, float a, bool alphaFade, float radius, bool temporary );
	int				(*R_LightForPoint)( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir );
	void			(*R_AddLightToScene)( const vec3_t org, float intensity, float r, float g, float b );
	void			(*R_AddAdditiveLightToScene)( const vec3_t org, float intensity, float r, float g, float b );
	void			(*R_RenderScene)( const refdef_t *fd );
	void			(*R_SetColor)( const float *rgba );
	void			(*R_DrawStretchPic)( float x, float y, float w, float h, 
							   float s1, float t1, float s2, float t2, qhandle_t hShader );
	void			(*R_ModelBounds)( clipHandle_t model, vec3_t mins, vec3_t maxs );
	int				(*R_LerpTag)( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, 
					   float frac, const char *tagName );
	void			(*R_DrawRotatePic)( float x, float y, float w, float h, 
				   float s1, float t1, float s2, float t2,float a, qhandle_t hShader );
	void			(*R_DrawRotatePic2)( float x, float y, float w, float h, 
				   float s1, float t1, float s2, float t2,float a, qhandle_t hShader );
	void			(*R_SetRangeFog)( float range );
	void			(*R_SetRefractProp)( float alpha, float stretch, bool prepost, bool negate );
	void			(*R_RemapShader)( const char *oldShader, const char *newShader, const char *timeOffset );
	void			(*R_GetLightStyle)( int style, color4ub_t color );
	void			(*R_SetLightStyle)( int style, int color );
	void			(*R_GetBModelVerts)( int bmodelIndex, vec3_t *verts, vec3_t normal );
	void			(*R_GetDistanceCull)( float *f );
	void			(*R_GetRealRes)( int *w, int *h );
	void			(*R_AutomapElevAdj)( float newHeight );
	bool			(*R_InitWireframeAutomap)( void );
	bool			(*R_inPVS)( const vec3_t p1, const vec3_t p2, byte *mask );
	void			(*JKG_OverrideServerFrame)( qhandle_t shader, int frame, int time );
	void			(*JKG_GetColorTable)( float **table );
	void			(*R_AddMiniRefEntityToScene)( miniRefEntity_t *ent );
	
	bool			(*Language_IsAsian)( void );
	bool			(*Language_UsesSpaces)( void );
	unsigned int	(*Language_ReadCharFromString)( const char *psText, int *piAdvanceCount, bool *pbIsTrailingPunctuation );
	int				(*SP_GetStringTextString)( const char *text, char *buffer, int bufferLength );

	bool			(*ROFF_Clean)( void );
	void			(*ROFF_UpdateEntities)( void );
	int				(*ROFF_Cache)( char *file );
	bool			(*ROFF_Play)( int entID, int roffID, bool doTranslation );
	bool			(*ROFF_PurgeEnt)( int entID );

	void			(*G2_ListModelSurfaces)( void *ghlInfo );
	void			(*G2_ListModelBones)( void *ghlInfo, int frame );
	void			(*G2_SetGhoul2ModelIndexes)( void *ghoul2, qhandle_t *modelList, qhandle_t *skinList );
	bool			(*G2_HaveWeGhoul2Models)( void *ghoul2 );
	bool			(*G2API_GetBoltMatrix)( void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix,
								const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale);
	bool			(*G2API_GetBoltMatrix_NoReconstruct)( void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix,
								const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale );
	bool			(*G2API_GetBoltMatrix_NoRecNoRot)( void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix,
								const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale );
	int				(*G2API_InitGhoul2Model)( void **ghoul2Ptr, const char *fileName, int modelIndex, qhandle_t customSkin,
						  qhandle_t customShader, int modelFlags, int lodBias );
	bool			(*G2API_SetSkin)( void *ghoul2, int modelIndex, qhandle_t customSkin, qhandle_t renderSkin );
	void			(*G2API_CollisionDetect)( CollisionRecord_t *collRecMap, void *ghoul2, const vec3_t angles, const vec3_t position, int frameNumber,
												int entNum, const vec3_t rayStart, const vec3_t rayEnd, const vec3_t scale, int traceFlags, int useLod, float fRadius );
	void			(*G2API_CollisionDetectCache)( CollisionRecord_t *collRecMap, void *ghoul2, const vec3_t angles, const vec3_t position, int frameNumber,
												int entNum, const vec3_t rayStart, const vec3_t rayEnd, const vec3_t scale, int traceFlags, int useLod, float fRadius );
	void			(*G2API_CleanGhoul2Models)( void **ghoul2ptr );
	bool			(*G2API_SetBoneAngles)( void *ghoul2, int modelIndex, const char *boneName, const vec3_t angles, const int flags,
								const int up, const int right, const int forward, qhandle_t *modelList,
								int blendTime , int currentTime );
	bool			(*G2API_SetBoneAnim)( void *ghoul2, const int modelIndex, const char *boneName, const int startFrame, const int endFrame,
							  const int flags, const float animSpeed, const int currentTime, const float setFrame , const int blendTime );
	bool			(*G2API_GetBoneAnim)( void *ghoul2, const char *boneName, const int currentTime, float *currentFrame,
						   int *startFrame, int *endFrame, int *flags, float *animSpeed, int *modelList, const int modelIndex );
	bool			(*G2API_GetBoneFrame)( void *ghoul2, const char *boneName, const int currentTime, float *currentFrame, int *modelList, const int modelIndex );
	void			(*G2API_GetGLAName)( void *ghoul2, int modelIndex, char *fillBuf );
	int				(*G2API_CopyGhoul2Instance)( void *g2From, void *g2To, int modelIndex );
	void			(*G2API_CopySpecificGhoul2Model)( void *g2From, int modelFrom, void *g2To, int modelTo );
	void			(*G2API_DuplicateGhoul2Instance)( void *g2From, void **g2To );
	bool			(*G2API_HasGhoul2ModelOnIndex)( void *ghlInfo, int modelIndex );
	bool			(*G2API_RemoveGhoul2Model)( void *ghlInfo, int modelIndex );
	bool			(*G2API_SkinlessModel)( void *ghlInfo, int modelIndex );
	int				(*G2API_GetNumGoreMarks)( void *ghlInfo, int modelIndex );
	void			(*G2API_AddSkinGore)( void *ghlInfo,SSkinGoreData *gore );
	void			(*G2API_ClearSkinGore)( void *ghlInfo );
	int				(*G2API_Ghoul2Size)( void *ghlInfo );
	int				(*G2API_AddBolt)( void *ghoul2, int modelIndex, const char *boneName );
	bool			(*G2API_AttachEnt)( int *boltInfo, void *ghlInfoTo, int toBoltIndex, int entNum, int toModelNum );
	void			(*G2API_SetBoltInfo)( void *ghoul2, int modelIndex, int boltInfo );
	bool			(*G2API_SetRootSurface)( void *ghoul2, const int modelIndex, const char *surfaceName );
	bool			(*G2API_SetSurfaceOnOff)( void *ghoul2, const char *surfaceName, const int flags );
	bool			(*G2API_SetNewOrigin)( void *ghoul2, const int boltIndex );
	bool			(*G2API_DoesBoneExist)( void *ghoul2, int modelIndex, const char *boneName );
	int				(*G2API_GetSurfaceRenderStatus)( void *ghoul2, const int modelIndex, const char *surfaceName );
	int				(*G2API_GetTime)( void );
	void			(*G2API_SetTime)( int time, int clock );
	void			(*G2API_AbsurdSmoothing)( void *ghoul2, bool status );
	void			(*G2API_SetRagDoll)( void *ghoul2, sharedRagDollParams_t *params );
	void			(*G2API_AnimateG2Models)( void *ghoul2, int time, sharedRagDollUpdateParams_t *params );
	bool			(*G2API_RagPCJConstraint)( void *ghoul2, const char *boneName, vec3_t min, vec3_t max );
	bool			(*G2API_RagPCJGradientSpeed)( void *ghoul2, const char *boneName, const float speed );
	bool			(*G2API_RagEffectorGoal)( void *ghoul2, const char *boneName, vec3_t pos); //override an effector bone's goal position (world coordinates)
	bool			(*G2API_GetRagBonePos)( void *ghoul2, const char *boneName, vec3_t pos, vec3_t entAngles, vec3_t entPos, vec3_t entScale); //current position of said bone is put into pos (world coordinates)
	bool			(*G2API_RagEffectorKick)( void *ghoul2, const char *boneName, vec3_t velocity); //add velocity to a rag bone
	bool			(*G2API_RagForceSolve)( void *ghoul2, bool force);
	bool			(*G2API_SetBoneIKState)( void *ghoul2, int time, const char *boneName, int ikState, sharedSetBoneIKStateParams_t *params );
	bool			(*G2API_IKMove)( void *ghoul2, int time, sharedIKMoveParams_t *params );
	bool			(*G2API_RemoveBone)( void *ghoul2, const char *boneName, int modelIndex );
	void			(*G2API_AttachInstanceToEntNum)( void *ghoul2, int entityNum, bool server);
	void			(*G2API_ClearAttachedInstance)( int entityNum );
	void			(*G2API_CleanEntAttachments)( void );
	bool			(*G2API_OverrideServer)( void *serverInstance );
	void			(*G2API_GetSurfaceName)( void *ghoul2, int surfNumber, int modelIndex, char *fillBuf );

	fxHandle_t		(*FX_RegisterEffect)( const char *file );
	void			(*FX_PlayEffect)( const char *file, vec3_t org, vec3_t fwd, int vol, int rad );
	void			(*FX_PlayEntityEffect)( const char *file, vec3_t org, 
						vec3_t axis[3], const int boltInfo, const int entNum, int vol, int rad );
	void			(*FX_PlayEffectID)( int id, const vec3_t org, const vec3_t fwd, int vol, int rad );
	void			(*FX_PlayPortalEffectID)( int id, vec3_t org, vec3_t fwd, int vol, int rad );
	void			(*FX_PlayEntityEffectID)( int id, const vec3_t org, 
						vec3_t axis[3], const int boltInfo, const int entNum, int vol, int rad );
	void			(*FX_PlayBoltedEffectID)( int id, vec3_t org, 
						void *ghoul2, const int boltNum, const int entNum, const int modelNum, int iLooptime, qboolean isRelative );
	void			(*FX_AddScheduledEffects)( bool skyPortal );
	void			(*FX_Draw2DEffects)( float screenXScale, float screenYScale );
	int				(*FX_InitSystem)( refdef_t *refdef );
	void			(*FX_SetRefDef)( refdef_t *refdef );
	bool			(*FX_FreeSystem)( void );
	void			(*FX_Reset)( void );
	void			(*FX_AdjustTime)( int time );
	void			(*FX_AddPoly)( addpolyArgStruct_t *p );
	void			(*FX_AddBezier)( addbezierArgStruct_t *p );
	void			(*FX_AddPrimitive)( effectTrailArgStruct_t *p );
	void			(*FX_AddSprite)( addspriteArgStruct_t *p );
	void			(*FX_AddElectricity)( addElectricityArgStruct_t *p );
	void			(*FX_AddLine)( const vec3_t start, const vec3_t end, float size1, float size2, float sizeParm,
									float alpha1, float alpha2, float alphaParm,
									const vec3_t sRGB, const vec3_t eRGB, float rgbParm,
									int killTime, qhandle_t shader, int flags );
	char*			(*FX_GetSharedMemory)( void );	// JKG ADD
SEffectTemplate*	(*FX_GetEffectCopy)( fxHandle_t handle, fxHandle_t *newHandle ); // JKG ADD
SEffectTemplate*	(*FX_GetEffectCopy2)( const char *file, fxHandle_t *newHandle ); // JKG ADD
CPrimitiveTemplate*	(*FX_GetPrimitiveCopy)( SEffectTemplate *efxFile, const char *componentName ); // JKG ADD

	void			(*GetGlconfig)( glconfig_t *glconfig );
	void			(*GetGameState)( gameState_t *gamestate );
	void			(*GetCurrentSnapshotNumber)( int *snapshotNumber, int *serverTime );
	bool			(*GetSnapshot)( int snapshotNumber, snapshot_t *snapshot );
	bool			(*GetDefaultState)( int entityIndex, entityState_t *state );
	bool			(*GetServerCommand)( int serverCommandNumber );
	int				(*GetCurrentCmdNumber)( void );
	bool			(*GetUserCmd)( int cmdNumber, usercmd_t *ucmd );
	void			(*SetUserCmdValue)( int stateValue, float sensitivityScale, float mPitchOverride, float mYawOverride, float mSensitivityOverride, int fpSel, int invenSel, qboolean fighterControls );
	void			(*SetClientForceAngle)( int time, vec3_t angle );
	void			(*SetClientTurnExtent)( float turnAdd, float turnSub, int turnTime );
	void			(*OpenUIMenu)( int menuID );
	bool			(*GetEntityToken)( char *buffer, int bufferSize );
	void			(*CG_RegisterSharedMemory)( char *memory );
	float**			(*JKG_GetViewAngles)( void );
	void			(*JKG_SetViewAngles)( vec3_t viewangles );

	bool			(*Key_IsDown)( int keynum );
	int				(*Key_GetCatcher)( void );
	void			(*Key_SetCatcher)( int catcher );
	int				(*Key_GetKey)( const char *binding );

	int				(*PC_AddGlobalDefine)( char *define );
	int				(*PC_LoadSource)( const char *filename );
	int				(*PC_FreeSource)( int handle );
	int				(*PC_ReadToken)( int handle, pc_token_t *pc_token );
	int				(*PC_SourceFileAndLine)( int handle, char *filename, int *line );
	int				(*PC_LoadGlobalDefines)( const char *filename );
	void			(*PC_RemoveAllGlobalDefines)( void );

	void			(*SnapVector)( float *v );

	int				(*CIN_PlayCinematic)( const char *arg0, int xpos, int ypos, int width, int height, int bits );
	e_status		(*CIN_StopCinematic)( int handle );
	e_status		(*CIN_RunCinematic)( int handle );
	void			(*CIN_DrawCinematic)( int handle );
	void			(*CIN_SetExtents)( int handle, int x, int y, int w, int h );
	
	void			(*RMG_Init)( int terrainID, const char *terrainInfo );
	void			(*RE_InitRendererTerrain)( const char *info );
	void			(*R_WeatherContentsOveride)( int contents );
	void			(*R_WorldEffectCommand)( const char *cmd );
	void			(*WE_AddWeatherZone)( const vec3_t mins, const vec3_t maxs );

uiCrossoverExports_t (*CO_InitCrossover)( cgCrossoverExports_t *uiImports );
	void			(*CO_Shutdown)( void );
	void			(*Syscall_UI)( void );
	void			(*Syscall_CG)( void );
} cgameImport_t;

#endif // __CG_PUBLIC_H
