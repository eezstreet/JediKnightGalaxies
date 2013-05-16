// jkg_gangwars.c: File by eezstreet
// Copyright (c) 2012 Jedi Knight Galaxies
// ===============================================
// jkg_gangwars.c controls the loading of .team files
// for Jedi Knight Galaxies' Gang Wars TFFA add-on.
// The data is handled as per BG specifications.
// ===============================================

#include "q_shared.h"
#include "bg_saga.h"
#include "jkg_gangwars.h"
#include "bg_weapons.h"
#include "bg_public.h"

extern int	trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
extern void	trap_FS_Read( void *buffer, int len, fileHandle_t f );
extern void	trap_FS_Write( const void *buffer, int len, fileHandle_t f );
extern void	trap_FS_FCloseFile( fileHandle_t f );
extern int	trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize );

extern int BG_SiegeGetValueGroup(char *buf, char *group, char *outbuf);
extern void BG_StripTabs(char *buf);
extern int BG_GetPairedValue(char *buf, char *key, char *outbuf);

#ifndef QAGAME //cgame, ui
#ifndef UI
qhandle_t	trap_R_RegisterShaderNoMip( const char *name );
qhandle_t	trap_R_RegisterShader( const char *name );
#endif
#endif

void JKG_BG_ParseGangWarsTeam(const char *filename)
{
	fileHandle_t f;
	int len;
	char buffer[8192];
	char parseBuf[4096];

	len = trap_FS_FOpenFile(filename, &f, FS_READ);

	if(!f)
	{
		Com_Printf("^1Error loading Gang wars file (%s): NULL handle\n", filename);
		return;
	}
	if(!len || len >= 8192)
	{
		Com_Printf("^1Error loading Gang wars file (%s): Invalid file size range\n", filename);
		trap_FS_FCloseFile(f);
		return;
	}

	trap_FS_Read(buffer, len, f);

	trap_FS_FCloseFile(f);

	buffer[len] = '\0';

	if (BG_GetPairedValue(buffer, "name", parseBuf))
	{
		strcpy(bgGangWarsTeams[bgnumGangWarTeams].name, parseBuf);
	}
	else
	{
		Com_Error(ERR_DROP, "Gang wars team (%s) with no name!\n", filename);
		return;
	}

	if (BG_GetPairedValue(buffer, "reference", parseBuf))
	{
		strcpy(bgGangWarsTeams[bgnumGangWarTeams].refPtr, parseBuf);
	}
	else
	{
		Com_Error(ERR_DROP, "Gang wars team (%s) with no reference name!\n", filename);
		return;
	}

	memset(bgGangWarsTeams[bgnumGangWarTeams].modelStore, 0, sizeof(bgGangWarsTeams[bgnumGangWarTeams].modelStore));
	
#ifndef QAGAME
#ifndef UI
	if (BG_GetPairedValue(buffer, "icon", parseBuf))
	{
		bgGangWarsTeams[bgnumGangWarTeams].teamIcon = trap_R_RegisterShader(parseBuf);
	}
	else
	{
		bgGangWarsTeams[bgnumGangWarTeams].teamIcon = trap_R_RegisterShader("sprites/team_red");
	}
#else
	bgGangWarsTeams[bgnumGangWarTeams].teamIcon = 0;
#endif
#else
	bgGangWarsTeams[bgnumGangWarTeams].teamIcon = 0;
#endif
	if ( BG_GetPairedValue( buffer, "useTeamColors", parseBuf ) )
	{
		bgGangWarsTeams[bgnumGangWarTeams].useTeamColors = (qboolean)atoi(parseBuf);
	}
	else
	{
		bgGangWarsTeams[bgnumGangWarTeams].useTeamColors = qfalse;
	}

	if (BG_GetPairedValue(buffer, "joinstring", parseBuf))
	{
		strcpy(bgGangWarsTeams[bgnumGangWarTeams].joinstring, parseBuf);
	}
	else
	{
		bgGangWarsTeams[bgnumGangWarTeams].joinstring[0] = 0;
	}
	if (BG_GetPairedValue(buffer, "leadstring", parseBuf))
	{
		strcpy(bgGangWarsTeams[bgnumGangWarTeams].leadstring, parseBuf);
	}
	else
	{
		bgGangWarsTeams[bgnumGangWarTeams].leadstring[0] = 0;
	}
	if (BG_GetPairedValue(buffer, "longname", parseBuf))
	{
		strcpy(bgGangWarsTeams[bgnumGangWarTeams].longname, parseBuf);
	}
	else
	{
		bgGangWarsTeams[bgnumGangWarTeams].longname[0] = 0;
	}
	if (BG_GetPairedValue(buffer, "menujoinstring", parseBuf))
	{
		strcpy(bgGangWarsTeams[bgnumGangWarTeams].menujoinstring, parseBuf);
	}
	else
	{
		bgGangWarsTeams[bgnumGangWarTeams].menujoinstring[0] = 0;
	}
	if (BG_GetPairedValue(buffer, "menustring", parseBuf))
	{
		strcpy(bgGangWarsTeams[bgnumGangWarTeams].menustring, parseBuf);
	}
	else
	{
		bgGangWarsTeams[bgnumGangWarTeams].menustring[0] = 0;
	}
	if (BG_GetPairedValue(buffer, "toomanystring", parseBuf))
	{
		strcpy(bgGangWarsTeams[bgnumGangWarTeams].toomanystring, parseBuf);
	}
	else
	{
		bgGangWarsTeams[bgnumGangWarTeams].toomanystring[0] = 0;
	}

	// Team colors! :3
	if (BG_GetPairedValue(buffer, "teamColor", parseBuf))
	{
		// Parse this good
		char *buffer = parseBuf+1;
		char tempBuffer[8];
		vec4_t tempColor;
		int i = 0;
		int j;
		for(j = 0; j < 4; j++)
		{
			while(buffer[i] != ',' && buffer[i] != '\0' && buffer[i] != '\r' && buffer[i] != '\n' && buffer[i] != '}')
			{
				tempBuffer[i] = buffer[i];
				i++;
			}
			tempBuffer[i] = '\0';
			// Grab the value
			tempColor[j] = atof(tempBuffer);
			tempColor[j] /= 255.0f;
			tempBuffer[0] = '\0';
			buffer += i;
			i = 0;
			buffer++;
		}
		VectorCopy4(tempColor, bgGangWarsTeams[bgnumGangWarTeams].teamColor);
	}
	else
	{
		vec4_t tempColor;
		tempColor[0] = tempColor[1] = tempColor[2] = tempColor[3] = 1.0f;
		VectorCopy4(tempColor, bgGangWarsTeams[bgnumGangWarTeams].teamColor);
	}

	// Now we do a mega hack
	if (BG_GetPairedValue(buffer, "modelstore", parseBuf))
	{
		// These are comma-delimited fields which specify each model
		// E.G.: "kyle/default,jan/default" would be the default
		int l = strlen(parseBuf);
		int i, j;
		j = 0;
		for(i = 0; i < l; i++)
		{
			if(parseBuf[i] == ',')
			{
				strncpy(bgGangWarsTeams[bgnumGangWarTeams].modelStore[bgGangWarsTeams[bgnumGangWarTeams].numModelsInStore++], parseBuf+j, i-j);
				bgGangWarsTeams[bgnumGangWarTeams].modelStore[bgGangWarsTeams[bgnumGangWarTeams].numModelsInStore][i-j+1] = '\0';
				j = i + 1;
			}
			if(parseBuf[i] == '\n')
			{
				break;
			}
			if(parseBuf[i] == '\0')
			{
				break;
			}
		}
	}
	else
	{
		//Just use Kyle for default I guess..
		char defaultModel[MAX_QPATH];
		strcpy(defaultModel, "kyle/default");
		strcpy(bgGangWarsTeams[bgnumGangWarTeams].modelStore[0], defaultModel);
		strcpy(defaultModel, "jan/default");
		strcpy(bgGangWarsTeams[bgnumGangWarTeams].modelStore[1], defaultModel);
	}

	if ( BG_GetPairedValue( buffer, "defaultModel", parseBuf ) )
	{
		strcpy(bgGangWarsTeams[bgnumGangWarTeams].defaultModel, parseBuf);
	}
	else
	{
		if(!Q_stricmp(bgGangWarsTeams[bgnumGangWarTeams].modelStore[0], "NULL"))
		{
			strcpy(bgGangWarsTeams[bgnumGangWarTeams].defaultModel, "kyle");
		}
		else
		{
			strcpy(bgGangWarsTeams[bgnumGangWarTeams].defaultModel, bgGangWarsTeams[bgnumGangWarTeams].modelStore[0]);	// Use the first model as the default if none specified
		}
	}

	bgnumGangWarTeams++;
}

