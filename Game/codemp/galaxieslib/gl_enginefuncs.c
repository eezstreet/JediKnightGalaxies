#include "gl_enginefuncs.h"
#include <stdio.h>
#include <windows.h>

#pragma warning(disable : 4996)

static int Q_vsnprintf( char *dest, int size, const char *fmt, va_list argptr ) {
	int ret;

#ifdef _WIN32
	ret = _vsnprintf( dest, size-1, fmt, argptr );
#else
	ret = vsnprintf( dest, size, fmt, argptr );
#endif

	dest[size-1] = '\0';
	if ( ret < 0 || ret >= size ) {
		return -1;
	}
	return ret;
}

void Com_sprintf( char *dest, int size, const char *fmt, ...) {
	int		ret;
	va_list		argptr;

	va_start (argptr,fmt);
	ret = Q_vsnprintf (dest, size, fmt, argptr);
	va_end (argptr);
	if (ret == -1) {
		Com_Printf ("Com_sprintf2: overflow of %i bytes buffer\n", size);
	}
}

char	*  va( char *format, ... ) {
	va_list		argptr;
	#define	MAX_VA_STRING	32000
	#define MAX_VA_BUFFERS 4
	static char		string[MAX_VA_BUFFERS][MAX_VA_STRING];	// in case va is called by nested functions
	static int		index = 0;
	char	*buf;


	va_start (argptr, format);
	buf = (char *)&string[index++ & 3];
	Q_vsnprintf (buf, MAX_VA_STRING-1, format, argptr);
	va_end (argptr);

	return buf;
}


int Cvar_GetValueInt(const char *cvarname) {
	cvar_t *cvar;
	cvar = Cvar_FindVar(cvarname);
	if (cvar) {
		return cvar->integer;
	}
	return 0;
}

float Cvar_GetValueFloat(const char *cvarname) {
	cvar_t *cvar;
	cvar = Cvar_FindVar(cvarname);
	if (cvar) {
		return cvar->value;
	}
	return 0;
}


cvar_t *Cvar_FindVar(const char *cvarname) {
	__asm
	{
		mov edi, cvarname
		mov eax, 0x4393B0
		call eax
	}
}




void __declspec(naked) Cmd_ExecuteString(const char *commands) {
	(void)commands;
	__asm
	{
		push 0x436ED0
		ret
	}
}


void __declspec(naked) Cvar_Set2(const char *cvarname, const char *newvalue, int force) {
	(void)cvarname;
	(void)newvalue;
	(void)force;
	__asm
	{
		push 0x4396A0
		ret
	}
}

cvar_t __declspec(naked) * Cvar_Get(const char *cvarname, const char *value, int flags) {
	(void)cvarname;
	(void)value;
	(void)flags;
	__asm
	{
		push 0x439470
		ret
	}
}

void __declspec(naked) Com_Printf(const char *fmt, ...) {
	(void)fmt;
	__asm {
		push 0x437080
		ret
	}	
}

void __declspec(naked) Com_Error(int level, const char *fmt, ...) {
	(void)fmt;
	(void)level;
	__asm {
		push 0x437290
		ret
	}	
}

void __declspec(naked) Com_DPrintf(const char *fmt, ...) {
	(void)fmt;
	__asm {
		push 0x437240
		ret
	}	
}

int __declspec(naked) FS_FOpenFileRead( const char *filename, fileHandle_t *file, qboolean uniqueFILE ) {
	(void)filename;
	(void)file;
	(void)uniqueFILE;
	__asm {
		push 0x43B5E0
		ret
	}
}

int __declspec(naked) FS_Read( void *buffer, int len, fileHandle_t f ) {
	(void)buffer;
	(void)len;
	(void)f;
	__asm {
		mov eax, [esp+0xC]
		push [esp+0x8]
		push [esp+0x8]	// +4 due to the previous push
		mov ebx, 0x43C060
		call ebx
		add esp, 0x8
		ret
	}
}

int __declspec(naked) FS_FOpenFileByMode (const char *filename, fileHandle_t *file, fsMode_t mode) {
	(void)filename;
	(void)file;
	(void)mode;
	__asm {
		push 0x43EF70
		ret
	}
}

