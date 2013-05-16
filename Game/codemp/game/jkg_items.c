// Copyright (C) 2011 Jedi Knight Galaxies
// bg_items.c : handles looting init, looting, item init, and treasure spawning procedures
// File by eezstreet

#include "jkg_items.h"
#include "g_local.h"
#include <json/cJSON.h>

#define MAX_ITEM_FILE_LENGTH (16384)
#define MAX_LOOT_FILE_LENGTH (16384)

itemData_t itemLookupTable[MAX_ITEM_TABLE_SIZE];
lootTable_t lootLookupTable[MAX_LOOT_TABLE_SIZE];

//itemInstance_t BG_GenerateItem ( itemInstance_t *instance, unsigned int id, unsigned int quality, qboolean loot)

itemData_t *JKG_GetItemByWeaponIndex ( int weaponIndex )
{
    int i;
    for ( i = 0; i < MAX_ITEM_TABLE_SIZE; i++ )
    {
        itemData_t *item = &itemLookupTable[i];
        if ( item->itemID && item->varID == weaponIndex )
        {
            return item;
        }
    }
    
    return NULL;
}

static qboolean JKG_ParseItem ( const char *itemFilePath, itemData_t *itemData )
{
	cJSON *json = NULL;
	cJSON *jsonNode = NULL;

	char error[MAX_STRING_CHARS];
	const char *str = NULL;
	int	item, i;

	char itemFileData[MAX_ITEM_FILE_LENGTH];
	fileHandle_t f;
	int fileLen = strap_FS_FOpenFile (itemFilePath, &f, FS_READ);

	if ( !f || fileLen == -1 )
	{
		Com_Printf (S_COLOR_RED "Unreadable or empty item file %s\n", itemFilePath);
		return qfalse;
	}

	if ( (fileLen + 1) >= MAX_ITEM_FILE_LENGTH )
	{
		strap_FS_FCloseFile(f);
		Com_Printf (S_COLOR_RED "%s item file too large\n", itemFilePath);
		return qfalse;
	}

	strap_FS_Read (&itemFileData, fileLen, f);
	itemFileData[fileLen] = '\0';

	strap_FS_FCloseFile (f);

	json = cJSON_ParsePooled (itemFileData, error, sizeof (error));
	if ( json == NULL )
	{
		Com_Printf (S_COLOR_RED "%s: %s\n", itemFilePath, error);
        return qfalse;
	}

	//Basic Item Information
	jsonNode = cJSON_GetObjectItem (json, "name");
	str = cJSON_ToString (jsonNode);
	strcpy(itemData->displayName, str);

	jsonNode = cJSON_GetObjectItem (json, "internal");
	str = cJSON_ToString (jsonNode);
	strcpy(itemData->internalName, str);

	jsonNode = cJSON_GetObjectItem (json, "id");
	item = cJSON_ToNumber (jsonNode);
	itemData->itemID = item;

	jsonNode = cJSON_GetObjectItem (json, "itemtype");
	str = cJSON_ToString (jsonNode);
	if ( Q_stricmp (str, "armor") == 0 )
	    itemData->itemType = ITEM_ARMOR;
	else if ( Q_stricmp (str, "weapon") == 0 )
	    itemData->itemType = ITEM_WEAPON;
	else if ( Q_stricmp (str, "clothing") == 0 )
	    itemData->itemType = ITEM_CLOTHING;
	else
	    itemData->itemType = ITEM_UNKNOWN;

	jsonNode = cJSON_GetObjectItem (json, "weight");
	item = cJSON_ToNumber (jsonNode);
	itemData->weight = item;

	//pSpell Data

	for(i = 0; i < MAX_PSPELLS; i++){
		if(i == 0)
			jsonNode = cJSON_GetObjectItem (json, "pSpell");
		else
			jsonNode = cJSON_GetObjectItem (json, va("pSpell%i", i));
		item = cJSON_ToNumber (jsonNode);
		if(!item)
			item = -1;
		itemData->pSpell[i] = item;

		if(i == 0)
			jsonNode = cJSON_GetObjectItem (json, "affector");
		else
			jsonNode = cJSON_GetObjectItem (json, va("affector%i", i));
		item = cJSON_ToNumber (jsonNode);
		itemData->affector[i] = item;

		if(i == 0)
			jsonNode = cJSON_GetObjectItem (json, "amount");
		else
			jsonNode = cJSON_GetObjectItem (json, va("amount%i", i));
		item = cJSON_ToNumber (jsonNode);
		itemData->amountBase[i] = item;

		if(i == 0)
			jsonNode = cJSON_GetObjectItem (json, "dur");
		else
			jsonNode = cJSON_GetObjectItem (json, va("dur%i", i));
		item = cJSON_ToNumber (jsonNode);
		itemData->duration[i] = item;

		if(i == 0)
			jsonNode = cJSON_GetObjectItem (json, "cnt");
		else
			jsonNode = cJSON_GetObjectItem (json, va("cnt%i", i));
		item = cJSON_ToNumber (jsonNode);
		if(item > 1)
			item = 1;
		else if(item < 0)
			item = 0;
		itemData->cntDown[i] = item;
	}

	//Equipment Info
	if(itemData->itemType == ITEM_WEAPON) {
			//This is a weapon. Grab the data.
			jsonNode = cJSON_GetObjectItem(json, "weapon");
			item = cJSON_ToNumber(jsonNode);
			itemData->weapon = item;

			jsonNode = cJSON_GetObjectItem(json, "variation");
			item = cJSON_ToNumber(jsonNode);
			itemData->variation = item;

			itemData->varID = BG_GetWeaponIndex(itemData->weapon, itemData->variation);
	} else if(itemData->itemType == ITEM_ARMOR) {
			//This is an armor piece. Grab the data.
			const char *armorSlot;
			jsonNode = cJSON_GetObjectItem(json, "armorID");
			item = cJSON_ToNumber(jsonNode);
			itemData->armorID = item;

			jsonNode = cJSON_GetObjectItem(json, "armorSlot");
			armorSlot = cJSON_ToString(jsonNode);

			if(!Q_stricmp(armorSlot, "head")){
				itemData->armorSlot = ARMSLOT_HEAD;
			} else if(!Q_stricmp(armorSlot, "neck")){
				itemData->armorSlot = ARMSLOT_NECK;
			} else if(!Q_stricmp(armorSlot, "body") || !Q_stricmp(armorSlot, "torso")){
				itemData->armorSlot = ARMSLOT_TORSO;
			} else if(!Q_stricmp(armorSlot, "robe")){
				itemData->armorSlot = ARMSLOT_ROBE;
			} else if(!Q_stricmp(armorSlot, "legs")){
				itemData->armorSlot = ARMSLOT_LEGS;
			} else if(!Q_stricmp(armorSlot, "hands") || !Q_stricmp(armorSlot, "hand") || !Q_stricmp(armorSlot, "gloves")){
				itemData->armorSlot = ARMSLOT_GLOVES;
			} else if(!Q_stricmp(armorSlot, "boots") || !Q_stricmp(armorSlot, "foot") || !Q_stricmp(armorSlot, "feet")){
				itemData->armorSlot = ARMSLOT_BOOTS;
			} else if(!Q_stricmp(armorSlot, "shoulder") || !Q_stricmp(armorSlot, "pauldron") || !Q_stricmp(armorSlot, "pauldrons")){
				itemData->armorSlot = ARMSLOT_SHOULDER;
			} else if(!Q_stricmp(armorSlot, "implant") || !Q_stricmp(armorSlot, "implants")){
				itemData->armorSlot = ARMSLOT_IMPLANTS;
			}

			//Armor Type

			//Light armor drains less force power and has no reduction to speed.
			//Medium armor has a reduction to speed equivalent to %(Defense - Weight) / # medium/heavy armor pieces equipped
			//Heavy armor has a chance to completely negate damage equal to its defense - weight / # heavy armor pieces equipped. (Capped at 15%)
			//Also, it reduces speed equivalent to %(Defense - Weight) / # heavy armor pieces equipped.
			//Note that damage negation only applies to the limb that corresponds to its slot.

			jsonNode = cJSON_GetObjectItem(json, "armorType");
			str = cJSON_ToString (jsonNode);

			if(Q_stricmp(str, "light") == 0)
				itemData->armorType = ARMTYPE_LIGHT;
			else if(Q_stricmp(str, "medium") == 0)
				itemData->armorType = ARMTYPE_MEDIUM;
			else if(Q_stricmp(str, "heavy") == 0)
				itemData->armorType = ARMTYPE_HEAVY;
			else
				itemData->armorType = ARMTYPE_MEDIUM;

			jsonNode = cJSON_GetObjectItem(json, "baseDefense");
			itemData->baseDefense = cJSON_ToNumber(jsonNode);

	}
	if(itemData->itemType == ITEM_ARMOR || itemData->itemType == ITEM_WEAPON)
	{
		jsonNode = cJSON_GetObjectItem(json, "maxDurability");
		itemData->baseDurabilityMax = cJSON_ToNumber(jsonNode);
		jsonNode = cJSON_GetObjectItem(json, "averageDurability");
		itemData->averageDurability = cJSON_ToNumber(jsonNode);
	}
	
	cJSON_Delete (json);

	return qtrue;
}

