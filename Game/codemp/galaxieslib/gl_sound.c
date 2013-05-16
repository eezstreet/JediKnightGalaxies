////////////////////////////////////////
//  JKG FMOD-based Sound System
////////////////////////////////////////

#include "gl_enginefuncs.h"
#include "gl_sound.h"
#include "fmod.h"
#include "fmod_errors.h"

#define WIN32_LEAN_AND_MEAN
#undef INFINITE
#include <Windows.h>
#include <Rpc.h> // GUID conversion funcs

// FMOD default error checking function
static void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        Com_Error(ERR_FATAL, "FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        return;
    }
}

static void GL_LoadFMODLibrary(void)
{
	// Load the FMOD library
	HMODULE handle;
#ifdef _DEBUG
	const char *libname = "fmodexL.dll";
#else
	const char *libname = "fmodex.dll";
#endif

	if ((handle = GetModuleHandle(libname)) == 0) {
		char *basepath = Cvar_VariableString("fs_basepath");
		char *fspath = Cvar_VariableString("fs_game");
		char *dllpath = FS_BuildOSPath(basepath, fspath, libname);
		
		FS_CopyFile(libname);
		handle = LoadLibrary(dllpath);
		if (!handle) {
			Com_Error(0, "GL_LoadFMODLibrary failed: Could not load library\n");
			return;
		}
	}

	Com_Printf("Loaded FMOD Library\n");
}

/*#define S_COLOR_BLACK	"^0"
#define S_COLOR_RED		"^1"
#define S_COLOR_GREEN	"^2"*/
#define S_COLOR_YELLOW	"^3"
/*#define S_COLOR_BLUE	"^4"
#define S_COLOR_CYAN	"^5"
#define S_COLOR_MAGENTA	"^6"
#define S_COLOR_WHITE	"^7"*/

FMOD_SYSTEM *system;
static int s_soundStarted;

// MAX_SFX may be larger than MAX_SOUNDS because
// of custom player sounds
#define		MAX_SFX			4096
jkgsfx_t	s_knownSfx[MAX_SFX];
int			s_numSfx = 0;

#define		LOOP_HASH		128
static	jkgsfx_t	*sfxHash[LOOP_HASH];

// =======================================================================
// Load a sound
// =======================================================================

/*
================
return a hash value for the sfx name
================
*/
static long JKG_S_HashSFXName(const char *name) {
	int		i;
	long	hash;
	char	letter;

	hash = 0;
	i = 0;
	while (name[i] != '\0') {
		letter = tolower(name[i]);
		if (letter =='.') break;				// don't include extension
		if (letter =='\\') letter = '/';		// damn path names
		hash+=(long)(letter)*(i+119);
		i++;
	}
	hash &= (LOOP_HASH-1);
	return hash;
}

/*
==================
JKG_S_FindName

Will allocate a new sfx if it isn't found
==================
*/
static jkgsfx_t *JKG_S_FindName( const char *name ) {
	int		i;
	int		hash;

	jkgsfx_t	*sfx;

	if (!name) {
		Com_Error (ERR_FATAL, "S_FindName: NULL\n");
	}
	if (!name[0]) {
		Com_Error (ERR_FATAL, "S_FindName: empty name\n");
	}

	if (strlen(name) >= MAX_QPATH) {
		Com_Error (ERR_FATAL, "Sound name too long: %s", name);
	}

	hash = JKG_S_HashSFXName(name);

	sfx = sfxHash[hash];
	// see if already loaded
	while (sfx) {
		if (!Q_stricmp(sfx->soundName, name) ) {
			return sfx;
		}
		sfx = sfx->next;
	}

	// find a free sfx
	for (i=0 ; i < s_numSfx ; i++) {
		if (!s_knownSfx[i].soundName[0]) {
			break;
		}
	}

	if (i == s_numSfx) {
		if (s_numSfx == MAX_SFX) {
			Com_Error (ERR_FATAL, "S_FindName: out of sfx_t");
		}
		s_numSfx++;
	}
	
	sfx = &s_knownSfx[i];
	memset(sfx, 0, sizeof(*sfx));
	strcpy (sfx->soundName, name);

	sfx->next = sfxHash[hash];
	sfxHash[hash] = sfx;

	return sfx;
}

