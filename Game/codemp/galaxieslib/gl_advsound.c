///////////////////////////////////////////////////////////////////
//
// FILE: jkg_advsound.c
// PURPOSE: Runs a separate instance of OpenAL in JKA
// AUTHOR: eezstreet
//
///////////////////////////////////////////////////////////////////

#include "gl_advsound.h"

static unsigned int numAdvSFX = 0;

///////////////////////////////////////////////////////////////////
//
// WAV PROCESSING
//
///////////////////////////////////////////////////////////////////

// This function requires the file to be open in order for it to work properly
qboolean __fastcall JKG_DetermineIfWAVFile(advSFX_t *file)
{
	char buffer[16]; // don't need a whole lot of buffer to determine if WAV
	trap_FS_Read(buffer, 16, file->handle);

	// No need for NULL termination, we're just checking a few things
	if(Q_stricmpn(buffer, "RIFF", 4))
	{
		return qfalse;	//No RIFF in file.
	}
	// There's a DWORD here. Unknown what it does.
	if(Q_stricmpn(buffer+8, "WAVE", 4))
	{
		return qfalse; //No WAVE in file.
	}

	if(Q_stricmpn(buffer+12, "fmt ", 4))
	{
		return qfalse; //No fmt in file.
	}
	return qtrue;
}

// This function requires the file to be open in order for it to work properly
void JKG_ProcessWAVFile(advSFX_t *file)
{
	char buffer[4];
	trap_FS_Read(&file->chunkSize, sizeof(DWORD), file->handle);
	trap_FS_Read(&file->formatType, sizeof(short), file->handle);
	trap_FS_Read(&file->channels, sizeof(short), file->handle);
	trap_FS_Read(&file->sampleRate, sizeof(DWORD), file->handle);
	trap_FS_Read(&file->bytesPerSecond, sizeof(DWORD), file->handle);
	trap_FS_Read(&file->bytesPerSample, sizeof(short), file->handle);
	trap_FS_Read(&file->bitsPerSample, sizeof(short), file->handle);


	trap_FS_Read(buffer, 4, file->handle);

	if(Q_stricmpn(buffer, "data", 4))
	{
		Com_Error(ERR_FATAL, "No data in WAV file");
	}

	trap_FS_Read(&file->dataSize, sizeof(DWORD), file->handle);
	file->data = malloc(file->dataSize);
	trap_FS_Read(file->data, file->dataSize, file->handle);
}

///////////////////////////////////////////////////////////////////
//
// GENERAL SOUND FUNCTIONALITY
//
///////////////////////////////////////////////////////////////////

sfxHandle_t __stdcall JKG_RegisterAdvSound( const char *fileName )
{
	int i = 0;
	advSFX_t file;
	// Loop through the list of registered SFX and see if we have an already existing file (that way we don't load files that have already been loaded)
	while(registeredSFX[i].fileName[0])
	{
		if(!Q_stricmp(registeredSFX[i].fileName, fileName))
		{
			return (sfxHandle_t)i;
		}
		i++;
	}

	file.len = trap_FS_FOpenFile(fileName, &file.handle, FS_READ);
	if(file.len <= 0)
	{
		Com_Printf("^1ERROR: Invalid len in advSFX file: %s\n", fileName);
		trap_FS_FCloseFile(file.handle);
		return (sfxHandle_t)-1;
	}
	if(!file.handle)
	{
		return (sfxHandle_t)-1;
	}

	strcpy(file.fileName, fileName);

	// hmk...determine file type
	if(JKG_DetermineIfWAVFile(&file))
	{
		// Format: WAV
		file.advFormat = ADVFMT_WAV;
		JKG_ProcessWAVFile(&file);
		trap_FS_FCloseFile(file.handle);
	}
#ifdef __JKGADVMP3
	else if(JKG_DetermineIfMP3File(&file))
	{
		// Format: MP3
		file.advFormat = ADVFMT_MP3;
	}
#endif
#ifdef __JKGADVOGG
	else if(JKG_DetermineIfOGGFile(&file))
	{
		// Format: OGG Vorbis
		file.advFormat = ADVFMT_OGG;
	}
#endif
	else
	{
		Com_Printf("^1ERROR: %s invalid sound format!\n", fileName);
		trap_FS_FCloseFile(file.handle);
		return -1; // Not a valid format
	}
	registeredSFX[numAdvSFX++] = file;
	return (sfxHandle_t)(numAdvSFX-1);
}

void JKG_InitOpenAL( void )
{
	oaldevice = alcOpenDevice(NULL);
	if(!oaldevice)
	{
		Com_Error(ERR_FATAL, "JKG_InitOpenAL: no sound device");
		return;
	}
	oaldc = alcCreateContext(oaldevice, NULL);
	alcMakeContextCurrent(oaldc);
	if(!oaldc)
	{
		Com_Error(ERR_FATAL, "JKG_InitOpenAL: no device context");
		return;
	}
}