static qboolean JKG_LoadItems ( void )
{
	int i, j;
	char itemFiles[8192];
	int numFiles = strap_FS_GetFileList ("ext_data/items/", ".itm", itemFiles, sizeof (itemFiles));
	const char *itemFile = itemFiles;
	int successful = 0;
	int failed = 0;

	Com_Printf ("------- Constructing Item Table -------\n");
	
	for ( i = 0; i < numFiles; i++ )
	{
	    itemData_t dummy;
		if ( !JKG_ParseItem(va ("ext_data/items/%s", itemFile), &dummy) )
		{
		    failed++;
		    continue;
		}

		if(dummy.itemID > 0 && dummy.itemID < MAX_ITEM_TABLE_SIZE )
		{
			successful++;
		}
		else
		{
			failed++;
			continue;
		}
		
		if(dummy.itemID >= MAX_ITEM_TABLE_SIZE){
			Com_Printf (S_COLOR_RED "ERROR: item ID out of range \nItem: ext_data/items/%s, ID: %d\n", itemFile, dummy.itemID);
			Com_Printf (S_COLOR_RED "Attempting to correct this now...\n");
			for( j = 1; j < MAX_ITEM_TABLE_SIZE; j++ )
			{
				if(!itemLookupTable[j].itemID)
				{
					dummy.itemID = j;
					Com_Printf (S_COLOR_YELLOW "New itemID: %d\n", dummy.itemID);
					break;
				}
			}
		}

		if(itemLookupTable[dummy.itemID].itemID)
			Com_Printf (S_COLOR_YELLOW "Duplicate item id: %d", dummy.itemID);
		itemLookupTable[dummy.itemID] = dummy;

		itemFile += strlen (itemFile) + 1;
	}
	Com_Printf ("Item Table: %d successful, %d failed.\n", successful, failed);
	Com_Printf ("-------------------------------------\n");

	return (qboolean)(successful > 0);
}

