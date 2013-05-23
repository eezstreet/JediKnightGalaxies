/*
                                                                                                            
  **    ***    ***            ***   ****                                   *               ***              
  **    ***    ***            ***  *****                                 ***               ***              
  ***  *****  ***             ***  ***                                   ***                                
  ***  *****  ***    *****    *** ******   *****    *** ****    ******  ******    *****    ***  *** ****    
  ***  *****  ***   *******   *** ******  *******   *********  ***  *** ******   *******   ***  *********   
  ***  ** **  ***  **** ****  ***  ***   ***   ***  ***   ***  ***       ***    ***   ***  ***  ***   ***   
  *** *** *** ***  ***   ***  ***  ***   *********  ***   ***  ******    ***    *********  ***  ***   ***   
   ****** ******   ***   ***  ***  ***   *********  ***   ***   ******   ***    *********  ***  ***   ***   
   *****   *****   ***   ***  ***  ***   ***        ***   ***    ******  ***    ***        ***  ***   ***   
   *****   *****   **** ****  ***  ***   ****  ***  ***   ***       ***  ***    ****  ***  ***  ***   ***   
   ****     ****    *******   ***  ***    *******   ***   ***  ***  ***  *****   *******   ***  ***   ***   
    ***     ***      *****    ***  ***     *****    ***   ***   ******    ****    *****    ***  ***   ***   
                                                                                                            
            ******** **                 ******                        *  **  **                             
            ******** **                 ******                       **  **  **                             
               **    **                 **                           **  **                                 
               **    ** **    ***       **      ** *  ****   ** **  **** **  **  ** **    ***               
               **    ******  *****      *****   ****  ****   ****** **** **  **  ******  *****              
               **    **  **  ** **      *****   **   **  **  **  **  **  **  **  **  **  ** **              
               **    **  **  *****      **      **   **  **  **  **  **  **  **  **  **  *****              
               **    **  **  **         **      **   **  **  **  **  **  **  **  **  **  **                 
               **    **  **  ** **      **      **   **  **  **  **  **  **  **  **  **  ** **              
               **    **  **  *****      **      **    ****   **  **  *** **  **  **  **  *****              
               **    **  **   ***       **      **    ****   **  **  *** **  **  **  **   ***               

*/


// BASS Simple Console Test, copyright (c) 1999-2004 Ian Luck.
#include "../game/q_shared.h"
#include "../game/bg_public.h"

#ifdef __MUSIC_ENGINE__
#include <windows.h>
#include <process.h>
#include <mmsystem.h>
#include <stdio.h>
#include <conio.h>