/*
==============
S_LoadSound

The filename may be different than sfx->name in the case
of a forced fallback of a player specific sound
==============
*/
static void JKG_S_LoadSound( jkgsfx_t *sound )
{
	byte	*data;
	int		size;
	FMOD_CREATESOUNDEXINFO sndExInfo;
	FMOD_SOUND *soundData;
	FMOD_RESULT result;

	// player specific sounds are never directly loaded
	if ( sound->soundName[0] == '*') {
		sound->defaultSound = qtrue;
		return;
	}

	// load it in
	size = FS_ReadFile( sound->soundName, (void **)&data );
	if ( !data || size <= 0 ) {
		sound->defaultSound = qtrue;
		return;
	}

	sound->lastTimeUsed = Sys_Milliseconds()+1;

	memset(&sndExInfo, 0, sizeof(sndExInfo));
	sndExInfo.cbsize = sizeof(sndExInfo);
	sndExInfo.length = size;
	result = FMOD_System_CreateSound(system, data, FMOD_SOFTWARE|FMOD_OPENMEMORY/*|FMOD_LOOP_NORMAL*/|FMOD_ACCURATETIME|FMOD_LOWMEM, &sndExInfo, &soundData); // Make the sound software mixed.
	ERRCHECK(result);

	FS_FreeFile( data );

	sound->soundObject = soundData;
	sound->inMemory = qtrue;
}

/*
==================
S_RegisterSound

Creates a default buzz sound if the file can't be loaded
==================
*/
sfxHandle_t	JKG_S_RegisterSound( const char *name )
{
	jkgsfx_t	*sound;

	if (!s_soundStarted) {
		return 0;
	}

	if ( strlen(name) >= MAX_QPATH ) {
		Com_Printf( "Sound name exceeds MAX_QPATH\n" );
		return 0;
	}

	sound = JKG_S_FindName( name );
	if ( sound->soundObject ) {
		if ( sound->defaultSound ) {
			Com_Printf( S_COLOR_YELLOW "WARNING: could not find %s - using default\n", sound->soundName );
			return 0;
		}
		return sound - s_knownSfx;
	}

	sound->inMemory = qfalse;

	JKG_S_LoadSound(sound);

	if ( sound->defaultSound ) {
		Com_Printf( S_COLOR_YELLOW "WARNING: could not find %s - using default\n", sound->soundName );
		return 0;
	}

	return sound - s_knownSfx;
}

void JKG_S_Init(void)
{
	FMOD_RESULT result;
	unsigned int version;
	int numdrivers;
	FMOD_SPEAKERMODE speakermode;
	FMOD_CAPS caps;
	char devicename[256];
	FMOD_GUID deviceguid;

	Com_Printf("------- JKG Sound Initialization -------\n");

	GL_LoadFMODLibrary(); // Fuck yeah, it works! :D

	////////// Start FMOD Init Code //////////
	/*
	Create a System object and initialize.
	*/
	result = FMOD_System_Create(&system);
	ERRCHECK(result);

	result = FMOD_System_GetVersion(system, &version);
	ERRCHECK(result);

	if (version < FMOD_VERSION)
	{
		Com_Error(ERR_FATAL, "Error! You are using an old version of FMOD %08x. This program requires %08x\n", version, FMOD_VERSION);
		return;
	}

	Com_DPrintf("FMOD Version: %08x\n", version);

	result = FMOD_System_GetNumDrivers(system, &numdrivers);
	ERRCHECK(result);

	if (numdrivers == 0)
	{
		Com_Printf("No sound devices found.\n");
		result = FMOD_System_SetOutput(system, FMOD_OUTPUTTYPE_NOSOUND);
		ERRCHECK(result);
	}
	else
	{
		// TODO: Handle cvar-configured driver to select
		result = FMOD_System_GetDriverInfo(system, 0, devicename, 256, &deviceguid);
		ERRCHECK(result);

		//check out UuidEqual and UuidFromString

		Com_Printf("Using sound device 0: %s\n", devicename );
		{
			char *guidstring = NULL;

			if ( UuidToString((UUID *)&deviceguid, &guidstring) == RPC_S_OK ) {
				Com_Printf("Device GUID: %s\n", guidstring );
				RpcStringFree(&guidstring);
			}
		}
		result = FMOD_System_GetDriverCaps(system, 0, &caps, 0, 0, &speakermode);
		ERRCHECK(result);

		// TODO: Handle cvar-configured speaker mode to set
		/*
		Set the user selected speaker mode.
		*/
		result = FMOD_System_SetSpeakerMode(system, speakermode);
		ERRCHECK(result);

		if (caps & FMOD_CAPS_HARDWARE_EMULATED)
		{
			/*
			The user has the 'Acceleration' slider set to off! This is really bad
			for latency! You might want to warn the user about this.
			*/
			// TODO: Show red message in Sound options menu to warn the user of this
			Com_Printf("^3Hardware acceleration is disabled!\n");
			result = FMOD_System_SetDSPBufferSize(system, 1024, 10);
			ERRCHECK(result);
		}

		if (strstr(devicename, "SigmaTel"))
		{
			/*
			Sigmatel sound devices crackle for some reason if the format is PCM 16bit.
			PCM floating point output seems to solve it.
			*/
			result = FMOD_System_SetSoftwareFormat(system, 48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0,0, FMOD_DSP_RESAMPLER_LINEAR);
			ERRCHECK(result);
		}
	}

#if 0
	FMOD_System_SetFileSystem(system, JKG_FMOD_FileOpen, JKG_FMOD_FileClose, JKG_FMOD_FileRead, JKG_FMOD_FileSeek, NULL, NULL, -1);
#endif

	result = FMOD_System_Init(system, 100, FMOD_INIT_NORMAL, 0);
	if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)
	{
		/*
		Ok, the speaker mode selected isn't supported by this soundcard. Switch it
		back to stereo...
		*/
		Com_Printf("Selected speaker mode not supported. Using stereo instead..\n");
		result = FMOD_System_SetSpeakerMode(system, FMOD_SPEAKERMODE_STEREO);
		ERRCHECK(result);

		/*
		... and re-init.
		*/
		result = FMOD_System_Init(system, 100, FMOD_INIT_NORMAL, 0);
		ERRCHECK(result);
	}
	////////// End FMOD Init Code //////////

	Com_Printf("----------------------------------------\n");

