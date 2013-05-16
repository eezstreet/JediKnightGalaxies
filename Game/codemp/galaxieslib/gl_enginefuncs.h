
#ifndef _GL_ENGINEFUNCS_H
#define _GL_ENGINEFUNCS_H

#define	CVAR_ARCHIVE		0x00000001		// set to cause it to be saved to vars.rc
											// used for system variables, not for player
											// specific configurations
#define	CVAR_USERINFO		0x00000002		// sent to server on connect or change
#define	CVAR_SERVERINFO		0x00000004		// sent in response to front end requests
#define	CVAR_SYSTEMINFO		0x00000008		// these cvars will be duplicated on all clients
#define	CVAR_INIT			0x00000010		// don't allow change from console at all,
											// but can be set from the command line
#define	CVAR_LATCH			0x00000020		// will only change when C code next does
											// a Cvar_Get(), so it can't be changed
											// without proper initialization.  modified
											// will be set, even though the value hasn't
											// changed yet
#define	CVAR_ROM			0x00000040		// display only, cannot be set by user at all (can be set by code)
#define	CVAR_USER_CREATED	0x00000080		// created by a set command
#define	CVAR_TEMP			0x00000100		// can be set even when cheats are disabled, but is not archived
#define CVAR_CHEAT			0x00000200		// can not be changed if cheats are disabled
#define CVAR_NORESTART		0x00000400		// do not clear when a cvar_restart is issued
#define CVAR_INTERNAL		0x00000800		// cvar won't be displayed, ever (for passwords and such)
#define	CVAR_PARENTAL		0x00001000		// lets cvar system know that parental stuff needs to be updated

#define MAX_QPATH 64

typedef unsigned char byte;
typedef enum {qfalse, qtrue}	qboolean;
typedef int fileHandle_t;
typedef int sfxHandle_t;

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];

#define	MAX_STRING_CHARS	1024	// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS	1024	// max tokens resulting from Cmd_TokenizeString
#define	MAX_TOKEN_CHARS		1024	// max length of an individual token

#define	MAX_INFO_STRING		1024
#define	MAX_INFO_KEY		1024
#define	MAX_INFO_VALUE		1024

#define	BIG_INFO_STRING		8192  // used for system info key only
#define	BIG_INFO_KEY		  8192
#define	BIG_INFO_VALUE		8192


typedef struct cvar_s {
	char		*name;
	char		*string;
	char		*resetString;		// cvar_restart will reset to this value
	char		*latchedString;		// for CVAR_LATCH vars
	int			flags;
	qboolean	modified;			// set each time the cvar is changed
	int			modificationCount;	// incremented each time the cvar is changed
	float		value;				// atof( string )
	int			integer;			// atoi( string )
	struct cvar_s *next;
	struct cvar_s *hashNext;
} cvar_t;

// parameters to the main Error routine
typedef enum {
	ERR_FATAL,					// exit the entire game with a popup window
	ERR_DROP,					// print to console and disconnect from game
	ERR_SERVERDISCONNECT,		// don't kill server
	ERR_DISCONNECT,				// client disconnected from the server
	ERR_NEED_CD					// pop up the need-cd dialog
} errorParm_t;

typedef enum {
	FS_READ,
	FS_WRITE,
	FS_APPEND,
	FS_APPEND_SYNC
} fsMode_t;

typedef enum {
	FS_SEEK_CUR,
	FS_SEEK_END,
	FS_SEEK_SET
} fsOrigin_t;

cvar_t *Cvar_FindVar(const char *cvarname);
void Cvar_Set2(const char *cvarname, const char *newvalue, int force);
cvar_t *Cvar_Get(const char *cvarname, const char *value, int flags);
int Cvar_GetValueInt(const char *cvarname);
float Cvar_GetValueFloat(const char *cvarname);
const char *Cvar_String(const char *cvarname);
char *Cvar_VariableString(const char *cvarname);

void Com_Printf(const char *fmt, ...);
void Com_DPrintf(const char *fmt, ...);

void Com_Error(int level, const char *fmt, ...);

int FS_FOpenFileRead( const char *filename, fileHandle_t *file, qboolean uniqueFILE );


int FS_FOpenFileByMode (const char *filename, fileHandle_t *file, fsMode_t mode);
int FS_Write( void *buffer, int len, fileHandle_t file );
int FS_Read( void *buffer, int len, fileHandle_t file );
void FS_FCloseFile( fileHandle_t file );

void S_RawSamples( int samples, int rate, int width, int s_channels, const char *data, float volume, qboolean additive );

int VM_CallUI(int command, ...);
int VM_CallCG(int callnum, ...);

void CBuf_AddText(const char *cmdbuffer);
void CBuf_InsertText(const char *cmdbuffer);

void RoQShutdown();

int Sys_Milliseconds (void);
int CL_ScaledMilliseconds(void);

void Cmd_AddCommand(const char *name, void (*funcptr)(void));
void Cmd_RemoveCommand(const char *name);

void Cmd_SetCommand(const char *name, void (*funcptr)(void));

// This line here is a bit brainfucky, so read it as follows:
// void (*)(void) Cmd_RedirectCommand(const char *name, void (*funcptr)(void));
void(*Cmd_RedirectCommand(const char *name, void (*funcptr)(void)))(void);

static void FS_ReplaceSeparators( char *path );

char *FS_BuildOSPath( const char *base, const char *game, const char *qpath );
qboolean FS_FileExists( const char *file );
int FS_CopyFile(const char *filename);
int FS_ReadFile( const char *qpath, void **buffer );
void FS_FreeFile( void *buffer );

void Com_sprintf( char *dest, int size, const char *fmt, ...);

//int Cmd_Argc();
static int __inline Cmd_Argc() {return *(int *)0xB3CC08;}

const char *Cmd_Argv( int arg );

void Cmd_ExecuteString(const char *commands);

char	*  va( char *format, ... );

void Q_strncpyz( char *dest, const char *src, int destsize );                 
int Q_stricmpn (const char *s1, const char *s2, int n);
int Q_strncmp (const char *s1, const char *s2, int n);
int Q_stricmp (const char *s1, const char *s2);
char *Q_strlwr( char *s1 );
char *Q_strupr( char *s1 );
void Q_strcat( char *dest, int size, const char *src );


#endif