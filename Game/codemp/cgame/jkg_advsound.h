#ifndef __JKG_ADVSOUND_H
#define __JKG_ADVSOUND_H

///////////////////////////////////////////////////////////////////
//
// FILE: jkg_advsound.h
// PURPOSE: Runs a separate instance of OpenAL in JKA
// AUTHOR: eezstreet
//
///////////////////////////////////////////////////////////////////

#include "cg_local.h"
#include <inc/fmod.h>
#include <inc/fmod_errors.h>

#define VectorCopyToFMOD_A(a,b) ((b)[0]=(a).x,(b)[1]=(a).y,(b)[2]=(a).z)
#define VectorCopyToFMOD_B(a,b) ((b).x=(a)[0],(b).y=(a)[1],(b).z=(a)[2])

// Using an FMOD based renderer, not OpenAL
void JKG_InitAdvSoundSystem(void);
void JKG_ShutdownAdvSoundSystem(void);

typedef struct {
	char filename[64];
	FMOD_SOUND *fmodsound;
} fmodRegistry_t;

typedef struct {
	float volumeDecrease;
	FMOD_CHANNEL *chan;
} fmodFadeStruct_t;

FMOD_SOUND *JKG_RegisterAdvSound(char *filename, int flags);
void JKG_AdvSoundPlaySound(FMOD_SOUND *fSound, qboolean startPaused, float startVolume, vec3_t origin, vec3_t velocity, FMOD_CHANNEL **channel, int soundChannel);
void JKG_AdvSoundUpdateListener(void);
void JKG_AdvSoundPlaySound2D(FMOD_SOUND *fSound, qboolean startPaused, float startVolume, FMOD_CHANNEL **channel, int soundChannel );
void JKG_AdvSoundFade(FMOD_CHANNEL *chan, float fadeTime);

typedef enum
{
	ADVCHAN_SOUND,
	ADVCHAN_MUSIC,
	ADVCHAN_VOICE,
	ADVCHAN_OTHER,
} jkgAdvMusicChannels_t;


// TEST
void JKG_TestFMOD3D ( void );
#endif