void JKG_BG_LoadGangWarTeams(void)
{
	int numFiles, i, filelen;
	char fileList[4096];
	char filename[MAX_QPATH];
	char* fileptr;
	numFiles = trap_FS_GetFileList("ext_data/gangwars", ".team", fileList, 4096);
	fileptr = fileList;
	for(i = 0; i < numFiles; i++, fileptr += filelen+1)
	{
		filelen = strlen(fileptr);
		strcpy(filename, "ext_data/gangwars/");
		strcat(filename, fileptr);
		JKG_BG_ParseGangWarsTeam(filename);
	}
}

void JKG_BG_GangWarsInit(void)
{
	bgnumGangWarTeams = 0;
	JKG_BG_LoadGangWarTeams();
}

void JKG_BG_GangWarsExit(void)
{
}

int JKG_GetTeamByReference(char *reference)
{
	// This is rather expensive. Please do not call this every frame if entirely possible
	int i;
	for(i = 0; i < bgnumGangWarTeams; i++)
	{
		if(!Q_stricmp(bgGangWarsTeams[i].refPtr, reference))
		{
			return i;
		}
	}
	return -1;
}

gangWarsTeam_t *JKG_GetTeamPtrByReference(char *reference)
{
	// This is rather expensive. Please do not call this every frame if entirely possible
	int i;
	for(i = 0; i < bgnumGangWarTeams; i++)
	{
		if(!Q_stricmp(bgGangWarsTeams[i].refPtr, reference))
		{
			return &bgGangWarsTeams[i];
		}
	}
	return NULL;
}