
///////////////////////////////////////////////////////////////////
//
// FILE: jkg_advsound.c
// PURPOSE: Runs a separate instance of OpenAL in JKA
// AUTHOR: eezstreet
//
///////////////////////////////////////////////////////////////////

#include "jkg_advsound.h"
// FMOD crapola
void JKG_CheckFMODError(FMOD_RESULT result)
{
	if(result != FMOD_OK)
	{
		Com_Error(ERR_FATAL, "FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		return;
	}
}

#define check	JKG_CheckFMODError(result)

static FMOD_SYSTEM *fmsystem;
static fmodRegistry_t soundRegistry[256];
static int numRegisteredAdvSounds;
static fmodFadeStruct_t *fadechannels;
static int numFadeChannels;
static int numFadeAllocatedMem;

void JKG_InitAdvSoundSystem(void)
{
	FMOD_RESULT      result;
	unsigned int version;
	int numDrivers;
	FMOD_SPEAKERMODE speakermode;
    FMOD_CAPS        caps;
	char             name[256];

	Com_Printf("----FMOD init----\n");

	numFadeChannels = 0;
	numFadeAllocatedMem = 1;


	result = FMOD_System_Create(&fmsystem);
	check;

	result = FMOD_System_GetVersion(fmsystem, &version);
	check;

	if ( version < FMOD_VERSION )
	{
		Com_Error(ERR_FATAL, "FMOD: FMOD version mismatch (found %08x, expected %08x)");
		return;
	}

	result = FMOD_System_GetNumDrivers(fmsystem, &numDrivers);
	check;

	if(numDrivers == 0)
	{
		result = FMOD_System_SetOutput(fmsystem, FMOD_OUTPUTTYPE_NOSOUND);
		check;

		Com_Printf("^3WARNING: FMOD set with no sound output.\n");
	}
	else
	{
		result = FMOD_System_GetDriverCaps(fmsystem, 0, &caps, 0, &speakermode);
        check;

		result = FMOD_System_SetSpeakerMode(fmsystem, speakermode);       /* Set the user selected speaker mode. */
        check;

		if (caps & FMOD_CAPS_HARDWARE_EMULATED)             /* The user has the 'Acceleration' slider set to off!  This is really bad for latency!. */
        {                                                   /* You might want to warn the user about this. */
			Com_Printf("^3WARNING: FMOD: Audio hardware acceleration disabled. This will likely affect FPS.\n");
            result = FMOD_System_SetDSPBufferSize(fmsystem, 1024, 10);
            check;
        }

		result = FMOD_System_GetDriverInfo(fmsystem, 0, name, 256, 0);
        check;

		if (strstr(name, "SigmaTel"))   /* Sigmatel sound devices crackle for some reason if the format is PCM 16bit.  PCM floating point output seems to solve it. */
        {
			Com_Printf("SigmaTel device detected, forced to PCM floating point output.\n");
            result = FMOD_System_SetSoftwareFormat(fmsystem, 48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0,0, FMOD_DSP_RESAMPLER_LINEAR);
            check;
        }
	}

	result = FMOD_System_Init(fmsystem, 100, FMOD_INIT_NORMAL, 0);		//TODO: cvar second arg
	if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)         /* Ok, the speaker mode selected isn't supported by this soundcard.  Switch it back to stereo... */
    {
		Com_Printf("^3WARNING: Speaker mode invalid for this sound card. Forcing to stereo...\n");
        result = FMOD_System_SetSpeakerMode(fmsystem, FMOD_SPEAKERMODE_STEREO);
        check;
                
        result = FMOD_System_Init(fmsystem, 100, FMOD_INIT_NORMAL, 0);/* ... and re-init. */		// TODO: cvar second arg
        check;
    }

	result = FMOD_System_Set3DSettings(fmsystem, 1.0, JKA_UNITS_PER_METER, 1.0f);
    check;

	numRegisteredAdvSounds = 0;

	fadechannels = malloc(sizeof(fmodFadeStruct_t)*numFadeAllocatedMem);
}

void JKG_ShutdownAdvSoundSystem(void)
{
	FMOD_RESULT		result;

	free(fadechannels);

	result = FMOD_System_Close(fmsystem);
	check;
	result = FMOD_System_Release(fmsystem);
	check;
}

qboolean JKG_AdvSoundCreate(char *filename, FMOD_SOUND **fSound, int flags)
{
	fileHandle_t f;
	char *buffer;
	FMOD_RESULT      result;
	FMOD_CREATESOUNDEXINFO info;
	int len = trap_FS_FOpenFile(filename, &f, FS_READ);
	if(!len || len < 0)
	{
		trap_FS_FCloseFile(f);
		return qfalse;
	}

	memset(&info, 0, sizeof(info));


	buffer = malloc(len);
	trap_FS_Read(buffer, len, f);

	info.cbsize                       = sizeof(FMOD_CREATESOUNDEXINFO);
	/*if(flags & FMOD_3D)
	{
		info.numchannels                = 1;  
	}
	else
	{
		info.numchannels				= 2;
	}*/
    info.defaultfrequency           = 44100;  
    //info.format                     = FMOD_SOUND_FORMAT_PCM16;
	info.length						= len;

	result = FMOD_System_CreateSound(fmsystem, buffer, flags | FMOD_SOFTWARE|FMOD_OPENMEMORY|FMOD_ACCURATETIME|FMOD_LOWMEM, &info, fSound);
	check;

	free(buffer);

	return qtrue;
}

