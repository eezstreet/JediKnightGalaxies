#ifndef _GL_SOUND_H
#define _GL_SOUND_H

#include "gl_enginefuncs.h"
#include "fmod.h"

/* [NOTES]
Required cvars:
s_device:		Stored sound device setting, will be set to "auto" on release which means it'll use the default sound device set in the Control Panel.
				GUID of the sound device will be stored so duplicate device names or re-ordered device list numbers don't affect the setting.

s_speakermode:	Stored speaker mode setting, will be set to "auto" on release which means it uses the Control Panel speaker mode setting

s_doppler:		Enable/disable the doppler effect. We could make this a float value for doppler scale.

s_enabled:		Enable/disable the sound system. "Disabling" the sound system will still start-up FMOD but use FMOD_OUTPUTTYPE_NOSOUND

s_3DOcclusion:	Enable/disable 3D geometry occlusion. Disabling this could be counted as "wallhacking" sounds?..

s_reverb:		Enable/disable environmental/local reverb effects.

s_volume:		Regular sounds volume. Shared with BaseJKA's sound system.

s_musicVolume:	Background music volume. Shared with BaseJKA's sound system.

s_volumeVoice:	Character voices volume. Shared with BaseJKA's sound system.

s_

*/

// -- Public interface
void		JKG_S_Init(void);
void		JKG_S_Shutdown(void);
void		JKG_S_Update(void);

// -- CGame/UI Trap Call Equivalents
// normal sounds will have their volume dynamically changed as their entity
// moves and the listener moves
int			JKG_S_GetVoiceVolume( int entityNum );
void		JKG_S_MuteSound( int entityNum, int entchannel );
void		JKG_S_StartSound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx );
void		JKG_S_StopLoopingSound(int entnum);

// a local sound is always played full volume
void		JKG_S_StartLocalSound( sfxHandle_t sfx, int channelNum );
void		JKG_S_ClearLoopingSounds( void );
void		JKG_S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
void		JKG_S_AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
void		JKG_S_UpdateEntityPosition( int entityNum, const vec3_t origin );

// repatialize recalculates the volumes of sound as they should be heard by the
// given entityNum and position
void		JKG_S_Respatialize( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater );
void		JKG_S_ShutUp(qboolean shutUpFactor);
sfxHandle_t	JKG_S_RegisterSound( const char *name ); // returns buzz if not found
void		JKG_S_StartBackgroundTrack( const char *intro, const char *loop, qboolean bReturnWithoutStarting);	// empty name stops music
void		JKG_S_StopBackgroundTrack( void );

void		JKG_S_UpdateAmbientSet( const char *name, vec3_t origin );
void		JKG_AS_ParseSets( void );
void		JKG_AS_AddPrecacheEntry( const char *name );
int			JKG_S_AddLocalSet( const char *name, vec3_t listener_origin, vec3_t origin, int entID, int time );
sfxHandle_t	JKG_AS_GetBModelSound( const char *name, int stage );

// -- FMOD Filesystem Wrappers
#if 0
FMOD_RESULT F_CALLBACK JKG_FMOD_FileOpen(const char *name, int unicode, unsigned int *filesize, void **handle, void **userdata);
FMOD_RESULT F_CALLBACK JKG_FMOD_FileClose(void *handle, void *userdata);
FMOD_RESULT F_CALLBACK JKG_FMOD_FileRead(void *handle, void *buffer, unsigned int sizebytes, unsigned int *bytesread, void *userdata);
FMOD_RESULT F_CALLBACK JKG_FMOD_FileSeek(void *handle, unsigned int pos, void *userdata);
#endif

// Looks like we have to manage sound memory ourselves...
typedef struct jkgsfx_s {
	char 			soundName[MAX_QPATH];
	qboolean		is3DSound;
	FMOD_SOUND		*soundObject;
	qboolean		defaultSound;
	qboolean		inMemory;
	/* BaseQ3 stuff:
	sndBuffer		*soundData;
	qboolean		defaultSound;			// couldn't be loaded, so use buzz
	qboolean		inMemory;				// not in Memory
	qboolean		soundCompressed;		// not in Memory
	int				soundCompressionMethod;	
	int 			soundLength;*/
	int				lastTimeUsed;
	struct jkgsfx_s	*next;
} jkgsfx_t;

#endif