int __declspec(naked) FS_Write(void *buffer, int len, fileHandle_t file) {
	(void)buffer;
	(void)len;
	(void)file;
	__asm {
		push 0x43C140
		ret
	}
}

void __declspec(naked) FS_FCloseFile( fileHandle_t file ) {
	(void)file;
	__asm {
		push 0x43B1A0
		ret
	}
}

void S_RawSamples( int samples, int rate, int width, int s_channels, const char *data, float volume, qboolean additive ) {
	__asm {
		push additive
		push volume
		push s_channels
		push width
		push rate
		push samples
		mov edi, data
		mov eax, 0x46B4B0
		call eax
		add esp, 0x18
	}
}

int __declspec(naked) VM_CallUI(int command, ...) {
	(void)command;
	__asm {
		mov eax, DS:[0xB28224]
		test eax,eax
		je skip
		push 0x44AE90
		ret
		skip:
		ret
	}
}

int __declspec(naked) VM_CallCG(int callnum, ...) {
	(void)callnum;
	__asm
	{
		mov eax, DS:[0x8AF0FC]
		test eax,eax
		je skip
		push 0x44AE90
		ret
		skip:
		ret
	}
}

void CBuf_AddText(const char *cmdbuffer) {
	__asm
	{
		mov edi, cmdbuffer
		mov eax,0x4366A0
		call eax
	}
}

void CBuf_InsertText(const char *cmdbuffer) {
	__asm
	{
		mov edi, cmdbuffer
		mov eax, 0x436700
		call eax
	}
}

void __declspec(naked) RoQShutdown() {
	__asm {
		push 0x416250
		ret
	}
}

void Cmd_AddCommand(const char *name, void (*funcptr)(void)) {
	__asm
	{
		mov eax, name
		mov ebx, funcptr
		mov edx, 0x436DA0
		call edx
	}
}


typedef struct cmd_function_s
{
  struct cmd_function_s *next;
  char *name;
  void (*function)(void);
} cmd_function_t;


void Cmd_SetCommand(const char *name, void (*funcptr)(void))
{
	cmd_function_t *cmd;

	for (cmd = (cmd_function_t *)0xB3CC18; cmd; cmd = cmd->next)
	{
		if (!stricmp(name, cmd->name)) {
			cmd->function = funcptr;
			return;
		}
	}
}

void(*Cmd_RedirectCommand(const char *name, void (*funcptr)(void)))(void) 
{
	cmd_function_t *cmd;
	void (*temp)(void);
	
	if (!name) return NULL;

	for (cmd = (cmd_function_t *)0xB3CC18; cmd; cmd = cmd->next)
	{
		if (cmd->name && stricmp(name, cmd->name) == 0) {
			temp = cmd->function;
			cmd->function = funcptr;
			return temp;
		}
	}
	return NULL;
}

//int __inline Cmd_Argc() {return *(int *)0xB3CC08;}

const char *Cmd_Argv( int arg ) {
	if ( /*(unsigned)*/arg >= Cmd_Argc() ) {
		return "";
	}
	return ((const char **)0xB39808)[arg];
}

void Cmd_RemoveCommand(const char *name) {
	__asm
	{
		mov edx, name
		mov eax,0x436E30
		call eax
	}
}

int Sys_Milliseconds (void) {
	int			sys_curtime;
	qboolean	*initialized = (qboolean *)0x12DD9D4;
	int			*sys_timeBase = (int *)0x12DD9D0;


	if (!*initialized) {
		*sys_timeBase = timeGetTime();
		*initialized = qtrue;
	}
	sys_curtime = timeGetTime() - *sys_timeBase;

	return sys_curtime;
}

int CL_ScaledMilliseconds(void) {
	cvar_t *com_timescale = *(cvar_t **)0xB43D1C;
	return Sys_Milliseconds() * ( int ) com_timescale->value;
}


void Q_strncpyz( char *dest, const char *src, int destsize ) {
  // bk001129 - also NULL dest
  if ( !dest ) {
    Com_Error( ERR_FATAL, "Q_strncpyz: NULL dest" );
  }
	if ( !src ) {
		Com_Error( ERR_FATAL, "Q_strncpyz: NULL src" );
	}
	if ( destsize < 1 ) {
		Com_Error(ERR_FATAL,"Q_strncpyz: destsize < 1" ); 
	}

	strncpy( dest, src, destsize-1 );
  dest[destsize-1] = 0;
}
                 