void JKG_CleanupOpenAL( void )
{
	alcMakeContextCurrent(NULL);
	alcDestroyContext(oaldc);
	alcCloseDevice(oaldevice);
}

// Call this relatively early in CGAME init
void JKG_InitAdvSoundSystem(void)
{
	numAdvSFX = 0;
	memset(registeredSFX, 0, sizeof(registeredSFX));
	memset(playingSounds, 0, sizeof(playingSounds));
	JKG_InitOpenAL();
}

// Call this relatively late in CGAME shutdown
void JKG_ShutdownAdvSoundSystem(void)
{
	int i = 0;
	while(registeredSFX[i].fileName[0])
	{
		free(registeredSFX[i].data);
		i++;
	}
	JKG_CleanupOpenAL();
}

static void JKG_AddToSoundsPlayed(sfxHandle_t soundIndex, int bufferID, int srcID)
{
	ALint bits;
	advSFX_t sfx = registeredSFX[soundIndex];

	if(sfx.formatType == ADVFMT_WAV)
	{
		playingSounds[numSoundsPlaying].bufferID = bufferID;
		playingSounds[numSoundsPlaying].srcID = srcID;
		playingSounds[numSoundsPlaying].fileHandle = soundIndex;
		// Calculate the start time and end time
		playingSounds[numSoundsPlaying].startTime = cg.time;
		alGetBufferi(bufferID, AL_BITS, &bits);
		playingSounds[numSoundsPlaying].endTime = cg.time + (((float)((sfx.dataSize * 8) / (sfx.channels * bits))/(float)sfx.sampleRate)*1000);
		numSoundsPlaying++;
	}
}

void JKG_PlaySoundIndex(sfxHandle_t soundIndex, qboolean looping, int channel)
{
	advSFX_t sfx;
	ALuint src;
	ALuint buffer;
	ALuint frequency;
	ALenum format = 0;
	ALfloat SourcePos[] = { 0.0, 0.0, 0.0 };                                    //Position of the source sound
    ALfloat SourceVel[] = { 0.0, 0.0, 1.0 };                                    //Velocity of the source sound
    ALfloat ListenerPos[] = { 0.0, 0.0, 0.0 };                                  //Position of the listener
    ALfloat ListenerVel[] = { 0.0, 0.0, 0.0 };                                  //Velocity of the listener
    ALfloat ListenerOri[] = { 0.0, 0.0, -1.0,  0.0, 1.0, 0.0 };                 //Orientation of the listener
	if(soundIndex < 0)
	{
		return; // Don't play the sound.
	}

	sfx = registeredSFX[soundIndex];
	frequency = sfx.sampleRate;

	alGenBuffers(1, &buffer);
	alGenSources(1, &src);
	if(alGetError() != AL_NO_ERROR)
	{
		Com_Error(ERR_FATAL, "JKG_PlaySoundIndex: bad gensource");
		return;
	}

	if(sfx.bitsPerSample == 8)
	{
		if(sfx.channels == 1)
		{
			format = AL_FORMAT_MONO8;
		}
		else if(sfx.channels == 2)
		{
			format = AL_FORMAT_STEREO8;
		}
	}
	else if(sfx.bitsPerSample == 16)
	{
		if(sfx.channels == 1)
		{
			format = AL_FORMAT_MONO16;
		}
		else if(sfx.channels == 2)
		{
			format = AL_FORMAT_STEREO16;
		}
	}

	if(!format)
	{
		Com_Printf("^1JKG_PlaySoundIndex: Bad Bits per Second\n");
		return;
	}

	alBufferData(buffer, format, sfx.data, sfx.dataSize, frequency);
	if(alGetError() != AL_NO_ERROR)
	{
		Com_Error(ERR_FATAL, "JKG_PlaySoundIndex: Could not init OpenAL buffer data.");
		return;
	}

	// DEBUG POSITIONING
	alListenerfv(AL_POSITION,    ListenerPos);                                  //Set position of the listener
    alListenerfv(AL_VELOCITY,    ListenerVel);                                  //Set velocity of the listener
    alListenerfv(AL_ORIENTATION, ListenerOri);                                  //Set orientation of the listener

	alSourcef (src, AL_MAX_DISTANCE, 100 );
	alSourcef (src, AL_REFERENCE_DISTANCE, 10 );
	alSourcef (src, AL_ROLLOFF_FACTOR, 0.8 );

	// Check the channel
	switch(channel)
	{
		case ADVCHAN_EFFECTS:
			{
				char volBuffer[16];
				trap_Cvar_VariableStringBuffer("s_volume", volBuffer, 16);
				if(atof(volBuffer) <= 0)
				{
					return;
				}
				alSourcef (src, AL_GAIN, atof(volBuffer)+0.00000001 );
			}
			break;
		case ADVCHAN_MUSIC:
			{
				char volBuffer[16];
				trap_Cvar_VariableStringBuffer("s_musicvolume", volBuffer, 16);
				if(atof(volBuffer) <= 0)
				{
					return;
				}
				alSourcef (src, AL_GAIN, atof(volBuffer) );
			}
			break;
		case ADVCHAN_VOICE:
			{
				char volBuffer[16];
				trap_Cvar_VariableStringBuffer("s_volumeVoice", volBuffer, 16);
				if(atof(volBuffer) <= 0)
				{
					return;
				}
				alSourcef (src, AL_GAIN, atof(volBuffer)+0.00000001 );
			}
			break;
		case ADVCHAN_EXT:
			if(s_ext_volume.value <= 0)
			{
				return;
			}
			alSourcef (src, AL_GAIN, s_ext_volume.value+0.00000001 );
			break;
	}

	alSourcei (src, AL_BUFFER,   buffer);                                 //Link the buffer to the source
    alSourcef (src, AL_PITCH,    1.0f     );                                 //Set the pitch of the source
    alSourcefv(src, AL_POSITION, SourcePos);                                 //Set the position of the source
    alSourcefv(src, AL_VELOCITY, SourceVel);                                 //Set the velocity of the source
    alSourcei (src, AL_LOOPING,  looping );                                 //Set if source is looping sound

	alDistanceModel(AL_LINEAR_DISTANCE);

	alSourcePlay(src);
	if(alGetError() != AL_NO_ERROR)
	{
		Com_Printf("^1Error playing advsound %s\n", sfx.fileName);
		return;
	}

	// Add to the array of playing sounds
	JKG_AddToSoundsPlayed(soundIndex, buffer, src);
}

