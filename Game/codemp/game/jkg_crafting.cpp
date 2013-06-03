#include "jkg_items.h"
#include "jkg_easy_items.h"
#include "jkg_dynarrays.h"
#include "g_local.h"
#include <json/cJSON.h>
#include "jkg_crafting.h"

jkgArray_t CraftingRecipes[CRAFTING_MAX];
jkgArray_t FreestyleRecipes[CRAFTING_MAX];

/*QUAKED jkg_workbench (1 0 0) (0 0 0) (0 0 0)
"model"		change the model of the workbench, default is map_objects/jkg/common/workbench.md3
"type"		"workbench", "welding", "laboratory". default is "workbench"
*/
void craftingtable_use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	if(!activator || !self || activator->isAtWorkbench || self->genericValue5)
		return;
	self->genericValue5 = qtrue;
	switch(self->genericValue4)
	{
		case CRAFTING_WELDING:
			trap_SendServerCommand(activator->client->ps.clientNum, "itemCraftStart welding");
			break;
		case CRAFTING_CHEMICAL:
			trap_SendServerCommand(activator->client->ps.clientNum, "itemCraftStart chemical");
			break;
		case CRAFTING_WORKBENCH:
		default:
			trap_SendServerCommand(activator->client->ps.clientNum, "itemCraftStart workbench");
			break;
	}
	activator->isAtWorkbench = qtrue;
	activator->currentlyLooting = self;
	self->currentLooter = activator;
}
void craftingtable_leave( gentity_t *self )
{
	self->currentlyLooting->genericValue5 = qfalse;
	self->currentlyLooting->currentLooter = NULL;
	self->currentlyLooting = NULL;
	self->isAtWorkbench = qfalse;
}
void SP_jkg_workbench(gentity_t *ent)
{
	char dummy[512];

	VectorSet( ent->r.mins, -16, -16, 0 );
	VectorSet( ent->r.maxs, 16, 16, 40 );


	G_SpawnString("type", "workbench", (char **)dummy);

	if(atoi(dummy))
	{
		ent->genericValue4 = atoi(dummy);
	}
	else if(!Q_stricmp(dummy, "workbench"))
	{
		ent->genericValue4 = CRAFTING_WORKBENCH;
	}
	else if(!Q_stricmp(dummy, "laboratory"))
	{
		ent->genericValue4 = CRAFTING_CHEMICAL;
	}
	else if(!Q_stricmp(dummy, "welding"))
	{
		ent->genericValue4 = CRAFTING_WELDING;
	}
	else
		ent->genericValue4 = CRAFTING_WORKBENCH;

	if(!ent->model || !ent->model[0])
	{
		ent->model = "/models/items/a_pwr_converter.md3";
	}
	ent->s.modelindex = G_ModelIndex(ent->model);

	ent->s.eFlags = 0;
	ent->r.svFlags |= SVF_PLAYER_USABLE;
	ent->r.contents = CONTENTS_SOLID;
	ent->clipmask = MASK_SOLID;

	ent->use = craftingtable_use;

	trap_LinkEntity(ent);
}

/*
======================================================================
CRAFTING RECIPE PARSING
======================================================================
*/