static qboolean JKG_ParseLootTable ( const char *lootFilePath, lootTable_t *loot )
{
	char error[MAX_STRING_CHARS];
	int	i, lootSize;
	cJSON *json = NULL;
	cJSON *jsonNode = NULL;
	cJSON *lootNode = NULL;

	char lootFileData[MAX_LOOT_FILE_LENGTH];
	fileHandle_t f;
	int fileLen = strap_FS_FOpenFile (lootFilePath, &f, FS_READ);

	if ( !f || fileLen == -1 )
	{
		Com_Printf (S_COLOR_RED "Unreadable or empty loot file %s\n", lootFilePath);
		return qfalse;
	}

	if ( (fileLen + 1) >= MAX_LOOT_FILE_LENGTH )
	{
		strap_FS_FCloseFile(f);
		Com_Printf (S_COLOR_RED "%s loot file too large\n", lootFilePath);
		return qfalse;
	}

	strap_FS_Read (&lootFileData, fileLen, f);
	lootFileData[fileLen] = '\0';

	strap_FS_FCloseFile (f);

	json = cJSON_ParsePooled (lootFileData, error, sizeof (error));
	if(json == NULL)
	{
		Com_Printf (S_COLOR_RED "%s: %s\n", lootFilePath, error);
		return qfalse;
	}
	

	jsonNode = cJSON_GetObjectItem(json, "lootArray");
	lootSize = cJSON_GetArraySize(jsonNode);
	if(lootSize > MAX_LOOT_ITEMS)
	{
		Com_Printf (S_COLOR_RED "Too many items in loot file %s\n", lootFilePath);
		return qfalse;
	}
	for(i = 0; i < lootSize; i++)
	{
		lootNode = cJSON_GetArrayItem(jsonNode, i);
		loot->lootItemID[i] = cJSON_ToInteger(cJSON_GetObjectItem(lootNode, "id"));
		loot->lootItemChance[i] = cJSON_ToInteger(cJSON_GetObjectItem(lootNode, "chance"));
	}

	loot->lootID = cJSON_ToInteger(cJSON_GetObjectItem(json, "lootID"));
	loot->numItems = lootSize;

	//lootLookupTable[loot->lootID] = *loot;
	cJSON_Delete (json);

	return qtrue;
}

