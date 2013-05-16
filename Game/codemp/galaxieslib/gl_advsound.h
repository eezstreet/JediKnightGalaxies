///////////////////////////////////////////////////////////////////
//
// FILE: jkg_advsound.h
// PURPOSE: Runs a separate instance of OpenAL in JKA
// AUTHOR: eezstreet
//
///////////////////////////////////////////////////////////////////

#ifndef __GL_ADVSOUND_H
#define __GL_ADVSOUND_H
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include "gl_enginefuncs.h"
#include <AL/al.h>
#include <AL/alc.h>

typedef enum {
	ADVFMT_NONE,
	ADVFMT_WAV,
#ifdef __JKGADVMP3
	ADVFMT_MP3,
#endif
#ifdef __JKGADVOGG
	ADVFMT_OGG,
#endif
} advSFXFormatTypes_e;

typedef enum {
	ADVCHAN_EFFECTS,
	ADVCHAN_MUSIC,
	ADVCHAN_VOICE,
	ADVCHAN_EXT,
} advSFXChannels_e;

typedef struct {

	// File data
	fileHandle_t handle;
	int len;
	char fileName[MAX_QPATH];
	advSFXFormatTypes_e advFormat;
	
	// WAV specific data
	DWORD chunkSize;
	short formatType;
	short channels;
	DWORD sampleRate;
	DWORD bytesPerSecond;
	short bytesPerSample;
	short bitsPerSample;
	DWORD dataSize;
	BYTE *data;
} advSFX_t;

typedef struct {
	sfxHandle_t fileHandle;
	int startTime;
	int endTime;
	int bufferID;
	int srcID;

	// State specific stuff
	qboolean fading;
	float fadeSpeed;
} advSoundPlaying_t;

advSFX_t registeredSFX[256];
advSoundPlaying_t playingSounds[512];

int numSoundsPlaying;

void JKG_InitAdvSoundSystem(void);
void JKG_ShutdownAdvSoundSystem(void);
int __stdcall JKG_RegisterAdvSound( const char *fileName );
void JKG_PlaySoundIndex(sfxHandle_t soundIndex, qboolean looping, int channel);
void JKG_StopSound(sfxHandle_t soundIndex);
void JKG_FadeOutSound(sfxHandle_t soundIndex, float speed);

////////////////////////////////////////////////////////////////////////////
//
/// OpenAL stuff
//
//

ALCdevice *oaldevice;
ALCcontext *oaldc;

#endif