int Q_stricmpn (const char *s1, const char *s2, int n) {
	int		c1, c2;

	// bk001129 - moved in 1.17 fix not in id codebase
        if ( s1 == NULL ) {
           if ( s2 == NULL )
             return 0;
           else
             return -1;
        }
        else if ( s2==NULL )
          return 1;


	
	do {
		c1 = *s1++;
		c2 = *s2++;

		if (!n--) {
			return 0;		// strings are equal until end point
		}
		
		if (c1 != c2) {
			if (c1 >= 'a' && c1 <= 'z') {
				c1 -= ('a' - 'A');
			}
			if (c2 >= 'a' && c2 <= 'z') {
				c2 -= ('a' - 'A');
			}
			if (c1 != c2) {
				return c1 < c2 ? -1 : 1;
			}
		}
	} while (c1);
	
	return 0;		// strings are equal
}

int Q_strncmp (const char *s1, const char *s2, int n) {
	int		c1, c2;
	
	do {
		c1 = *s1++;
		c2 = *s2++;

		if (!n--) {
			return 0;		// strings are equal until end point
		}
		
		if (c1 != c2) {
			return c1 < c2 ? -1 : 1;
		}
	} while (c1);
	
	return 0;		// strings are equal
}

int Q_stricmp (const char *s1, const char *s2) {
	return (s1 && s2) ? Q_stricmpn (s1, s2, 99999) : -1;
}


char *Q_strlwr( char *s1 ) {
    char	*s;

    s = s1;
	while ( *s ) {
		*s = (char)tolower(*s);
		s++;
	}
    return s1;
}

char *Q_strupr( char *s1 ) {
    char	*s;

    s = s1;
	while ( *s ) {
		*s = (char)toupper(*s);
		s++;
	}
    return s1;
}


// never goes past bounds or leaves without a terminating 0
void Q_strcat( char *dest, int size, const char *src ) {
	int		l1;

	l1 = strlen( dest );
	if ( l1 >= size ) {
		Com_Error( ERR_FATAL, "Q_strcat: already overflowed" );
	}
	Q_strncpyz( dest + l1, src, size - l1 );
}


/*
====================
FS_ReplaceSeparators

Fix things up differently for win/unix/mac
====================
*/
#define	PATH_SEP '\\'
static void FS_ReplaceSeparators( char *path ) {
	char	*s;

	for ( s = path ; *s ; s++ ) {
		if ( *s == '/' || *s == '\\' ) {
			*s = PATH_SEP;
		}
	}
}

const char *Cvar_String(const char *cvarname) {
	cvar_t *cvar;
	cvar = Cvar_FindVar(cvarname);
	if (cvar) {
		if (cvar->string) {
			return cvar->string;
		} else {
			return "";
		}
	} else {
		return "";
	}
}

/*
===================
FS_BuildOSPath

Qpath may have either forward or backwards slashes
===================
*/
#define	MAX_OSPATH			256	
char *FS_BuildOSPath( const char *base, const char *game, const char *qpath ) {
	char	temp[MAX_OSPATH];
	static char ospath[2][MAX_OSPATH];
	static int toggle;
	
	toggle ^= 1;		// flip-flop to allow two returns without clash

	if( !game || !game[0] ) {
		game = Cvar_String( "fs_game" );
	}

	Com_sprintf( temp, sizeof(temp), "/%s/%s", game, qpath );
	FS_ReplaceSeparators( temp );	
	Com_sprintf( ospath[toggle], sizeof( ospath[0] ), "%s%s", base, temp );
	
	return ospath[toggle];
}

/*
================
FS_FileExists

Tests if the file exists in the current gamedir, this DOES NOT
search the paths.  This is to determine if opening a file to write
(which always goes into the current gamedir) will cause any overwrites.
NOTE TTimo: this goes with FS_FOpenFileWrite for opening the file afterwards
================
*/

qboolean FS_FileExists( const char *file )
{
	FILE *f;
	char *testpath;

	testpath = FS_BuildOSPath( Cvar_String( "fs_homepath" ), Cvar_String( "fs_game" ), file );

	f = fopen( testpath, "rb" );
	if (f) {
		fclose( f );
		return qtrue;
	}
	return qfalse;
}