//#include "bass.h"
#ifdef CGAME
extern void QDECL	CG_Printf ( const char *msg, ... );		// for CGame messages...
#else
extern void QDECL	Com_Printf ( const char *msg, ... );	// for UI messages...
#endif
extern float		CG_GetMusicVolume ( void );				// cg_main.c
extern int			trap_FS_GetFileList ( const char *path, const char *extension, char *listbuf, int bufsize );
extern int			trap_FS_FOpenFile ( const char *qpath, fileHandle_t *f, fsMode_t mode );
extern void			trap_FS_Read ( void *buffer, int len, fileHandle_t f );
extern void			trap_FS_FCloseFile ( fileHandle_t f );
//extern int			Q_irand ( int value1, int value2 );
extern int			Q_irand ( int value1, int value2 );
extern void			trap_Cvar_Register ( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
vmCvar_t			fs_game;
#define MAX_SOUNDPATH	512
#define BASSDEF( f )	( WINAPI * f )						// define the functions as pointers
#include "bass.h"
char				tempfile[MAX_PATH];						// temporary BASS.DLL
HINSTANCE			bass = 0;								// bass handle
extern int trap_PC_SourceFileAndLine( int handle, char *filename, int *line );
extern int trap_PC_FreeSource( int handle );
extern int trap_PC_ReadToken( int handle, pc_token_t *pc_token );
extern int trap_PC_LoadSource( const char *filename );
float fft[1024]; // get the FFT data

// Better linkage with cgame --eez
#include "cg_local.h"

/* load BASS and the required functions */
qboolean
LoadBASS ( void )
{
	BYTE	*data;
	HANDLE	hres, hfile;
	DWORD	len, c;
	char	temppath[MAX_PATH];

	/* get the BASS.DLL resource */

	if
	(
#ifdef CGAME
		!(hres = FindResource( GetModuleHandle( "cgamex86.dll"), "BASS_DLL", RT_RCDATA)) ||
		!(len = SizeofResource( GetModuleHandle( "cgamex86.dll"), hres)) ||
		!(hres = LoadResource( GetModuleHandle( "cgamex86.dll"), hres)) ||
#else
		!(hres = FindResource( GetModuleHandle( "uix86.dll"), "BASS_DLL", RT_RCDATA)) ||
		!(len = SizeofResource( GetModuleHandle( "uix86.dll"), hres)) ||
		!(hres = LoadResource( GetModuleHandle( "uix86.dll"), hres)) ||
#endif
		!(data = LockResource( hres))
	)
	{
#ifndef CGAME
		Com_Printf( "Error: Can't get the BASS.DLL resource\n" );
#else
		CG_Printf( "Error: Can't get the BASS.DLL resource\n" );
#endif
		return ( qfalse );
	}

	/* get a temporary filename */
	GetTempPath( MAX_PATH, temppath );
	GetTempFileName( temppath, "bas", 0, tempfile );

	/* write BASS.DLL to the temporary file */
	if
	(
		INVALID_HANDLE_VALUE ==
			(hfile = CreateFile( tempfile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL))
	)
	{
#ifndef CGAME
		Com_Printf( "Error: Can't write BASS.DLL\n" );
#else
		CG_Printf( "Error: Can't write BASS.DLL\n" );
#endif
		return ( qfalse );
	}

	WriteFile( hfile, data, len, &c, NULL );
	CloseHandle( hfile );

	/* load the temporary BASS.DLL library */
	if ( !(bass = LoadLibrary( tempfile)) )
	{
#ifndef CGAME
		Com_Printf( "Error: Can't load BASS.DLL\n" );
#else
		CG_Printf( "Error: Can't load BASS.DLL\n" );
#endif
		return ( qfalse );
	}

/* "load" all the BASS functions that are to be used */
#define LOADBASSFUNCTION( f )	*( (void **) &f ) = GetProcAddress( bass, #f )
	LOADBASSFUNCTION( BASS_ErrorGetCode );
	LOADBASSFUNCTION( BASS_Init );
	LOADBASSFUNCTION( BASS_Free );
	LOADBASSFUNCTION( BASS_GetCPU );
	LOADBASSFUNCTION( BASS_MusicLoad );
	//LOADBASSFUNCTION( BASS_MusicGetLength );
	LOADBASSFUNCTION( BASS_StreamCreateFile );
	LOADBASSFUNCTION( BASS_StreamCreateURL );
	//LOADBASSFUNCTION( BASS_StreamGetLength );
	LOADBASSFUNCTION( BASS_StreamGetFilePosition );
	LOADBASSFUNCTION( BASS_ChannelPlay );
	LOADBASSFUNCTION( BASS_ChannelBytes2Seconds );
	LOADBASSFUNCTION( BASS_ChannelIsActive );
//	LOADBASSFUNCTION( BASS_ChannelSlideAttributes );
	LOADBASSFUNCTION( BASS_ChannelIsSliding );
	LOADBASSFUNCTION( BASS_ChannelGetPosition );
	LOADBASSFUNCTION( BASS_ChannelGetLevel );
	LOADBASSFUNCTION( BASS_ChannelSetSync );
	LOADBASSFUNCTION( BASS_GetVersion );
	LOADBASSFUNCTION( BASS_ErrorGetCode );
	LOADBASSFUNCTION( BASS_MusicLoad );
//	LOADBASSFUNCTION( BASS_MusicGetAttribute );
	//LOADBASSFUNCTION( BASS_MusicGetLength );
	LOADBASSFUNCTION( BASS_ChannelBytes2Seconds );
	LOADBASSFUNCTION( BASS_ChannelSetSync );
	LOADBASSFUNCTION( BASS_ChannelPlay );
//	LOADBASSFUNCTION( BASS_ChannelSetAttributes );
	/* Gone in ver 2.4
	LOADBASSFUNCTION( BASS_GetDeviceDescription );
	*/
	LOADBASSFUNCTION( BASS_StreamFree );
	LOADBASSFUNCTION( BASS_ChannelIsActive );
	LOADBASSFUNCTION( BASS_ChannelGetLevel );
	LOADBASSFUNCTION( BASS_ChannelGetPosition );
	LOADBASSFUNCTION( BASS_ChannelGetLength );

	LOADBASSFUNCTION( BASS_ChannelGetData );
	LOADBASSFUNCTION( BASS_ChannelGetTags );
	
	// ver 2.4
	LOADBASSFUNCTION( BASS_SetVolume ); // To set global volume.
	LOADBASSFUNCTION( BASS_ChannelSetAttribute ); // Lets me set chanel volume separetly.

	return ( qtrue );
}

static qboolean BASS_DLL_READY = qfalse;
qboolean	engine_initialized = qfalse;

/* */
qboolean
CheckBASSLoaded ( void )
{
	if ( BASS_DLL_READY == qfalse )
	{
		BASS_DLL_READY = LoadBASS();
	}

	return ( BASS_DLL_READY );
}


/* free the BASS library from memory and delete the temporary file */
void
FreeBASS ( void )
{
	if ( !bass )
	{
		return;
	}

	FreeLibrary( bass );
	//bass = 0;
	DeleteFile( tempfile );

	engine_initialized = qfalse;
	BASS_DLL_READY = qfalse;
}

typedef struct soundengine_MyMusic_s	// Sounds Table Structure
{
	char	musicFile[MAX_SOUNDPATH];		// Sound name.
	char	station_name[MAX_SOUNDPATH];	// Radio Station Name
} soundengine_MyMusic_t;				// MyMusic Table Typedef
soundengine_MyMusic_t	MyMusic[MAX_SOUNDPATH];
int						MyMusicTotal = 0;
qboolean				MyMusicLoaded = qfalse;
int						MyMusicSelection = 0;

// display error messages
static void
Error ( char *text )
{
#ifndef CGAME
	Com_Printf( "Error(%d): %s\n", BASS_ErrorGetCode(), text );
#else
	CG_Printf( "Error(%d): %s\n", BASS_ErrorGetCode(), text );
#endif
	BASS_Free();

	//ExitProcess(0);
}

static DWORD	starttime;


// looping synchronizer, resets the clock
static void CALLBACK
LoopSync ( HSYNC handle, DWORD channel, DWORD data, DWORD user )
{
	starttime = timeGetTime();
}

static DWORD	chan, act, music_time, level;
static BOOL		ismod;
static QWORD	pos;
static QWORD	length;
static int		a;
#ifndef CGAME


/* */
/*
void QDECL
CG_Error ( const char *msg, ... )
{
	Com_Printf( "%s\n", msg );
}
*/

#else
extern void QDECL	CG_Error ( const char *msg, ... );
#endif
extern void			trap_Cvar_Set ( const char *var_name, const char *value );


// update stream title from metadata
void
DoMeta ( char *meta )
{
	/*char	*p;

	CG_Printf("Meta: %s.\n", meta);
	if ( meta && (p = strstr( meta, "StreamTitle='")) )
	{
		char	name[512];
		p = strdup( p + 13 );
		strchr( p, ';' )[-1] = 0;
		strcpy( name, p );
		//CG_Printf("Name: %s.\n", name);

		if (name && name[0])
			trap_Cvar_Set( "s_radioStationInfo3", va( "%s", name) );
		else
			trap_Cvar_Set( "s_radioStationInfo3", va("Untitled") );

		free( p );
	}*/
}


/* */
void CALLBACK
MetaSync ( HSYNC handle, DWORD channel, DWORD data, DWORD user )
{
	DoMeta( (char *) data );
}


/* */
void CALLBACK
StatusProc ( void *buffer, DWORD length2, DWORD user )
{
	//	if (buffer && !length)
	//		MESS(32,WM_SETTEXT,0,buffer); // display connection status
} 

char			radio_station[255];
DWORD			cthread = 0;
qboolean		Playing_Radio = qfalse;
qboolean		Already_Playing = qfalse;
extern vmCvar_t s_radioStation;
extern vmCvar_t s_radioStationOverride;
extern vmCvar_t s_radioStationInfo1;
extern vmCvar_t s_radioStationInfo2;
extern vmCvar_t s_radioStationInfo3;
extern vmCvar_t s_inetRadio;
extern vmCvar_t s_radioVolume;
#pragma warning( disable : 4028 )
#pragma warning( disable : 4024 )


/* */
void __cdecl
OpenURL ( DWORD index )
{
	BASS_StreamFree( chan );	// close old stream
	if ( !(chan = BASS_StreamCreateURL( radio_station, 0, /*BASS_STREAM_META |*/ BASS_STREAM_STATUS, StatusProc, 0)) )
	{

		//		Error("Can't play the stream");
	}
	else
	{
		while ( 1 )
		{						// pre-buffer...
			//char text[20];
			DWORD	progress, len = BASS_StreamGetFilePosition( chan, BASS_FILEPOS_END );
			if ( len == -1 )
			{
				goto done;		// something's gone wrong! (eg. BASS_Free called)
			}

			progress =
				(
					BASS_StreamGetFilePosition( chan, BASS_FILEPOS_DOWNLOAD) -
					BASS_StreamGetFilePosition( chan, BASS_FILEPOS_DECODE)
				) *
				100 /
				len;			// percentage of buffer filled

			//CG_Printf("ICY: %s\n", BASS_ChannelGetTags(chan,BASS_TAG_ICY));

						{
			//CG_Printf(va("ICY: %s\n", BASS_ChannelGetTags(chan, BASS_TAG_ICY)));
			/*char *icy=BASS_ChannelGetTags(chan, BASS_TAG_ICY);
			if (icy)
			{
				CG_Printf("ICY: %s\n", icy);

				for (;*icy;icy+=strlen(icy)+1) {
					if (!memcmp(icy,"icy-name:",9))
						//MESS(31,WM_SETTEXT,0,icy+9);
						trap_Cvar_Set( "s_radioStationInfo1", va( "%s", icy+9 ) );
					if (!memcmp(icy,"icy-br:",7)) {
						char br[30]="bitrate: ";
						//strcat(br,icy+7);
						//MESS(32,WM_SETTEXT,0,br);
						trap_Cvar_Set( "s_radioStationInfo2", va( "%s", icy+7 ) );
					}
				}
			}*/

			}
			if ( progress > 75 )
			{
				break;			// over 75% full, enough
			}

			//sprintf(text,"buffering... %d%%",progress);
			//			MESS(31,WM_SETTEXT,0,text);

			// EQ //
			//BASS_ChannelGetData(chan,fft,BASS_DATA_FFT2048);

			Sleep( 50 );
		}

		// get the stream title and set sync for subsequent titles
		//DoMeta(BASS_ChannelGetTags(chan,BASS_TAG_META));
		BASS_ChannelSetSync( chan, BASS_SYNC_META, 0, &MetaSync, 0 );

		// play it!
		BASS_ChannelPlay( chan, FALSE );
	}

done:
	cthread = 0;
	Already_Playing = qfalse;

	/* _endthread given to terminate */
	_endthread();
}


/* */
void
CloseURL ( void )
{
	cthread = 0;
	Already_Playing = qfalse;
}

void		CG_StartMusicTrackFile ( int argc, char *music, qboolean loop );


/* */
void
CG_StartMusicTrack ( int argc, char *music, qboolean loop )
{
	if ( Playing_Radio )
	{
		int i = 0, track = 0;
		qboolean found = qfalse;

		strcpy( radio_station, music );

		for (i = 0; i < MyMusicTotal; i++)
		{
			if (!strcmp(MyMusic[i].musicFile, music))
			{
				track = i;
				found = qtrue;
				break;
			}
		}

		if (found)
			trap_Cvar_Set( "s_radioStationInfo1", va( "%s", MyMusic[track].station_name) );
		else
			trap_Cvar_Set( "s_radioStationInfo1", va( "Unknown Station") );

		trap_Cvar_Set( "s_radioStationInfo2", va( "%s", music) );

		// open URL in a new thread (so that main thread is free)
		cthread = _beginthread( OpenURL, 0, LOWORD( 0) );
	}
	else
	{
		CG_StartMusicTrackFile( argc, music, loop );
	}
}


/* */
void
CG_StartMusicTrackFile ( int argc, char *music, qboolean loop)
{
	if ( !CheckBASSLoaded() )
	{
		return;
	}

	// check that BASS 2.2 was loaded
/*	if ( BASS_GetVersion() != MAKELONG( 2, 2) )
	{
#ifndef CGAME
		Com_Printf( "BASS version 2.2 was not loaded\n" );
#else
		CG_Printf( "BASS version 2.2 was not loaded\n" );
#endif
		return;
	}*/

	if ( argc != 2 )
	{

		//CG_Printf("\tusage: contest <file>\n");
		return;
	}

	//BASS_SetConfig(BASS_CONFIG_NET_PREBUF,0); // minimize automatic pre-buffering, so we can do it (and display it)
	// try streaming the file/url
	if
	(
	(chan = BASS_StreamCreateFile( FALSE, music, 0, 0, loop ? BASS_SAMPLE_LOOP : 0 )) ||
		(chan = BASS_StreamCreateURL( music, 0, loop ? /*BASS_STREAM_META |*/ BASS_STREAM_STATUS | BASS_SAMPLE_LOOP : /*BASS_STREAM_META |*/ BASS_STREAM_STATUS , NULL, 0))
	)
	{
		pos = BASS_ChannelGetLength( chan, BASS_POS_BYTE );
		if ( BASS_StreamGetFilePosition( chan, BASS_FILEPOS_DOWNLOAD) != -1 )
		{

			// streaming from the internet

			/*if (pos)
				CG_Printf("streaming internet file [%I64d bytes]",pos);
			else
				CG_Printf("streaming internet file");*/
			while ( 1 )
			{												// pre-buffer...
				DWORD	progress, len = BASS_StreamGetFilePosition( chan, BASS_FILEPOS_END );
				if ( len == -1 )
				{
					Com_Error(0, "Can't buffer!\n" );			//return;//goto done; // something's gone wrong! (eg. BASS_Free called)
				}

				progress =
					(
						BASS_StreamGetFilePosition( chan, BASS_FILEPOS_DOWNLOAD) -
						BASS_StreamGetFilePosition( chan, BASS_FILEPOS_DECODE)
					) *
					100 /
					len;									// percentage of buffer filled

				// EQ //
				//BASS_ChannelGetData(chan,fft,BASS_DATA_FFT2048);

				Sleep( 50 );
			}
			{												// get the broadcast name and bitrate
				//char *icy=BASS_ChannelGetTags(chan,BASS_TAG_ICY);
				// get the stream title and set sync for subsequent titles
				BASS_ChannelGetTags( chan, BASS_TAG_META );
				BASS_ChannelSetSync( chan, BASS_SYNC_META, 0, &MetaSync, 0 );

				//CG_Printf("META: %s\n", BASS_ChannelGetTags(chan,BASS_TAG_META));
				//CG_Printf("ICY: %s\n", BASS_ChannelGetTags(chan,BASS_TAG_ICY));

				// play it!
				BASS_ChannelPlay( chan, FALSE );
			}
		}

		//else
		//CG_Printf("streaming file [%I64d bytes]",pos);
		ismod = FALSE;
	}
	/*else
	{

		// try loading the MOD (with looping, sensitive ramping, and calculate the duration)
		if
		(
			!(
				chan = BASS_MusicLoad( FALSE, music, 0, 0, loop ? BASS_MUSIC_RAMPS | BASS_MUSIC_CALCLEN | BASS_SAMPLE_LOOP :  BASS_MUSIC_RAMPS | BASS_MUSIC_CALCLEN, 0)
			)
		)
		{													// not a MOD either
			Error( "Can't play the file" );
		}

		// count channels
		for ( a = 0; BASS_MusicGetAttribute( chan, BASS_MUSIC_ATTRIB_VOL_CHAN + a) != -1; a++ );

		//CG_Printf("playing MOD music \"%s\" [%d chans, %d orders]",BASS_MusicGetName(chan),a,BASS_MusicGetLength(chan,FALSE));
		pos = BASS_ChannelGetLength( chan, BASS_POS_BYTE );
		ismod = TRUE;
	}*/

	// display the music_time length
	if ( pos )
	{
		music_time = ( DWORD ) BASS_ChannelBytes2Seconds( chan, pos );

		//CG_Printf(" %d:%02d\n",music_time/60,music_time%60);
	}

	//else // no music_time length available
	//CG_Printf("\n");
	// set a synchronizer for when it reaches the end, and start playback
	BASS_ChannelSetSync( chan, BASS_SYNC_END, 0, &LoopSync, 0 );
	BASS_ChannelPlay( chan, FALSE );
	starttime = timeGetTime();
	{
		float volume = CG_GetMusicVolume();
		//BASS_ChannelSetAttributes( chan, -1 /*leave crrent*/, volume, -101 /*leave crrent*/ );
		BASS_ChannelSetAttribute(chan, BASS_ATTRIB_VOL, volume);
	}
}

qboolean
CG_CheckMyMusic ( void ){
	fileHandle_t	f;
	char	*loadPath;
	int				len;

	trap_Cvar_Register( &fs_game, "fs_game", "", CVAR_SERVERINFO | CVAR_ROM );
	loadPath = va( "%s/MyMusic/MyMusic.list", fs_game.string );
	len = trap_FS_FOpenFile( loadPath, &f, FS_READ );

	if ( !f )
	{
		return qfalse;
	}

	// empty file?
	if ( len <= 0 )
	{
		return qfalse;
	}

	trap_FS_FCloseFile( f );

	return qtrue;
}

/* */
void
CG_LoadMyMusic ( void )
{				// Load music playlist from external file.
	char	*s, *t;
	int		len = 0;
	FILE	*f;
	char	*buf, *buf2;
	char	*loadPath;
	int		num = 0;
	trap_Cvar_Register( &fs_game, "fs_game", "", CVAR_SERVERINFO | CVAR_ROM );
#ifdef __FULL_VERSION__
	loadPath = va( "etmain/MyMusic/MyMusic.list" );
#else //!__FULL_VERSION__
	loadPath = va( "%s/MyMusic/MyMusic.list", fs_game.string );
#endif //__FULL_VERSION__
	f = fopen( va( "%s", loadPath), "r" );
	if ( !f )
	{
		return;
	}

	buf2 = (char *) malloc( 1 + 1/*, "CG_LoadMyMusic"*/ );

	// Calculate the length of this data file...
	while ( fread( buf2, 1, 1, f) )
	{
		len++;
	}

	fclose( f );
	if ( !len )
	{			//empty file
		free( buf2 );
		return;
	}

	// OK, we have a length value now... Re-Open the file and read into the main buffer...
	f = fopen( va( "%s", loadPath), "r" );
	if ( (buf = (char *) malloc( len + 1/*, "CG_LoadMyMusic"*/)) == 0 )
	{			//alloc memory for buffer
		free( buf2 );
		free( buf );
		return;
	}

	fread( buf, len, 1, f );
	fclose( f );
#ifndef CGAME
	Com_Printf( "^4*** ^3%s^4: ^5Loading MyMusic playlist from file ^7%s^5.\n", GAME_VERSION, loadPath );
#else
	CG_Printf( "^4*** ^3%s^4: ^5Loading MyMusic playlist from file ^7%s^5.\n", GAME_VERSION, loadPath );
#endif
	for ( t = s = buf; *t; /* */ )
	{
		num++;
		s = strchr( s, '\n' );
		if ( !s || num > len )
		{
			break;
		}

		while ( *s == '\n' )
		{
			*s++ = 0;
		}

		if ( *t )
		{
			if ( !Q_strncmp( "//", va( "%s", t), 2) == 0 && strlen( va( "%s", t)) > 0 )
			{	// Not a comment either... Record it in our list...
#ifdef __FULL_VERSION__
				strcpy( MyMusic[MyMusicTotal].musicFile, va( "etmain/MyMusic/%s", t) );
#else //!__FULL_VERSION__
				strcpy( MyMusic[MyMusicTotal].musicFile, va( "%s/MyMusic/%s", fs_game.string, t) );
#endif //__FULL_VERSION__
				MyMusicTotal++;
			}
		}

		t = s;
	}

	MyMusicLoaded = qtrue;
#ifndef CGAME
	Com_Printf( "^4*** ^3%s^4: ^5Loaded ^7%i^5 MyMusic playlist tracks.\n", GAME_VERSION, MyMusicTotal );
#else
	CG_Printf( "^4*** ^3%s^4: ^5Loaded ^7%i^5 MyMusic playlist tracks.\n", GAME_VERSION, MyMusicTotal );
#endif

	// Remove the extra MyMusicTotal++ from above...
	MyMusicTotal--;
	free( buf );
	free( buf2 );
}

#define MAX_STATION_TEXT	8192


/* */
#ifdef __OLD_INTERNET_RADIO_FILE__

void
CG_LoadInternetMusic ( void )
{							// Load internet music sites from external file.
	char			*s, *t;
	int				len = 0;
	fileHandle_t	f = 0;
	char			*buf;	//, *buf2;

	//char			buf[MAX_STATION_TEXT];
	char			*loadPath;
	int				num = 0;
	trap_Cvar_Register( &fs_game, "fs_game", "", CVAR_SERVERINFO | CVAR_ROM );
	loadPath = va( "MyMusic/internet_radio.list\0", fs_game.string );
	len = trap_FS_FOpenFile( va( "%s", loadPath), &f, FS_READ );
	if ( !f || !len )
	{
		trap_FS_FCloseFile( f );
		return;
	}

	buf = (char *) B_Alloc( 1 + 1/*, "CG_LoadInternetMusic"*/ );

	//trap_FS_Read(&buf, len, f);
	trap_FS_Read( buf, len + 1, f );
	buf[len] = 0;

	// done
	trap_FS_FCloseFile( f );
#ifndef CGAME
	Com_Printf( "^4*** ^3%s^4: ^5Loading internet music site list from file ^7%s^5.\n", GAME_VERSION, loadPath );
#else
	CG_Printf( "^4*** ^3%s^4: ^5Loading internet music site list from file ^7%s^5.\n", GAME_VERSION, loadPath );
#endif
	for ( t = s = buf; *t; /* */ )
	{
		num++;
		s = strchr( s, '\n' );
		if ( !s || num > len )
		{
			break;
		}

		while ( *s == '\n' )
		{
			*s++ = 0;
		}

		if ( *t )
		{
			if ( !Q_strncmp( "//", va( "%s", t), 2) == 0 && strlen( va( "%s", t)) > 0 )
			{				// Not a comment either... Record it in our list...
				//strcpy(MyMusic[MyMusicTotal].musicFile, va("%s", t));
				//Q_strncpyz( MyMusic[MyMusicTotal].musicFile, va( "%s", t), strlen( va( "%s", t)) );
				strcpy( MyMusic[MyMusicTotal].musicFile, va( "%s", t) );
				if ( strlen( va( "%s", MyMusic[MyMusicTotal].musicFile)) > 0 )
				{

					//CG_Printf("Loaded: \"%s\"\n", MyMusic[MyMusicTotal].musicFile);
					MyMusicTotal++;
				}
			}
		}

		t = s;
	}

	MyMusicLoaded = qtrue;
#ifndef CGAME
	Com_Printf( "^4*** ^3%s^4: ^5Loaded ^7%i^5 internet music sites.\n", GAME_VERSION, MyMusicTotal );
#else
	CG_Printf( "^4*** ^3%s^4: ^5Loaded ^7%i^5 internet music sites.\n", GAME_VERSION, MyMusicTotal );
#endif

	// Remove the extra MyMusicTotal++ from above...
	MyMusicTotal--;
}

#else //!__OLD_INTERNET_RADIO_FILE__

static qboolean RADIO_LoadData_ParseError( int handle, char *format, ... )
{
	int line;
	char filename[128];
	va_list argptr;
	static char string[4096];

	va_start( argptr, format );
	Q_vsnprintf( string, sizeof(string), format, argptr );
	va_end( argptr );

	filename[0] = '\0';
	line = 0;
	trap_PC_SourceFileAndLine( handle, filename, &line );

	if (filename[0])
		Com_Printf( S_COLOR_RED "RADIO_LoadData: ERROR - %s, line %d: %s\n", filename, line, string );
	else
		Com_Printf( S_COLOR_RED "RADIO_LoadData: ERROR - %s\n", string );

	trap_PC_FreeSource( handle );

	return qfalse;
}

qboolean RADIO_String_Parse(int handle, char *out, size_t size) 
{
	pc_token_t token;

	if( !trap_PC_ReadToken(handle, &token) )
		return qfalse;

	//Q_strncpyz( out, token.string, size );
	strcpy( out, token.string );
    return qtrue;
}

qboolean RADIO_LoadData(const char* szFileName)
{
	pc_token_t	token;
	int			handle;
	char		string[MAX_TOKEN_CHARS];

#ifndef CGAME
	Com_Printf( "^4*** ^3%s^4: ^5Loading internet music site list from file ^7%s^5.\n", GAME_VERSION, va("MyMusic/%s.list", szFileName) );
#else
	CG_Printf( "^4*** ^3%s^4: ^5Loading internet music site list from file ^7%s^5.\n", GAME_VERSION, va("MyMusic/%s.list", szFileName) );
#endif

	handle = trap_PC_LoadSource( va("MyMusic/%s.list", szFileName) );

	if( !handle )
		return qfalse;

	while (1)
	{
		if( !trap_PC_ReadToken( handle, &token ) || Q_stricmp( token.string, "{" ) ) 
		{
			break;
		}

		while (1)
		{
			if( !trap_PC_ReadToken( handle, &token )) 
			{
				return RADIO_LoadData_ParseError( handle, "unexpected eof" );
			}

			if( !Q_stricmp( token.string, "}" ) ) 
			{
				break;
			}
			else if( !Q_stricmp( token.string, "address" ) ) 
			{
				if( !RADIO_String_Parse( handle, string, MAX_TOKEN_CHARS)) 
				{
					return RADIO_LoadData_ParseError( handle, "expected address" );
				}
				else
				{
					//Q_strncpyz( MyMusic[MyMusicTotal].musicFile, va( "%s", string), strlen( va( "%s", string))+1 );
					strcpy( MyMusic[MyMusicTotal].musicFile, va( "%s", string) );
				}
			}
			else if( !Q_stricmp( token.string, "description" ) ) 
			{
				if( !RADIO_String_Parse( handle, string, MAX_TOKEN_CHARS)) 
				{
					return RADIO_LoadData_ParseError( handle, "expected description" );
				}
				else
				{
					//Q_strncpyz( MyMusic[MyMusicTotal].station_name, va( "%s", string), strlen( va( "%s", string))+1 );
					strcpy( MyMusic[MyMusicTotal].station_name, va( "%s", string) );
#ifndef CGAME
					Com_Printf("^4*** ^3%s^4: ^5Loaded Radio Station ^7%s^5 (^7%s^5).\n", GAME_VERSION, MyMusic[MyMusicTotal].station_name, MyMusic[MyMusicTotal].musicFile);
#else
					CG_Printf("^4*** ^3%s^4: ^5Loaded Radio Station ^7%s^5 (^7%s^5).\n", GAME_VERSION, MyMusic[MyMusicTotal].station_name, MyMusic[MyMusicTotal].musicFile);
#endif
					MyMusicTotal++;
				}
			}
			else 
			{
				return RADIO_LoadData_ParseError( handle, "unknown token '%s'", token.string );
			}
		}
	}

	trap_PC_FreeSource( handle );

	MyMusicLoaded = qtrue;
#ifndef CGAME
	Com_Printf( "^4*** ^3%s^4: ^5Loaded ^7%i^5 internet music sites.\n", GAME_VERSION, MyMusicTotal );
#else
	CG_Printf( "^4*** ^3%s^4: ^5Loaded ^7%i^5 internet music sites.\n", GAME_VERSION, MyMusicTotal );
#endif

	// Remove the extra MyMusicTotal++ from above...
	MyMusicTotal--;

	return qtrue;
}

void
CG_LoadInternetMusic ( void )
{
	RADIO_LoadData("internet_radio");
}

const char *RADIO_GetStationName(int index)
{
	return MyMusic[index].station_name;
}

const char *RADIO_GetStationAddress(int index)
{
	return MyMusic[index].musicFile;
}

#endif //__OLD_INTERNET_RADIO_FILE__

char MyMusic_Current[MAX_TOKEN_CHARS];
/* */
void
CG_MyMusicPlay ( void )
{
	int selection = 0;
	qboolean change_station = qfalse;

	if ( MyMusicTotal > 0 )
	{
		selection = Q_irand(0, MyMusicTotal-1);//rand() % MyMusicTotal;	//Q_irand(0, MyMusicTotal);
	}

	if ( CG_GetMusicVolume() <= 0.01 )
	{
		if ( cthread )
		{
			CloseURL();
		}

		if ( chan )
		{
			BASS_StreamFree( chan );
		}

		return;
	}

	if (Q_stricmp( s_radioStationOverride.string, ""))
	{
		if (Q_stricmp( va("%s", s_radioStationOverride.string), va("%s", MyMusic_Current)))
		{// Has changed! Need to switch to a new station!
			change_station = qtrue;

			//CG_Printf("%s does not equal %s!\n", va("%s", s_radioStationOverride.string), va("%s", MyMusic_Current));	// LOL WAT --eez

			if ( cthread )
			{
				CloseURL();
			}

			if ( chan )
			{
				BASS_StreamFree( chan );
			}
		}
	}

	if ( Already_Playing && !change_station/*&& BASS_ChannelIsActive(chan)*/ )
	{
//#ifdef CGAME
//		CG_Printf( "MusicEngine Debug: Already Playing!\n" );
//#endif
		return;
	}

	if (Q_stricmp( s_radioStationOverride.string, ""))
	{
		//Q_strncpyz( MyMusic_Current, va( "%s", s_radioStationOverride.string), strlen( va( "%s", s_radioStationOverride.string))+1 );
		strcpy( MyMusic_Current, va( "%s", s_radioStationOverride.string) );
	}

	while ( selection > 0 && !MyMusic[MyMusicTotal].musicFile )
	{
		selection = Q_irand(0, MyMusicTotal-1);//rand() % MyMusicTotal;	//Q_irand(0, MyMusicTotal);
	}

	if ( Playing_Radio )
	{
		if ( !Q_stricmp( s_radioStation.string, "") && !Q_stricmp( s_radioStationOverride.string, "") )
		{
#ifndef CGAME
			Com_Printf( "^4*** ^3%s^4: ^5Playing internet radio: ^7%s^5.\n", GAME_VERSION, MyMusic[selection].musicFile );
#else
			CG_Printf( "^4*** ^3%s^4: ^5Playing internet radio: ^7%s^5.\n", GAME_VERSION, MyMusic[selection].musicFile );
#endif
		}
		else
		{
			if ( Q_stricmp( s_radioStationOverride.string, "") )
			{
#ifndef CGAME
				Com_Printf( "^4*** ^3%s^4: ^5Playing internet radio: ^7%s^5.\n", GAME_VERSION,
							s_radioStationOverride.string );
#else
				CG_Printf( "^4*** ^3%s^4: ^5Playing internet radio: ^7%s^5.\n", GAME_VERSION,
						   s_radioStationOverride.string );
#endif
			}
			else
			{
#ifndef CGAME
				Com_Printf( "^4*** ^3%s^4: ^5Playing internet radio: ^7%s^5.\n", GAME_VERSION, s_radioStation.string );
#else
				CG_Printf( "^4*** ^3%s^4: ^5Playing internet radio: ^7%s^5.\n", GAME_VERSION, s_radioStation.string );
#endif
			}
		}
	}
	else
	{
#ifndef CGAME
		Com_Printf( "^4*** ^3%s^4: ^5Playing MyMusic playlist file ^7%s^5.\n", GAME_VERSION,
					MyMusic[selection].musicFile );
#else
		CG_Printf( "^4*** ^3%s^4: ^5Playing MyMusic playlist file ^7%s^5.\n", GAME_VERSION, MyMusic[selection].musicFile );
#endif
	}

	if
	(
		!Playing_Radio ||
		(Playing_Radio && !Q_stricmp( s_radioStation.string, "") && !Q_stricmp( s_radioStationOverride.string, ""))
	)
	{
		CG_StartMusicTrack( 2, MyMusic[selection].musicFile, qfalse );
	}
	else
	{
		if ( Q_stricmp( s_radioStationOverride.string, "") )
		{
			CG_StartMusicTrack( 2, s_radioStationOverride.string, qfalse );
		}
		else
		{
			CG_StartMusicTrack( 2, s_radioStation.string, qfalse );
		}
	}

	length = BASS_ChannelGetLength( chan, BASS_POS_BYTE );
	if ( Playing_Radio )
	{
		Already_Playing = qtrue;
	}

//#ifdef CGAME
//	CG_Printf( "MusicEngine Debug: CG_MyMusicPlay() completed!\n" );
//#endif
}

void
UI_MusicPlay ( void )
{
	if ( CG_GetMusicVolume() <= 0.01 )
	{
		if ( cthread )
		{
			CloseURL();
		}

		if ( chan )
		{
			BASS_StreamFree( chan );
		}

		return;
	}

	if ( Already_Playing )
	{
		return;
	}

	Com_Printf( "^4*** ^3%s^4: ^5Playing MyMusic playlist file ^7%s^5.\n", GAME_VERSION, "sound/music/eastfront_main.mp3" );

	CG_StartMusicTrack( 2, "sound/music/eastfront_main.mp3", qfalse );

	length = BASS_ChannelGetLength( chan, BASS_POS_BYTE );
	
	Already_Playing = qtrue;
}

/* */
void
CG_SoundEngineInfo ( void )
{
	int i;
	for ( i = 0; i < 1024; i++ )
	{
/*
		if ( BASS_GetDeviceDescription( i) )
#ifndef CGAME
			Com_Printf( "Found sound device %i (%s).\n", i, BASS_GetDeviceDescription( i) );
#else
		CG_Printf( "Found sound device %i (%s).\n", i, BASS_GetDeviceDescription( i) );
#endif
*/
	}
}

void UpdateSpectrum( void );

/* */
void
CG_DoMusic ( void )
{
	qboolean change_station = qfalse;

	//Com_Printf("DEBUG: DoMusic!\n");

	if ( !CheckBASSLoaded() )
	{
		//Com_Printf("DEBUG: BASS not loaded!\n");
		return;
	}

/*	if ( BASS_GetVersion() != MAKELONG( 2, 2) )
	{
#ifndef CGAME
		Com_Printf( "BASS version 2.2 was not loaded\n" );
#else
		CG_Printf( "BASS version 2.2 was not loaded\n" );
#endif
		return;
	}*/

	// setup output - default device
	if ( !engine_initialized )
	{
		if ( cthread )
		{
			CloseURL();
		}

		if ( chan )
		{
			BASS_StreamFree( chan );
		}

		BASS_Free();
		CG_SoundEngineInfo();
		if ( !BASS_Init( 1, 44100, 0, 0, NULL) )
		{
			//Error( "MUSIC ENGINE: Can't initialize autio device." );
			return;
		}
		else
		{
			engine_initialized = qtrue;
		}
	}

#ifdef CGAME
	if ( !MyMusicLoaded 
#ifdef __INETRADIO__
		&& (s_inetRadio.integer || CG_CheckMyMusic())
#else //!__INETRADIO__
		&& CG_CheckMyMusic()
#endif //__INETRADIO__
		)
	{
		BASS_StreamFree( chan );
		if ( cthread )
		{
			CloseURL();
		}

		CG_LoadMyMusic();

#ifdef __INETRADIO__
		if ( MyMusicTotal <= 0 
			&& s_inetRadio.integer  )
		{
			CG_LoadInternetMusic();
			if
			(
				MyMusicTotal > 0 ||
				Q_stricmp( s_radioStation.string, "") ||
				Q_stricmp( s_radioStationOverride.string, "")
			)
			{
				Playing_Radio = qtrue;
				MyMusicLoaded = qtrue;
			}
		}
#endif //__INETRADIO__
	}
#endif //CGAME

	if ( CG_GetMusicVolume() <= 0.01  )
	{
		if ( cthread )
		{
			CloseURL();
		}

		if ( chan )
		{
			BASS_StreamFree( chan );
		}

		return;
	}

	if ( chan )
	{
		float volume = CG_GetMusicVolume();
		//BASS_ChannelSetAttributes( chan, -1 /*leave crrent*/, volume, -101 /*leave crrent*/ );
		BASS_ChannelSetAttribute(chan, BASS_ATTRIB_VOL, volume);
		pos = BASS_ChannelGetPosition( chan, BASS_POS_BYTE );
	}

	if (Playing_Radio && (Q_stricmp( s_radioStationOverride.string, "") || (!Q_stricmp( s_radioStationOverride.string, "") && Q_stricmp( MyMusic_Current, ""))))
	{
		if (Q_stricmp( s_radioStationOverride.string, MyMusic_Current))
		{// Has changed! Need to switch to a new station!
			change_station = qtrue;

			if ( cthread )
			{
				CloseURL();
			}

			//if ( chan )
			{
				BASS_StreamFree( chan );
			}

			BASS_Free();

			if ( !BASS_Init( 1, 44100, 0, 0, NULL) )
			{
				//Error( va( "MUSIC ENGINE: Can't initialize device.") );
				return;
			}
		}
	}

	if (Playing_Radio/*Q_stricmp( s_radioStationOverride.string, "")*/)
	{
		//Q_strncpyz( MyMusic_Current, va( "%s", s_radioStationOverride.string), strlen( va( "%s", s_radioStationOverride.string))+1 );
		strcpy( MyMusic_Current, va( "%s", s_radioStationOverride.string) );
	}

#ifdef CGAME
	if ( MyMusicTotal > 0 
#ifdef __INETRADIO__
		|| (Q_stricmp( s_radioStation.string, "") || Q_stricmp( s_radioStationOverride.string, "") && s_inetRadio.integer) 
#endif //__INETRADIO__
		)
	{	// MyMusic overrides the game music, if the player has some installed...
		if (/*!CG_CheckMyMusic()*/MyMusicTotal <= 0
#ifdef __INETRADIO__
			&& (!Playing_Radio && s_inetRadio.integer)
#endif //__INETRADIO__
			)
		{
		} 
		else if (!Q_stricmp( s_radioStation.string, "") && !Q_stricmp( s_radioStationOverride.string, "") && Playing_Radio)
		{
			if ( cthread )
			{
				CloseURL();
			}

			//if ( chan )
			{
				BASS_StreamFree( chan );
			}
		}
		else if ( !chan || pos >= length )
		{
			//chan = 0;
			BASS_StreamFree( chan );
			CG_MyMusicPlay();
		}
		else if ( act = BASS_ChannelIsActive( chan) )
		{
			// display some stuff and wait a bit
			music_time = timeGetTime() - starttime;
			level = BASS_ChannelGetLevel( chan );
			pos = BASS_ChannelGetPosition( chan, BASS_POS_BYTE );
		}
		else
		{
			BASS_StreamFree( chan );
			CG_MyMusicPlay();
		}
	}
#endif //CGAME
//#if defined( __WARZONE__ ) || defined( __FULL_VERSION__ )
	else if ( act = BASS_ChannelIsActive( chan) )
	{
		// display some stuff and wait a bit
		music_time = timeGetTime() - starttime;
		level = BASS_ChannelGetLevel( chan );
		pos = BASS_ChannelGetPosition( chan, BASS_POS_BYTE );
	}
//#endif //defined(__WARZONE__) || defined(__FULL_VERSION__)
}

#ifdef CGAME
#include "cg_local.h"

extern	cgs_t			cgs;

#define RADIO_POSITION_X 574 //634
#define RADIO_POSITION_Y 220

#define SPECWIDTH 53	// display width
#define SPECHEIGHT 100 //127	// height (changing requires palette adjustments too)

#define SPECTRUM_BANDS SPECWIDTH
int spectrum_values[SPECTRUM_BANDS];

void UpdateSpectrum( void )
{// UQ1: For my funky new spectrum analyzer/radio on hud! :)
	int x = 0, y = 0;
	int b0=0;
	float pos_x = RADIO_POSITION_X + 6;
	float pos_y = RADIO_POSITION_Y + 4 + 8;

	BASS_ChannelGetData(chan,fft,BASS_DATA_FFT2048);
	
	for (x=0;x<SPECTRUM_BANDS;x++) {
		float sum=0;
		int sc, b1 = pow(2,x*10.0/(SPECTRUM_BANDS-1));

		if (b1 > 1023) b1  =1023;

		if (b1 <= b0) b1 = b0 + 1; // make sure it uses at least 1 FFT bin

		sc  =10+b1-b0;

		for (; b0 < b1; b0++) sum += fft[1+b0];

		y=(sqrt(sum/log10(sc))*1.7*SPECHEIGHT)-4; // scale it

		if (y > SPECHEIGHT) y = SPECHEIGHT; // cap it

		spectrum_values[x] = y;
	}

	for (x=0;x<SPECTRUM_BANDS;x++) {
		CG_VerticalPercentBarNoBorder( pos_x + x, pos_y, 1, 16/*24*//*16*//*24*/, (float)((float)spectrum_values[x] * 0.01) );
	}
}

#define		MAX_RADIO_TITLE_SCROLL_CHARACTERS 19
#define		MAX_RADIO_DESCRIPTION_SCROLL_CHARACTERS 30

int			current_title_text_scroll_position = 0;
qboolean	current_title_text_scroll_back = qfalse;
int			next_title_text_scroll_change = 0;

int			current_description_text_scroll_position = 0;
qboolean	current_description_text_scroll_back = qfalse;
int			next_description_text_scroll_change = 0;

void CG_RealDrawMusicInformation( char *music_title, char *music_description ) {
	int			w;
	vec4_t		tclr			=	{ 0.625f,	0.625f,	0.6f,	1.0f	};
	float		title_size = 0.22;//0.47f;
	float		description_size = 0.2;//0.37f;
	int			pos_x = RADIO_POSITION_X, pos_y = RADIO_POSITION_Y;
	char		music_title_short[MAX_RADIO_TITLE_SCROLL_CHARACTERS+1] = "";
	char		music_description_short[MAX_RADIO_DESCRIPTION_SCROLL_CHARACTERS+1] = "";

	// PLAYER PIC
	CG_DrawPic( RADIO_POSITION_X, RADIO_POSITION_Y, 64, 32, cgs.media.radio_player );

	// THE SPECTRUM ANALYZER :)
	UpdateSpectrum();

	Q_CleanStr( music_title );
	Q_CleanStr( music_description );

	//
	// Display/Scroll the Station Title Text...
	//

	if (strlen( music_title ) > MAX_RADIO_TITLE_SCROLL_CHARACTERS)
	{// Need to truncate the name for displaying...
		Q_strncpyz( music_title_short, music_title + current_title_text_scroll_position, MAX_RADIO_TITLE_SCROLL_CHARACTERS );

		if (current_title_text_scroll_back)
		{// Scroll backwards...
			if (next_title_text_scroll_change <= cg.time)
			{
				current_title_text_scroll_position--;

				if (current_title_text_scroll_position < 0)
				{
					current_title_text_scroll_position++;
					current_title_text_scroll_back = qfalse;
				}

				next_title_text_scroll_change = cg.time + 200;
			}
		}
		else
		{// Scroll forwards...
			if (next_title_text_scroll_change <= cg.time)
			{
				current_title_text_scroll_position++;

				if (current_title_text_scroll_position + (MAX_RADIO_TITLE_SCROLL_CHARACTERS-1) > strlen( music_title ))
				{
					current_title_text_scroll_position--;
					current_title_text_scroll_back = qtrue;
				}

				next_title_text_scroll_change = cg.time + 200;
			}
		}
	}
	else
	{// No scrolling, all characters of the name fit on the display...
		Q_strncpyz( music_title_short, music_title, MAX_RADIO_TITLE_SCROLL_CHARACTERS );
	}
	
	// STATION TITLE
	w = CG_Text_Width(music_title_short, title_size, FONT_SMALL3);
	pos_x += (32 - (w * 0.5f));

	CG_Text_Paint( pos_x-2, pos_y  + 8, title_size, tclr, music_title_short, 0, 0, 0, FONT_SMALL3 );

	//
	// Display/Scroll the Description Text...
	//

	if (strlen( music_description ) > MAX_RADIO_DESCRIPTION_SCROLL_CHARACTERS)
	{// Need to truncate the name for displaying...
		Q_strncpyz( music_description_short, music_description + current_description_text_scroll_position, MAX_RADIO_DESCRIPTION_SCROLL_CHARACTERS );

		if (current_description_text_scroll_back)
		{// Scroll backwards...
			if (next_description_text_scroll_change <= cg.time)
			{
				current_description_text_scroll_position--;

				if (current_description_text_scroll_position < 0)
				{
					current_description_text_scroll_position++;
					current_description_text_scroll_back = qfalse;
				}

				next_description_text_scroll_change = cg.time + 200;
			}
		}
		else
		{// Scroll forwards...
			if (next_description_text_scroll_change <= cg.time)
			{
				current_description_text_scroll_position++;

				if (current_description_text_scroll_position + (MAX_RADIO_DESCRIPTION_SCROLL_CHARACTERS-1) > strlen( music_description ))
				{
					current_description_text_scroll_position--;
					current_description_text_scroll_back = qtrue;
				}

				next_description_text_scroll_change = cg.time + 200;
			}
		}
	}
	else
	{// No scrolling, all characters of the name fit on the display...
		Q_strncpyz( music_description_short, music_description, MAX_RADIO_DESCRIPTION_SCROLL_CHARACTERS );
	}

	pos_x = RADIO_POSITION_X;

	// STATION TITLE
	w = CG_Text_Width(music_description_short, description_size, FONT_SMALL3);
	w *= ((description_size/title_size) * 2);
	pos_x += (32 - (w * 0.5f));

	CG_Text_Paint( pos_x-2, pos_y + 11 + 6, description_size, tclr, va("^7%s", music_description_short), 0, 0, 0, FONT_SMALL3 );
}

void CG_DrawMusicInformation( void ) 
{
	char music_title[128] = "";

	if ( CG_GetMusicVolume() <= 0.01 )
		return; // Music is switched off!

	if (Playing_Radio)
	{
		char *meta= (char *)BASS_ChannelGetTags(chan,BASS_TAG_META);
		if (meta) { // got Shoutcast metadata
			char *p=strstr(meta,"StreamTitle='");
			if (p) {
				p=strdup(p+13);
				strchr(p,';')[-1]=0;
				Q_strncpyz(music_title, p, strlen(p)+1);
				free(p);
			}
		} else {
			meta=BASS_ChannelGetTags(chan,BASS_TAG_OGG);
			if (meta) { // got Icecast/OGG tags
				const char *artist=NULL,*title=NULL,*p=meta;
				for (;*p;p+=strlen(p)+1) {
					if (!strnicmp(p,"artist=",7)) // found the artist
						artist=p+7;
					if (!strnicmp(p,"title=",6)) // found the title
						title=p+6;
				}
				if (artist) {
					char text[100];
					_snprintf(text,sizeof(text),"%s - %s",artist,title);
					Q_strncpyz(music_title, text, strlen(text)+1);
				} else if (title)
					Q_strncpyz(music_title, title, strlen(title)+1);
			}
		}

		CG_RealDrawMusicInformation( s_radioStationInfo1.string, music_title );
	}
	else if ( act = BASS_ChannelIsActive( chan) )
	{
		char artist[128] = "";
		TAG_ID3 *meta2 = (TAG_ID3*)BASS_ChannelGetTags(chan,BASS_TAG_ID3);

		if (!meta2)
		{
			CG_RealDrawMusicInformation( "Unknown Title", "Unknown Artist" );
			return;
		}
		else if (meta2->artist) {
			Q_strncpyz(artist, va("by %s.", meta2->artist), strlen(va("by %s.", meta2->artist))+1);
			Q_strncpyz(music_title, meta2->title, strlen(meta2->title)+1);
		} 
		else if (meta2->title)
		{
			Q_strncpyz(music_title, meta2->title, strlen(meta2->title)+1);
		}

		CG_RealDrawMusicInformation( music_title, artist );
	}
}
#endif //CGAME

/* */
void
UI_DoMusic ( void )
{
	if ( !CheckBASSLoaded() )
	{
		return;
	}

	// setup output - default device
	if ( !engine_initialized )
	{
		if ( cthread )
		{
			CloseURL();
		}

		if ( chan )
		{
			BASS_StreamFree( chan );
		}

		BASS_Free();
		CG_SoundEngineInfo();
		if ( !BASS_Init( 1, 44100, 0, 0, NULL) )
		{
			//Error( va( "Can't initialize device: %s.", BASS_GetDeviceDescription( 1)) );
			return;
		}
		else
		{
			engine_initialized = qtrue;
		}
	}

	if ( CG_GetMusicVolume() <= 0.01  )
	{
		if ( cthread )
		{
			CloseURL();
		}

		if ( chan )
		{
			BASS_StreamFree( chan );
		}

		return;
	}

	if ( chan )
	{
		float volume = CG_GetMusicVolume();
		//BASS_ChannelSetAttributes( chan, -1 /*leave crrent*/, volume, -101 /*leave crrent*/ );
		BASS_ChannelSetAttribute(chan, BASS_ATTRIB_VOL, volume);
		pos = BASS_ChannelGetPosition( chan, BASS_POS_BYTE );
	}

	if ( !chan || pos >= length )
	{
		//chan = 0;
		BASS_StreamFree( chan );
		UI_MusicPlay();
	}
	else if ( act = BASS_ChannelIsActive( chan) )
	{
		// display some stuff and wait a bit
		music_time = timeGetTime() - starttime;
		level = BASS_ChannelGetLevel( chan );
		pos = BASS_ChannelGetPosition( chan, BASS_POS_BYTE );
	}
	else
	{
		BASS_StreamFree( chan );
		UI_MusicPlay();
	}
}

/* */
void CG_StopMusic ( void )
{

	// wind the frequency down...
	//BASS_ChannelSlideAttributes(chan,1000,-1,-101,500);
	//Sleep(300);
	// ...and fade-out to avoid a "click"
	//BASS_ChannelSlideAttributes(chan,-1,-2,-101,200);
	//while (BASS_ChannelIsSliding(chan))
	//	Sleep(1);

	if (engine_initialized != qfalse && BASS_DLL_READY != qfalse)
	{
		if ( cthread )
		{
			CloseURL();
		}

		if ( chan )
		{
			BASS_StreamFree( chan );
			Sleep(500);
		}

		BASS_Free();
		FreeBASS();
	}

	FreeBASS();

	// HACK: 
	// stop DSound.dll from deadlocking by making it not unload at all!
	// reason we do this is because Bass lib is already unloading the
	// instance of DSound.dll for us.
	// verreh verreh hack indeed. will break when changing mods. --eez
	/*{
		DWORD i;
		for(i = 0x004533BF; i < 0x004533DE; i += 1)
		{
			(*(unsigned char *)i) = 0x90; // NOP
		}
	}*/
	// fuck, just ret i guess. ugly.
	(*(unsigned char *)0x004532F0) = 0xC3;


	//Com_Error(ERR_FATAL, "DEBUG: Music Engine is shut down.\n");
}

vmCvar_t	fs_game;
/* */
void
CG_SoundEngineStartMusicTrack ( char *sound, qboolean loop )
{
	char				filename[MAX_QPATH];

	if ( !CheckBASSLoaded() )
	{
		return;
	}

/*	if ( BASS_GetVersion() != MAKELONG( 2, 2) )
	{
#ifndef CGAME
		Com_Printf( "BASS version 2.2 was not loaded\n" );
#else
		CG_Printf( "BASS version 2.2 was not loaded\n" );
#endif
		return;
	}*/

	// FIXME: use engine version of Cvar_Get --eez
	trap_Cvar_Register( &fs_game, "fs_game", "", CVAR_SERVERINFO | CVAR_ROM );

	Q_strncpyz( filename, va("%s/", fs_game.string), MAX_QPATH );
	Q_strcat( filename, MAX_QPATH, va( "%s", sound) );



	// setup output - default device
	if ( !engine_initialized )
	{
		CG_SoundEngineInfo();
		if ( !BASS_Init( 1, 44100, 0, 0, NULL) )
		{
			//Error( va( "Can't initialize device: %s.", BASS_GetDeviceDescription( 1)) );
			return;
		}
		else
		{
			engine_initialized = qtrue;
		}
	}

	if ( MyMusicTotal > 0 )
	{
		return;
	}

//#if defined( __WARZONE__ ) || defined( __FULL_VERSION__ )
	if ( chan )
	{
		BASS_StreamFree( chan );
	}

	CG_StartMusicTrack( 2, va( "%s", filename), loop );
//#endif //defined(__WARZONE__) || defined(__FULL_VERSION__)
}

#ifndef __SOUND_ENGINE__


/* */
void
CG_SoundEngineStartStreamingSound ( char *sound, qboolean loop )
{
/*	if ( BASS_GetVersion() != MAKELONG( 2, 2) )
	{
#ifndef CGAME
		Com_Printf( "BASS version 2.2 was not loaded\n" );
#else
		CG_Printf( "BASS version 2.2 was not loaded\n" );
#endif
		return;
	}*/

	// setup output - default device
	if ( !engine_initialized )
	{
		CG_SoundEngineInfo();
		if ( !BASS_Init( 1, 44100, 0, 0, NULL) )
		{
			//Error( va( "Can't initialize device: %s.", BASS_GetDeviceDescription( 1)) );
			return;
		}
		else
		{
			engine_initialized = qtrue;
		}
	}

	if ( MyMusicTotal > 0 )
	{
		return;
	}

	if ( chan )
	{
		BASS_StreamFree( chan );
	}

	if ( act = BASS_ChannelIsActive( chan) )
	{

		// display some stuff and wait a bit
		music_time = timeGetTime() - starttime;
		level = BASS_ChannelGetLevel( chan );
		pos = BASS_ChannelGetPosition( chan, BASS_POS_BYTE );
	}

	CG_StartMusicTrack( 2, va( "%s", sound), loop );
}
#endif //__SOUND_ENGINE__
#endif //__MUSIC_ENGINE__