FMOD_SOUND *JKG_RegisterAdvSound(char *filename, int flags)
{
	int i;
	FMOD_SOUND *fSound;
	for(i = 0; i < numRegisteredAdvSounds; i++)
	{
		if(!Q_stricmp(soundRegistry[i].filename, filename))
		{
			return soundRegistry[i].fmodsound;
		}
	}
	// File not found. Best to register it then.
	if(!JKG_AdvSoundCreate(filename, &fSound, flags))
	{
		return NULL;
	}
	else
	{
		soundRegistry[numRegisteredAdvSounds].fmodsound = fSound;
		strcpy(soundRegistry[numRegisteredAdvSounds].filename, filename);
		numRegisteredAdvSounds++;
		return fSound;
	}
}

void JKG_AdvSoundSetMinMaxDistance(FMOD_SOUND *fSound, float minDist, float maxDist)
{
	FMOD_RESULT      result;
	result = FMOD_Sound_Set3DMinMaxDistance(fSound, minDist, maxDist);
	check;
}

void JKG_AdvSoundSetCone(FMOD_SOUND *fSound, float insideConeAngle, float outsideConeAngle, float outsideVolume)
{
	FMOD_RESULT      result;
	result = FMOD_Sound_Set3DConeSettings(fSound, insideConeAngle, outsideConeAngle, outsideVolume);
	check;
}

void JKG_AdvSoundSetMode(FMOD_SOUND *fSound, unsigned int mode)
{
	FMOD_RESULT      result;
	result = FMOD_Sound_SetMode(fSound, mode);
	check;
}



// Handle playing sound shizz --eez
void JKG_AdvSoundPlaySound(FMOD_SOUND *fSound, qboolean startPaused, float startVolume, vec3_t origin, vec3_t velocity, FMOD_CHANNEL **channel, int soundChannel)
{
	FMOD_RESULT      result;
	FMOD_VECTOR		 vel, org;
	FMOD_CHANNEL *derp;
	float soundVolume;
	char buffer[16];
	vec3_t ZeroVector = { 0, 0, 0 };

	result = FMOD_System_Set3DSettings(fmsystem, 1.0, 1.0, 0.3f);
    check;

	if(velocity)
	{
		VectorCopyToFMOD_B(velocity, vel);
	}
	else
	{
		VectorCopyToFMOD_B(ZeroVector, vel);
	}
	if(origin)
	{
		VectorCopyToFMOD_B(origin, org);
	}
	else
	{
		VectorCopyToFMOD_B(ZeroVector, org);
	}

	switch(soundChannel)
	{
		default:
		case ADVCHAN_SOUND:
			{
				trap_Cvar_VariableStringBuffer("s_volume", buffer, 16);
				soundVolume = atof(buffer);
			}
			break;
		case ADVCHAN_MUSIC:
			{
				trap_Cvar_VariableStringBuffer("s_musicvolume", buffer, 16);
				soundVolume = atof(buffer);
			}
			break;
		case ADVCHAN_VOICE:
			{
				trap_Cvar_VariableStringBuffer("s_voicevolume", buffer, 16);
				soundVolume = atof(buffer);
			}
			break;
	}

	if(channel == NULL || *channel == NULL)
	{
		//JKG_AdvSoundSetMinMaxDistance(fSound, 1.0, 0.1);
		result = FMOD_System_PlaySound( fmsystem, FMOD_CHANNEL_FREE, fSound, (FMOD_BOOL)startPaused, &derp );
		check;
		result = FMOD_Channel_Set3DAttributes( derp, &org, &vel );
		check;
		result = FMOD_Channel_SetPaused( derp, startPaused );
		check;
		result = FMOD_Channel_SetVolume( derp, (soundVolume*startVolume) );
		check;
	}
	else
	{
		result = FMOD_System_PlaySound(fmsystem, FMOD_CHANNEL_FREE, fSound, (FMOD_BOOL)startPaused, channel);
		check;
		result = FMOD_Channel_Set3DAttributes(*channel, &org, &vel);
		check;
		result = FMOD_Channel_SetPaused(*channel, startPaused);
		check;
		result = FMOD_Channel_SetVolume(*channel, (soundVolume*startVolume) );
		check;
	}
}