static qboolean JKG_LoadLootTable ( void )
{
	int i;
	char lootFiles[8192];
	int numFiles = strap_FS_GetFileList ("ext_data/loot", ".loot", lootFiles, sizeof (lootFiles));
	const char *lootFile = lootFiles;
	int successful = 0;
	int failed = 0;

	Com_Printf ("------- Constructing Loot Table -------\n");

	for ( i = 0; i < numFiles; i++ )
	{
	    lootTable_t dummy;
		if ( !JKG_ParseLootTable(va("ext_data/loot/%s", lootFile), &dummy) )
		{
		    failed++;
		    continue;
		}

		if(dummy.lootID && dummy.lootItemChance[0] && dummy.lootItemID[0] && dummy.lootID < MAX_LOOT_TABLE_SIZE)
		{
			if(dummy.lootID < MAX_LOOT_TABLE_SIZE)
				successful++;
		}
		else
		{
			failed++;
			continue;
		}

		if(lootLookupTable[dummy.lootID].lootID)
			Com_Printf (S_COLOR_YELLOW "Duplicate loot id: %d", dummy.lootID);

		lootLookupTable[dummy.lootID] = dummy;
		lootFile += strlen(lootFile) + 1;
	}

	Com_Printf ("Loot Table: %d successful, %d failed.\n", successful, failed);
	Com_Printf ("-------------------------------------\n");

	return (qboolean)(successful > 0);
}

static void JKG_CalculateDefense(itemInstance_t *item)
{
	item->defense = item->id->baseDefense + Q_irand(-2,2);
}

