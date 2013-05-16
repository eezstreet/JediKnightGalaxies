/////////////////////////////////
//
//  Engine function definitions
//
//
/////////////////////////////////

#include "gls_enginefuncs.h"

#ifdef _WIN32
	#define _CRT_SECURE_NO_WARNINGS
	#include <windows.h>
#else
	// TODO: Linux includes
        #include <stdarg.h>
#endif

#include <stdio.h>

#ifdef _WIN32
// Windows defs here
void (*Com_Printf)( const char *fmt, ... ) = (void (*)(const char *, ...))0x40FBE0;
void (*Com_DPrintf)( const char *fmt, ... ) = (void (*)(const char *, ...))0x40FDB0;
const char *(*Cmd_Argv)( int arg ) = (const char *(*)(int))0x40F490;

int	(*FS_FOpenFileByMode)( const char *qpath, fileHandle_t *f, fsMode_t mode ) = (int(*)(const char*, fileHandle_t *, fsMode_t))0x4160C0;
void (*FS_FCloseFile)( fileHandle_t f ) = (void(*)(fileHandle_t))0x4135E0;
int	(*FS_Write)	( const void *buffer, int len, fileHandle_t f ) = (int(*)(const char*, int, fileHandle_t))0x414350;

int __inline Cmd_Argc() {return *(int *)0x4DC188;}

#else
// Linux defs here
void (*Com_Printf)( const char *fmt, ... ) = (void (*)(const char *, ...))0x8072CA4;
void (*Com_DPrintf)( const char *fmt, ... ) = (void (*)(const char *, ...))0x8072ED4;
const char *(*Cmd_Argv)( int arg ) = (const char *(*)(int))0x812C264;

int	(*FS_FOpenFileByMode)( const char *qpath, fileHandle_t *f, fsMode_t mode ) = (int(*)(const char*, fileHandle_t *, fsMode_t))0x8131574;
void (*FS_FCloseFile)( fileHandle_t f ) = (void(*)(fileHandle_t))0x812D1B4;
int	(*FS_Write)	( const void *buffer, int len, fileHandle_t f ) = (int(*)(const char*, int, fileHandle_t))0x812E074;


int Cmd_Argc() {return *(int *)0x8260E20;}


#endif

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