void JKG_StopSound(sfxHandle_t soundIndex)
{
	int i;
	qboolean soundFound = qfalse;
	for(i = 0; i < numSoundsPlaying; i++)
	{
		if(playingSounds[i].fileHandle == soundIndex)
		{
			soundFound = qtrue;
			break;
		}
	}
	if(!soundFound)
	{
		return;
	}
	// Great. Got our sound. Now let's get down to business.
	alDeleteSources(1, &playingSounds[i].srcID);
	alDeleteBuffers(1, &playingSounds[i].bufferID);
	// Remember to delete the sound!
	memset(&playingSounds[i], 0, sizeof(playingSounds[i]));
	if(i < numSoundsPlaying-1)
	{
		int j = i+1;
		for(; j < numSoundsPlaying; j++)
		{
			memcpy(&playingSounds[j-1], &playingSounds[j], sizeof(advSoundPlaying_t));
		}
		memset(&playingSounds[numSoundsPlaying], 0, sizeof(playingSounds[numSoundsPlaying]));
	}
	numSoundsPlaying--;
}

void JKG_StopSoundIndex(int index)
{
	// Great. Got our sound. Now let's get down to business.
	alDeleteSources(1, &playingSounds[index].srcID);
	alDeleteBuffers(1, &playingSounds[index].bufferID);
	// Remember to delete the sound!
	memset(&playingSounds[index], 0, sizeof(playingSounds[index]));
	if(index < numSoundsPlaying-1)
	{
		int j = index+1;
		for(; j < numSoundsPlaying; j++)
		{
			memcpy(&playingSounds[j-1], &playingSounds[j], sizeof(advSoundPlaying_t));
		}
		memset(&playingSounds[numSoundsPlaying], 0, sizeof(playingSounds[numSoundsPlaying]));
	}
	numSoundsPlaying--;
}

void JKG_FadeOutSound(sfxHandle_t soundIndex, float speed)
{
	int i;
	qboolean soundFound = qfalse;
	for(i = 0; i < numSoundsPlaying; i++)
	{
		if(playingSounds[i].fileHandle == soundIndex)
		{
			soundFound = qtrue;
			break;
		}
	}
	if(!soundFound)
	{
		return;
	}
	// Great. Got our sound. Now let's get down to business.
	playingSounds[i].fading = qtrue;
	playingSounds[i].fadeSpeed = speed;
}

void JKG_SoundLoop(void)
{
	// Check for sounds and delete them when necessary
	int i;
	for(i = 0; i < numSoundsPlaying; i++)
	{
		if(cg.time > playingSounds[i].endTime)
		{
			int looping;
			alGetBufferi(playingSounds[i].bufferID, AL_LOOPING, &looping);
			if(!looping)
			{
				JKG_StopSoundIndex(i);
			}
		}
		// Fade out stuff
		if(playingSounds[i].fading)
		{
			float gain;
			alGetSourcef(playingSounds[i].srcID, AL_GAIN, &gain);
			alSourcef(playingSounds[i].srcID, AL_GAIN, gain-playingSounds[i].fadeSpeed);
		}
	}
}