qboolean JKG_Items_LoadCraftFile( const char *filePath, craftRecipe_t *craftData, const char * const bench, const char * const type )
{
	char error[MAX_STRING_CHARS];
	int i, j;

	cJSON *json = NULL;
	cJSON *jsonNode = NULL;
	
	char craftFileData[MAX_CRAFTING_FILE_SIZE];
	fileHandle_t f;
	int fileLen = strap_FS_FOpenFile(filePath, &f, FS_READ);

	if(!f || fileLen == -1)
	{
		Com_Printf(S_COLOR_RED "Unreadable or empty loot file: %s\n", filePath);
		return qfalse;
	}
	if( (fileLen + 1) >= MAX_CRAFTING_FILE_SIZE )
	{
		strap_FS_FCloseFile(f);
		Com_Printf(S_COLOR_RED "%s: File too large\n", filePath);
		return qfalse;
	}

	strap_FS_Read(&craftFileData, fileLen, f);
	craftFileData[fileLen] = '\0';

	strap_FS_FCloseFile(f);

	json = cJSON_ParsePooled(craftFileData, error, sizeof(error));
	if(json == NULL)
	{
		Com_Printf(S_COLOR_RED "%s:%s\n", filePath, error);
		return qfalse;
	}

	JKG_Array_Init(&craftData->input, sizeof(craftInput_t), 1);
	JKG_Array_Init(&craftData->output, sizeof(craftInput_t), 1);

	for(i = 0; i < MAX_INPUTS; i++)
	{
		int inputSize;
		cJSON *inputNode = NULL;
		jsonNode = cJSON_GetObjectItem(json, va("input%i", i));
		inputSize = cJSON_GetArraySize(jsonNode);
		for(j = 0; j < inputSize; j++)
		{
			craftInput_t dummy2;
			inputNode = cJSON_GetArrayItem(jsonNode, j);
			dummy2.internalName = cJSON_ToString(cJSON_GetObjectItem(inputNode, "internal"));
			dummy2.itemID = cJSON_ToInteger(cJSON_GetObjectItem(inputNode, "itemid"));
			dummy2.quantity = 1;
			dummy2.quantity = cJSON_ToInteger(cJSON_GetObjectItem(inputNode, "quantity"));
			JKG_Array_Add(&craftData->input, &dummy2);
		}
	}

	for(i = 0; i < MAX_INPUTS; i++)
	{
		int outputSize;
		cJSON *outputNode = NULL;
		jsonNode = cJSON_GetObjectItem(json, va("output%i", i));
		outputSize = cJSON_GetArraySize(jsonNode);
		for(j = 0; j < outputSize; j++)
		{
			craftInput_t dummy2;
			outputNode = cJSON_GetArrayItem(jsonNode, j);
			dummy2.internalName = cJSON_ToString(cJSON_GetObjectItem(outputNode, "internal"));
			dummy2.itemID = cJSON_ToInteger(cJSON_GetObjectItem(outputNode, "itemid"));
			dummy2.quantity = 1;
			dummy2.quantity = cJSON_ToInteger(cJSON_GetObjectItem(outputNode, "quantity"));
			JKG_Array_Add(&craftData->output, &dummy2);
		}
	}

	if(Q_stricmp(bench, "workbench"))
		craftData->craftBench = CRAFTING_WORKBENCH;
	else if(Q_stricmp(bench, "welding"))
		craftData->craftBench = CRAFTING_WELDING;
	else if(Q_stricmp(bench, "laboratory"))
		craftData->craftBench = CRAFTING_CHEMICAL;
	else
		craftData->craftBench = CRAFTING_WORKBENCH;

	craftData->freestyle = (Q_stricmp(type, "freestyle") == 0) ? qtrue : qfalse;
	craftData->group = (craftData->freestyle == qtrue) ? cJSON_ToInteger(cJSON_GetObjectItem(json, "group")) : -1;

	if(craftData->freestyle)
		JKG_Array_Add(&FreestyleRecipes[craftData->craftBench], craftData);
	else
		JKG_Array_Add(&CraftingRecipes[craftData->craftBench], craftData);

	cJSON_Delete(json);
	return qtrue;
}

void JKG_Items_LoadCraftingSection( const char * const bench, const char * const section )
{
	int i = 0;
	char craftFiles[16384];
	int numFiles = strap_FS_GetFileList(va("ext_data/crafting/%s/%s", bench, section), ".craft", craftFiles, sizeof(craftFiles));
	const char *craftFile = craftFiles;
	int successful = 0;
	int failed = 0;
	qboolean freestyle = (Q_stricmp(section, "freestyle") == 0) ? qtrue : qfalse;

	Com_Printf("Loading %s recipes for %s\n", bench, section);

	for( ; i < numFiles; i++ )
	{
		craftRecipe_t dummy;
		if(!JKG_Items_LoadCraftFile(va("ext_data/crafting/%s/%s/%s", bench, section, craftFile), &dummy, bench, section))
		{
			failed++;
			continue;
		}

		if(dummy.craftBench < CRAFTING_MAX && dummy.craftBench >= CRAFTING_WORKBENCH)
			successful++;
		else
		{
			failed++;
			continue;
		}

		craftFile += strlen(craftFile)+1;
	}

	Com_Printf("Result: %d successful, %d failed", successful, failed);
}

