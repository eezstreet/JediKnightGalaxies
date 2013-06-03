#include "bg_strap.h"
#include "bg_public.h"
#include <json/cJSON.h>

bgConstants_t bgConstants;

#define MAX_CONSTANTS_FILE_SIZE (16834) // 16kb

static void DefineBaselineConstants(void)
{
	bgConstants.baseForceJumpLevel = 0;
	bgConstants.baseJumpHeight = 32;
	bgConstants.baseJumpTapHeight = 32;
	bgConstants.baseJumpVelocity = 225;
	bgConstants.baseJumpTapVelocity = 225;
	//Stoiss add, missing code from saber merge 
	bgConstants.walkingSpeed = 64.0f;
	bgConstants.ironsightsMoveSpeed = 64.0f;
	bgConstants.blockingModeMoveSpeed = 64.0f;

	bgConstants.backwardsSpeedModifier = -0.45f;
	bgConstants.strafeSpeedModifier = -0.25f;
	bgConstants.backwardsDiagonalSpeedModifier = 0.21;
	bgConstants.baseSpeedModifier = -0.1f;
	bgConstants.walkSpeedModifier = -0.2f;
	bgConstants.minimumSpeedModifier = 0.5f;
	bgConstants.sprintSpeedModifier = 1.3f;
	//Stoiss end
}

static void ParseConstantsFile ( const char *fileText )
{
	int i = 0;
    cJSON *json = NULL;
    char jsonError[MAX_STRING_CHARS] = { 0 };
	cJSON *jsonNode;
    const char *string = NULL;

    json = cJSON_ParsePooled (fileText, jsonError, sizeof (jsonError));
    if ( json == NULL )
    {
        Com_Printf (S_COLOR_RED "Error: %s\n", jsonError);
    }
	// Well, that's strange. It all loaded fine. Let's break all for a minute, and then check the values again.
	// Hm. All seems fine now. Let's go ingame. I need to unpause the game first.
    else
    {
		jsonNode = cJSON_GetObjectItem (json, "baseJumpHeight");
		bgConstants.baseJumpHeight = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "baseForceJumpLevel");
		bgConstants.baseForceJumpLevel = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "baseJumpVelocity");
		bgConstants.baseJumpVelocity = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "baseJumpTapHeight");
		bgConstants.baseJumpTapHeight = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "baseJumpTapVelocity");
		bgConstants.baseJumpTapVelocity = cJSON_ToNumber(jsonNode);
		//Stoiss add missing merge code from saber code
		jsonNode = cJSON_GetObjectItem (json, "walkingSpeed");
		bgConstants.walkingSpeed = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "ironsightsMoveSpeed");
		bgConstants.ironsightsMoveSpeed = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "blockingModeMoveSpeed");
		bgConstants.blockingModeMoveSpeed = cJSON_ToNumber(jsonNode);


		jsonNode = cJSON_GetObjectItem (json, "backwardsSpeedModifier");
		bgConstants.backwardsSpeedModifier = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "strafeSpeedModifier");
		bgConstants.strafeSpeedModifier = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "backwardsDiagonalSpeedModifier");
		bgConstants.backwardsDiagonalSpeedModifier = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "baseSpeedModifier");
		bgConstants.baseSpeedModifier = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "walkSpeedModifier");
		bgConstants.walkSpeedModifier = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "minimumSpeedModifier");
		bgConstants.minimumSpeedModifier = cJSON_ToNumber(jsonNode);

		jsonNode = cJSON_GetObjectItem (json, "sprintSpeedModifier");
		bgConstants.sprintSpeedModifier = cJSON_ToNumber(jsonNode);
		//Stoiss end
    }
    
    cJSON_Delete (json);
}

qboolean ReadConstantsFile(void)
{
	fileHandle_t f;
    char buffer[MAX_CONSTANTS_FILE_SIZE + 1];
    int fileLength;
    
    fileLength = strap_FS_FOpenFile ("ext_data/tables/constants.json", &f, FS_READ);
    if ( fileLength == -1 || !f )
    {
        Com_Printf (S_COLOR_RED "Error: Failed to read the constants.json file. File is unreadable or does not exist.\n");
        return qfalse;
    }
    
    if ( fileLength == 0 )
    {
        Com_Printf (S_COLOR_RED "Error: constants.json file is empty.\n");
        strap_FS_FCloseFile (f);
        return qfalse;
    }
    
    if ( fileLength > MAX_CONSTANTS_FILE_SIZE )
    {
        Com_Printf (S_COLOR_RED "Error: constants.json file is too large (max file size is %d bytes)\n", MAX_CONSTANTS_FILE_SIZE);
        strap_FS_FCloseFile (f);
        return qfalse;
    }
    
    strap_FS_Read (buffer, fileLength, f);
    buffer[fileLength] = '\0';
    strap_FS_FCloseFile (f);
    
    ParseConstantsFile (buffer);
    
    Com_Printf ("-----------------------------------\n");
    
    return qtrue;
}

void JKG_InitializeConstants(void)
{
	DefineBaselineConstants();
	if(!ReadConstantsFile())
	{
		//Com_Error(ERR_DISCONNECT, "Unable to parse the constants file.");
	}
}