void JKG_AdvSoundPlaySound2D(FMOD_SOUND *fSound, qboolean startPaused, float startVolume, FMOD_CHANNEL **channel, int soundChannel )
{
	FMOD_RESULT      result;
	FMOD_CHANNEL *derp;
	float soundVolume;
	char buffer[16];

	switch(soundChannel)
	{
		default:
		case ADVCHAN_SOUND:
			{
				trap_Cvar_VariableStringBuffer("s_volume", buffer, 16);
				soundVolume = atof(buffer);
			}
			break;
		case ADVCHAN_MUSIC:
			{
				trap_Cvar_VariableStringBuffer("s_musicvolume", buffer, 16);
				soundVolume = atof(buffer);
			}
			break;
		case ADVCHAN_VOICE:
			{
				trap_Cvar_VariableStringBuffer("s_voicevolume", buffer, 16);
				soundVolume = atof(buffer);
			}
			break;
	}

	if(channel == NULL)
	{
		result = FMOD_System_PlaySound(fmsystem, FMOD_CHANNEL_FREE, fSound, (FMOD_BOOL)startPaused, &derp);
	}
	else
	{
		result = FMOD_System_PlaySound(fmsystem, FMOD_CHANNEL_FREE, fSound, (FMOD_BOOL)startPaused, channel);
	}
	check;
	if(channel && *channel)
	{
		result = FMOD_Channel_SetPaused(*channel, startPaused);
	}
	else
	{
		result = FMOD_Channel_SetPaused(derp, startPaused);
	}
	check;
	if(channel && *channel)
	{
		result = FMOD_Channel_SetVolume(*channel, (soundVolume*startVolume));
	}
	else
	{
		result = FMOD_Channel_SetVolume(derp, (soundVolume*startVolume));
	}
	check;
}

void JKG_AdvSoundFade(FMOD_CHANNEL *chan, float fadeTime)
{
	fmodFadeStruct_t fadeStruct;
	if((numFadeChannels + 1) >= numFadeAllocatedMem)
	{
		numFadeAllocatedMem *= 2;
		fadechannels = realloc(fadechannels, (sizeof(fmodFadeStruct_t)*numFadeAllocatedMem));
	}

	fadeStruct.volumeDecrease = fadeTime;
	fadeStruct.chan = chan;

	fadechannels[numFadeChannels++] = fadeStruct;
}

void JKG_AdvSoundRemoveFromFadeList(int index, qboolean stopmode)
{
	FMOD_RESULT      result;
	if(!fadechannels)
	{
		return;
	}
	if(fadechannels[index].chan)
	{
		if(stopmode)
		{
			result = FMOD_Channel_Stop(fadechannels[index].chan);
		}
	}
	memcpy(fadechannels+((sizeof(fmodFadeStruct_t))*index), (fadechannels+((sizeof(fmodFadeStruct_t))*(index+1))), (sizeof(fmodFadeStruct_t)*(numFadeChannels-index-1)));
	numFadeChannels--;
}

void JKG_AdvSoundDoFadeNonsense(void)
{
	// Fade out some of the sounds that we have going.
	FMOD_RESULT      result;
	int i;
	if(!fadechannels)
	{
		return;
	}
	for(i = 0; i < numFadeChannels; i++)
	{
		if(!fadechannels[i].chan)
		{
			continue;
		}
		else
		{
			float curVolume;
			result = FMOD_Channel_GetVolume(fadechannels[i].chan, &curVolume);
			check;

			if(curVolume <= 0 )
			{
				JKG_AdvSoundRemoveFromFadeList(i, qtrue);
			}
			else if(curVolume >= 1)
			{
				JKG_AdvSoundRemoveFromFadeList(i, qfalse);
			}
			else
			{
				FMOD_Channel_SetVolume(fadechannels[i].chan, curVolume+fadechannels[i].volumeDecrease);
				check;
			}
		}
	}
}

// Sound loop
void JKG_AdvSoundUpdateListener(void)
{
	FMOD_VECTOR listenerPos;
	FMOD_VECTOR listenerVel;
	FMOD_VECTOR forward, up;
	FMOD_RESULT	result;
	vec3_t ZeroVec = {0, 0, 0};

	VectorCopyToFMOD_B(ZeroVec, forward);
	VectorCopyToFMOD_B(ZeroVec, up);

	VectorCopyToFMOD_B(cg.predictedPlayerState.origin, listenerPos);
	VectorCopyToFMOD_B(ZeroVec, listenerVel); // TODO: Set listener velocity to player velocity


	result = FMOD_System_Set3DListenerAttributes(fmsystem, 0, &listenerPos, &listenerVel, &forward, &up);
	check;

	JKG_AdvSoundDoFadeNonsense();

	FMOD_System_Update(fmsystem);
}

void JKG_TestFMOD3D ( void )
{
	float x = atof(CG_Argv(1));
	float y = atof(CG_Argv(2));
	float z = atof(CG_Argv(3));
	vec3_t origin = {x, y, z};

	JKG_AdvSoundPlaySound(JKG_RegisterAdvSound("sound/test/xwing_fly.wav", FMOD_3D), qfalse, 10.0f, origin, NULL, NULL, ADVCHAN_SOUND);
}

#undef check