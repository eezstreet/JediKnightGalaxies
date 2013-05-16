#include <json/cJSON.h>
#include "bg_ammo.h"
#include "bg_strap.h"

#ifdef CGAME
#include "../cgame/cg_local.h"
#endif

ammo_t ammoTable[JKG_MAX_AMMO_INDICES];
static unsigned int numAmmoLoaded = 0;
static ammo_t defaultAmmo;

// TODO: Maybe change to a hash table? It's only happening once at initialization though
// so it shouldn't be too much of a problem.
const ammo_t *BG_GetAmmo ( const char *ammoName )
{
    static const ammo_t *prevAmmo = NULL;
    const ammo_t *ammo = &ammoTable[0];
    int i;
    if ( Q_stricmp (ammoName, prevAmmo->name) == 0 )
    {
        return prevAmmo;
    }
    
    for ( i = 0; i < JKG_MAX_AMMO_INDICES; i++, ammo++ )
    {
        if ( Q_stricmp (ammo->name, ammoName) == 0 )
        {
            prevAmmo = ammo;
            return ammo;
        }
    }
    
    Com_Printf (S_COLOR_RED "Error: Failed to find ammo type \"%s\". Falling back to default ammo.\n", ammoName);
    return &defaultAmmo;
}

static void ParseAmmoFile ( const char *fileText )
{
	int i = 0;
    cJSON *json = NULL;
    char jsonError[MAX_STRING_CHARS] = { 0 };

    json = cJSON_ParsePooled (fileText, jsonError, sizeof (jsonError));
    if ( json == NULL )
    {
        Com_Printf (S_COLOR_RED "Error: %s\n", jsonError);
    }
    else
    {
        ammo_t *ammo = &ammoTable[0];
        cJSON *jsonNode;
        cJSON *field;
        const char *string = NULL;
        
        for ( jsonNode = cJSON_GetFirstItem (json); jsonNode; jsonNode = cJSON_GetNextItem (jsonNode), ammo++, numAmmoLoaded++, i++ )
        {
            field = cJSON_GetObjectItem (jsonNode, "name");
            string = cJSON_ToString (field);
			if(string && string[0])
				Q_strncpyz (ammo->name, string, sizeof (ammo->name));

			field = cJSON_GetObjectItem (jsonNode, "ammoMax");
			ammo->ammoMax = cJSON_ToNumber(field);

			ammo->ammoIndex = i;
            
            #ifdef CGAME
            {
                cJSON *visual = cJSON_GetObjectItem (jsonNode, "visual");
                if ( visual )
                {
                    cJSON *child = NULL;
                
                    field = cJSON_GetObjectItem (visual, "model");
                    string = cJSON_ToString (field);
                    if ( string && string[0] )
                    {
                        ammo->model = trap_R_RegisterModel (string);
                    }
                    
                    field = cJSON_GetObjectItem (visual, "fx");
                    string = cJSON_ToString (field);
                    if ( string && string[0] )
                    {
                        ammo->fx = trap_FX_RegisterEffect (string);
                    }
                    
                    field = cJSON_GetObjectItem (visual, "deathfx");
                    string = cJSON_ToString (field);
                    if ( string && string[0] )
                    {
                        ammo->deathFx = trap_FX_RegisterEffect (string);
                    }
                    
                    child = cJSON_GetObjectItem (visual, "miss");
                    if ( child )
                    {
                        field = cJSON_GetObjectItem (child, "impactfx");
                        string = cJSON_ToString (field);
                        if ( string && string[0] )
                        {
                            ammo->missFx = trap_FX_RegisterEffect (string);
                        }
                    }
                    
                    child = cJSON_GetObjectItem (visual, "hit");
                    if ( child )
                    {
                        field = cJSON_GetObjectItem (child, "impactfx");
                        string = cJSON_ToString (field);
                        if ( string && string[0] )
                        {
                            ammo->hitFx = trap_FX_RegisterEffect (string);
                        }
                    }
                }
            }
            #endif
        }
    }
    
    cJSON_Delete (json);
    
    Com_Printf ("Successfully loaded %d ammo types.\n", numAmmoLoaded);
}

#define MAX_AMMO_FILE_SIZE (16834) // 16kb
static qboolean LoadAmmo ( void )
{
    fileHandle_t f;
    char buffer[MAX_AMMO_FILE_SIZE + 1];
    int fileLength;
    
    Com_Printf ("------- Ammo Initialization -------\n");
    
    fileLength = strap_FS_FOpenFile ("ext_data/tables/ammo.json", &f, FS_READ);
    if ( fileLength == -1 || !f )
    {
        Com_Printf (S_COLOR_RED "Error: Failed to read the ammo.json file. File is unreadable or does not exist.\n");
        return qfalse;
    }
    
    if ( fileLength == 0 )
    {
        Com_Printf (S_COLOR_RED "Error: ammo.json file is empty.\n");
        strap_FS_FCloseFile (f);
        return qfalse;
    }
    
    if ( fileLength > MAX_AMMO_FILE_SIZE )
    {
        Com_Printf (S_COLOR_RED "Error: ammo.txt file is too large (max file size is %d bytes)\n", MAX_AMMO_FILE_SIZE);
        strap_FS_FCloseFile (f);
        return qfalse;
    }
    
    strap_FS_Read (buffer, fileLength, f);
    buffer[fileLength] = '\0';
    strap_FS_FCloseFile (f);
    
    ParseAmmoFile (buffer);
    
    Com_Printf ("-----------------------------------\n");
    
    return qtrue;
}

void BG_InitializeAmmo ( void )
{
    Q_strncpyz (defaultAmmo.name, "_defaultAmmo", sizeof (defaultAmmo.name));
    defaultAmmo.ammoMax = 100;
    
    if ( !LoadAmmo() )
    {
        Com_Error (ERR_DISCONNECT, "No ammo data file not found.");
        return;
    }
}