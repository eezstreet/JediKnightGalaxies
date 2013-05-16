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