static void JKG_CalculateDurability(itemInstance_t *item)
{
	//item->durabilityCurrent = (item->id->averageDurability == item->id->baseDurabilityMax ? item->id->averageDurability : \
	//	(item->id->averageDurability > 1 ? item->id->averageDurability + Q_irand(-1,1) : item->id->averageDurability));

	int averageDurability = item->id->averageDurability;
	int durabilityMax = item->id->baseDurabilityMax;

	if(averageDurability < durabilityMax)
	{
		if(averageDurability > 2)
		{
			item->durabilityCurrent = averageDurability + Q_irand(-2,2);
			return;
		}
		else
		{
			goto setAsAverage;
		}
	}
	else if(averageDurability > durabilityMax)
	{
		item->durabilityCurrent = durabilityMax;
		return;
	}
setAsAverage:
	item->durabilityCurrent = averageDurability;
	return;

}

static itemInstance_t *JKG_RollItem( unsigned int index, int qualityOverride, itemInstance_t *item )
{
	//WARNING! This function may be unsafe!
	//Be sure to free() memory as needed!
	itemData_t *itemData = &itemLookupTable[index];
	int i;

	item->id = itemData;
	if(qualityOverride != -1)
	{
		item->itemQuality = qualityOverride;
	}
	else
	{
		//Do standard loot-QA here. No time to write this up.
		item->itemQuality = IQUAL_NORMAL;
	}

	//This will be calculated more precisely at a later date.
	for(i = 0; i < MAX_PSPELLS; i++)
		item->amount[i] = item->id->amountBase[i];
	item->calc1 = 0;
	item->calc2 = 0;
	item->equipped = qfalse;

	switch(itemData->itemType)
	{
		case ITEM_WEAPON:
			JKG_CalculateDurability(item);
			break;
		case ITEM_ARMOR:
			JKG_CalculateDurability(item);
			JKG_CalculateDefense(item);
			break;
	}

	return item;
}

static void JKG_A_BlankItemInstance( unsigned int inventoryIndex, inv_t *inventory )
{
	itemInstance_t item;
	int i;


	item.id = NULL;
	item.itemQuality = IQUAL_NORMAL;
	for(i = 0; i < MAX_PSPELLS; i++)
		item.amount[i] = 0;
	item.calc1 = 0;
	item.calc2 = 0;
	item.equipped = qfalse;

	inventory->items[inventoryIndex] = item;

	return;
}

extern void JKG_Easy_DIMA_Add(inv_t *inventory, itemInstance_t item);
void JKG_A_RollItem( unsigned int itemIndex, int qualityOverride, inv_t *inventory )
{
	itemInstance_t item;
	itemData_t *itemData = &itemLookupTable[itemIndex];
	int i;

	item.id = itemData;
	if(qualityOverride != -1)
	{
		item.itemQuality = qualityOverride;
	}
	else
		item.itemQuality = IQUAL_NORMAL;

	for(i = 0; i < MAX_PSPELLS; i++)
	{
		item.amount[i] = item.id->amountBase[i];
	}
	item.calc1 = 0;
	item.calc2 = 0;
	item.equipped = qfalse;
	JKG_Easy_DIMA_Add(inventory, item);
}

//Instead of returning an array, this function modifies an existing array.
void JKG_PickItemsClean( gentity_t *ent, lootTable_t *loot )
{
	int i, j = 0;

	for(i=0; i < loot->numItems; i++)
	{
		if(Q_irand(1,99) < loot->lootItemChance[i])
		{
			JKG_A_RollItem(loot->lootItemID[i], -1, ent->inventory);
			j++;
		} else {
			JKG_A_BlankItemInstance(j, ent->inventory);
		}
	}
}


void JKG_InitItems ( void )
{
	memset(itemLookupTable, 0, sizeof(itemLookupTable));
	memset(lootLookupTable, 0, sizeof(lootLookupTable));
	if(!JKG_LoadItems())
	{
		Com_Error (ERR_FATAL, "No master item table.");
		return;
	}
	if(!JKG_LoadLootTable())
	{
		Com_Error (ERR_FATAL, "No master loot table.");
		return;
	}
}

void JKG_StopLooting(int clientNum, gentity_t *targetEnt)
{
	trap_SendServerCommand(clientNum, "loot cl");
	targetEnt->currentLooter = NULL;
}