#if 0
	// Temp testing:
	Com_Printf("...Creating sound\n");
	result = FMOD_System_CreateSound(system, "Audio/SoundEffects/Anouncements/anouncementstart.mp3", FMOD_SOFTWARE/*|FMOD_LOOP_NORMAL*/|FMOD_ACCURATETIME|FMOD_LOWMEM, 0, &sound); // Make the sound software mixed.
	ERRCHECK(result);
	
	Com_Printf("...Playing sound\n");
	result = FMOD_System_PlaySound(system, FMOD_CHANNEL_FREE, sound, 0, &channel);
	ERRCHECK(result);

	Com_Printf("Ready\n");
#endif

	s_soundStarted = 1;

	{
		sfxHandle_t announcementSound;
		FMOD_CHANNEL *channel;

		announcementSound = JKG_S_RegisterSound("Audio/SoundEffects/Anouncements/anouncementstart.mp3");

		result = FMOD_System_PlaySound(system, FMOD_CHANNEL_FREE, s_knownSfx[announcementSound].soundObject, 0, &channel);
		ERRCHECK(result);
	}

	return;
}

void JKG_S_Update(void)
{
	FMOD_System_Update(system);
}

#if 0
FMOD_RESULT F_CALLBACK JKG_FMOD_FileOpen(const char *name, int unicode, unsigned int *filesize, void **handle, void **userdata)
{
	Com_Printf("JKG_FMOD_FileOpen(%s, %i, %08x, %08x, %08x)\n", name, unicode, filesize, handle, userdata);
	*filesize = FS_FOpenFileRead( name, (fileHandle_t *)handle, qtrue );

	if ( *filesize == -1 ) {
		return FMOD_ERR_FILE_NOTFOUND;
	} else {
		return FMOD_OK;
	}
}

FMOD_RESULT F_CALLBACK JKG_FMOD_FileClose(void *handle, void *userdata)
{
	Com_Printf("JKG_FMOD_FileClose(%08x, %08x)\n", handle, userdata);
	FS_FCloseFile( (fileHandle_t)handle );

	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK JKG_FMOD_FileRead(void *handle, void *buffer, unsigned int sizebytes, unsigned int *bytesread, void *userdata)
{
	Com_Printf("JKG_FMOD_FileRead(%08x, %08x, %i, %08x, %08x)\n", handle, buffer, sizebytes, bytesread, userdata);
	*bytesread = FS_Read( buffer, sizebytes, (fileHandle_t)handle );

	if ( *bytesread < sizebytes ) {
		return FMOD_ERR_FILE_EOF;
	} else {
		return FMOD_OK;
	}
}

FMOD_RESULT F_CALLBACK JKG_FMOD_FileSeek(void *handle, unsigned int pos, void *userdata)
{
	int result;

	Com_Printf("JKG_FMOD_FileSeek(%08x, %i, %08x)\n", handle, pos, userdata);
	result = FS_Seek( (fileHandle_t)handle, pos, FS_SEEK_SET );

	if ( result == -1 ) {
		return FMOD_ERR_FILE_COULDNOTSEEK;
	} else {
		return FMOD_OK;
	}
}
#endif