void JKG_Items_LoadCraftingSys( void )
{
	int i = 0;
	for(; i < CRAFTING_MAX; i++)
	{
		JKG_Array_Init(&CraftingRecipes[i], (unsigned int)sizeof(craftRecipe_t), 5);
		JKG_Array_Init(&FreestyleRecipes[i], (unsigned int)sizeof(craftRecipe_t), 5);
	}

	JKG_Items_LoadCraftingSection("workbench", "freestyle");
	JKG_Items_LoadCraftingSection("workbench", "crafting");
	JKG_Items_LoadCraftingSection("welding", "freestyle");
	JKG_Items_LoadCraftingSection("welding", "crafting");
	JKG_Items_LoadCraftingSection("laboratory", "freestyle");
	JKG_Items_LoadCraftingSection("laboratory", "crafting");
}

void JKG_Items_UnloadCraftingSys( void )
{
	int i = 0;
	for(; i < CRAFTING_MAX; i++)
	{
		JKG_Array_Free(&CraftingRecipes[i]);
		JKG_Array_Free(&FreestyleRecipes[i]);
	}
}

void JKG_Items_CraftRecipe( gentity_t *ent, craftRecipe_t *craftData )
{
	qboolean outputIsNotItem = qfalse;
	craftInput_t **inputArray;
	craftInput_t **outputArray;
	unsigned int i;
	unsigned int *matsWeHave;
	if(!craftData)
		return;

	inputArray = (craftInput_t **)craftData->input.data;
	outputArray = (craftInput_t **)craftData->output.data;
	matsWeHave = (unsigned int *)malloc(craftData->input.size * sizeof(unsigned int));

	if(!ent->isAtWorkbench)
	{
		free(matsWeHave);
		trap_SendServerCommand(ent->client->ps.clientNum, "print \"^1You need a medium to craft on.\n\"");
		return;
	}
	
	if(craftData->craftBench != ent->currentlyLooting->genericValue4)
	{
		free(matsWeHave);
		trap_SendServerCommand(ent->client->ps.clientNum, "print \"^1You can't craft that recipe here.\n\"");
		return;
	}

	for(i=0; i < craftData->input.size; i++)
	{
		//Compare internals first. They always take prevalence.
		int inventoryID = (JKG_Easy_DIMA_CMPInternal(ent->inventory, (char *)inputArray[i]->internalName) > -1) ? JKG_Easy_DIMA_CMPInternal(ent->inventory, (char *)inputArray[i]->internalName) : JKG_Easy_DIMA_CMPItemID(ent->inventory, inputArray[i]->itemID);
		if(inventoryID == -1)
		{ // :(
			free(matsWeHave);
			trap_SendServerCommand(ent->client->ps.clientNum, "print \"^1You do not have the materials required to make this.\n\"");
			return;
		}
		matsWeHave[i] = inventoryID;
	}
	//Found all mats. Now delete them out of our inventory, one by one.
	for(i=0; i < craftData->input.size; i++)
	{
		JKG_Easy_DIMA_Remove(ent->inventory, i);
	}
	//Now...create the output.
	for(i = 0; i < craftData->output.size; i++)
	{
		int quantity = outputArray[i]->quantity;
		unsigned int outputID = (!outputArray[i]->itemID) ? JKG_Easy_GetItemIDFromInternal(outputArray[i]->internalName) : outputArray[i]->itemID;
		int j = 0;
		for(; j < quantity; j++)
		{
			JKG_A_RollItem(outputID, 0, ent->inventory);
		}
	}

	free(matsWeHave);
}

void JKG_Items_CraftFreestyle( gentity_t *ent, int **mats )
{
}