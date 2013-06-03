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
vendorStruct_t *vendorLookupTable[32];

static int lastUsedVendorID;

extern void NPC_ConversationAnimation(gentity_t *NPC);

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

stringID_table_t WPTable[]; // From bg_saga.c

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

	itemData->baseCost = 100;
	jsonNode = cJSON_GetObjectItem (json, "cost");
	item = cJSON_ToNumber (jsonNode);
	if(item > 0)
		itemData->baseCost = item;

	memset(itemData->pSpell, -1, (sizeof(signed int)*MAX_PSPELLS));

	//pSpell Data

	for(i = 0; i < MAX_PSPELLS; i++){
		if(i == 0)
			jsonNode = cJSON_GetObjectItem (json, "pSpell");
		else
			jsonNode = cJSON_GetObjectItem (json, va("pSpell%i", i+1));
		if(!jsonNode)
		{
			continue;
		}
		item = cJSON_ToNumber (jsonNode);
		itemData->pSpell[i] = item;

		if(i == 0)
			jsonNode = cJSON_GetObjectItem (json, "affector");
		else
			jsonNode = cJSON_GetObjectItem (json, va("affector%i", i+1));
		if(itemData->pSpell[i] != PSPELL_PLAYSOUND)
		{
			item = cJSON_ToNumber (jsonNode);
			itemData->affector[i] = item;
		}
		else
		{
			// Some affectors have a special setup involved.
			switch( itemData->pSpell[i] )
			{
				case PSPELL_PLAYSOUND:
					itemData->affector[i] = G_SoundIndex(cJSON_ToString(jsonNode));
					break;
			}
		}

		if(i == 0)
			jsonNode = cJSON_GetObjectItem (json, "amount");
		else
			jsonNode = cJSON_GetObjectItem (json, va("amount%i", i+1));
		item = cJSON_ToNumber (jsonNode);
		itemData->amountBase[i] = item;

		if(i == 0)
			jsonNode = cJSON_GetObjectItem (json, "dur");
		else
			jsonNode = cJSON_GetObjectItem (json, va("dur%i", i+1));
		item = cJSON_ToNumber (jsonNode);
		itemData->duration[i] = item;

		if(i == 0)
			jsonNode = cJSON_GetObjectItem (json, "cnt");
		else
			jsonNode = cJSON_GetObjectItem (json, va("cnt%i", i+1));
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
			str = cJSON_ToString (jsonNode);
			if(!atoi(str))
			{
				itemData->weapon = GetIDForString (WPTable, str);
			}
			else
			{
				itemData->weapon = atoi(str);
			}

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
			Com_Printf (S_COLOR_YELLOW "Duplicate item id: %d\n", dummy.itemID);
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

void JKG_A_GiveEntItem( unsigned int itemIndex, int qualityOverride, inv_t *inventory, gclient_t *owner )
{
	itemInstance_t item;
	itemData_t *itemData = &itemLookupTable[itemIndex];
	int invNum = 0;
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

	//Find inventory ID for the pInv call
	invNum = inventory->elements;

	//Add a DIMA instance
	JKG_Easy_DIMA_Add(inventory, item);

	//Perform the pInv call to network to clients
	trap_SendServerCommand(owner->ps.clientNum, va("pInv add %i %i %i %i %i %i %i %i %i %i %i %i %i %i",
		invNum, item.id->itemID, item.itemQuality, item.amount[0], item.amount[1], item.amount[2],
		        item.amount[3], item.amount[4], item.amount[5], item.amount[6], item.amount[7],
		        item.amount[8], item.amount[9], item.equipped));
	owner->coreStats.weight += itemData->weight;
}

void JKG_A_GiveEntItemForcedToACI( unsigned int itemIndex, int qualityOverride, inv_t *inventory, gclient_t *owner, unsigned int ACIslot )
{
	itemInstance_t item;
	itemData_t *itemData = &itemLookupTable[itemIndex];
	int invNum = 0;
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

	//Find inventory ID for the pInv call
	invNum = inventory->elements;

	//Add a DIMA instance
	JKG_Easy_DIMA_Add(inventory, item);

	//Perform the pInv call to network to clients
	trap_SendServerCommand(owner->ps.clientNum, va("pInv addfrc %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i",
		invNum, item.id->itemID, ACIslot, item.itemQuality, item.amount[0], item.amount[1], item.amount[2],
		        item.amount[3], item.amount[4], item.amount[5], item.amount[6], item.amount[7],
		        item.amount[8], item.amount[9], item.equipped));
	owner->coreStats.weight += itemData->weight;
}

//Instead of returning an array, this function modifies an existing array.
extern void JKG_Easy_DIMA_Init(inv_t *inventory);
void JKG_PickItemsClean( gentity_t *ent, lootTable_t *loot )
{
	int i, j = 0;

	if(!ent->inventory)
	{
		ent->inventory = (inv_t *)malloc(sizeof(inv_t));
		JKG_Easy_DIMA_Init(ent->inventory);
	}

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

static randomItemStruct_t randomStock;
#define MAX_RANDOM_VENDOR_BUFFER_SIZE	4096
static void JKG_ParseRandomVendorFile(const char *fileName)
{
	//Generates the randomStock based on the fileName.
	//In phases 1-2, this is very minimal
	//In phase 3, this sorts the database more towards ilvls and stuff
	//In phase 4, this uses SQL
	int len;
	int i = 0;
	char buffer[MAX_RANDOM_VENDOR_BUFFER_SIZE];
	char temp[MAX_RANDOM_VENDOR_BUFFER_SIZE];
	fileHandle_t f;

	len = trap_FS_FOpenFile(fileName, &f, FS_READ);
	if(!len || !f)
	{
		Com_Printf("No random vendor file found. Vendors will not work.\n");
		if(!len)
		{
			trap_FS_FCloseFile(f);
		}
		return;
	}
	if(len >= MAX_RANDOM_VENDOR_BUFFER_SIZE)
	{
		Com_Printf("Random vendor file >= MAX_RANDOM_VENDOR_BUFFER_SIZE. Vendors will not work.\n");
		trap_FS_FCloseFile(f);
		return;
	}

	trap_FS_Read(buffer, len, f);
	trap_FS_FCloseFile(f);
	buffer[len] = '\0';

	//Now we parse the file
	//First line specifies how many items the vendor will put up for sale at any one time
	while(buffer[i] != '\n' && buffer[i] != '\r')
	{
		if(buffer[i] == '\0')
		{
			return; //New lines are required!
		}
		i++;
	}
	i++;
	Q_strncpyz(temp, buffer, i);
	randomStock.numPickedItems = atoi(temp);

	if(buffer[i] == '\n' || buffer[i] == '\r')
	{
		i++;
	}
	//Next line is a bit trickier
	{
		int offset = i;
		int offset2 = 0;
		qboolean comma = qfalse;
		qboolean nextIsWeapon = qfalse;
		qboolean nextIsItem = qfalse;
		qboolean closingQuote = qfalse;
		qboolean dashOkay = qfalse;
		qboolean handleDash = qfalse;
		char fieldBuffer[16];
		char valueBuffer[64];
		while(buffer[i] != '\n' && buffer[i] != '\r' && buffer[i] != '\0')
		{
			if(comma)
			{
				offset = i;
				comma = qfalse;
				i++;
				continue;
			}
			if(buffer[i] == ',')
			{
				if(closingQuote)
				{
					Com_Printf(" *** ERROR PARSING %s *** : Found ,, expected \"\n", fileName);
					return;
				}
				dashOkay=qfalse;
				if(handleDash)
				{
					Q_strncpyz(valueBuffer, buffer+offset, i-offset+1);
					randomStock.rarity[randomStock.numItems-1] = atoi(valueBuffer);
					handleDash = qfalse;
				}
				nextIsWeapon = qfalse;
				nextIsItem = qfalse;
				comma = qtrue;
				i++;
				continue;
			}
			else if(buffer[i] == '-' && dashOkay)
			{
				//Next field deals with dashes. These state the rarity
				handleDash = qtrue;
				offset = i+1;
			}
			else if(buffer[i] == ':')
			{
				if(closingQuote)
				{
					Com_Printf(" *** ERROR PARSING %s *** : Found :, expected \"\n", fileName);
					return;
				}
				if(nextIsWeapon || nextIsItem)
				{
					Com_Printf(" *** ERROR PARSING %s *** : Found :, expected ,\n", fileName);
					return;
				}
				//Grab field name
				Q_strncpyz(fieldBuffer, buffer+offset, i-offset+1);				//Compare field names
				if(!Q_stricmp(fieldBuffer, "weapon"))
				{
					//The next field will be a weapon
					nextIsWeapon = qtrue;
				}
				else if(!Q_stricmp(fieldBuffer, "item"))
				{
					//The next field will be an item
					nextIsItem = qtrue;
				}
			}
			else if(buffer[i] == '"')
			{
				if(!closingQuote)
				{
					//Next one will be a closing quote
					closingQuote = qtrue;
					offset2 = i+1;
				}
				else
				{
					//Grab the contents of our previous field
					dashOkay=qtrue;
					closingQuote = qfalse;
					Q_strncpyz(valueBuffer, buffer+offset2, (i-offset2+1));
					//Check our type
					if(nextIsItem)
					{
						//OK, add an item to our stock
						randomStock.itemId[randomStock.numItems] = atoi(valueBuffer);
						randomStock.rarity[randomStock.numItems] = 1;
						randomStock.numItems++;
					}
					else if(nextIsWeapon)
					{
						//Add a weapon to our stock
						weaponData_t *weapon = BG_GetWeaponByClassName (valueBuffer);
						itemData_t *item = JKG_GetItemByWeaponIndex(BG_GetWeaponIndex((unsigned int)weapon->weaponBaseIndex, (unsigned int)weapon->weaponModIndex));
						int itemID = 0;
						if(!weapon)
						{
							Com_Error(ERR_FATAL, "Could not find weapon from class name in random vendor file: %s", valueBuffer);
							return;
						}
						else
						{
							if(!item)
							{
								Com_Error(ERR_FATAL, "Weapon in vendor file (%s) must have associated item file!", valueBuffer);
								return;
							}
							else
							{
								//OK ALL FINE K
								itemID = item->itemID;
								randomStock.itemId[randomStock.numItems] = itemID;
								randomStock.rarity[randomStock.numItems] = 1;
								randomStock.numItems++;
							}
						}
					}
					//Unknown?
					else
					{
						Com_Printf("*** ERROR PARSING %s *** Unknown value for field\n", fileName);
					}
				}
			}
			i++;
		}
	}
}

void JKG_VendorInit(void)
{
	lastUsedVendorID = 0;
	level.lastVendorCheck = 0;
	level.lastVendorUpdateTime = 0;
	level.lastUpdatedVendor = -1;
	memset(&randomStock, 0, sizeof(randomItemStruct_t));
	JKG_ParseRandomVendorFile("ext_data/vendors.dat");
}

extern void JKG_target_vendor_use(gentity_t *ent, gentity_t *other, gentity_t *activator);

void JKG_CreateNewVendor(gentity_t *ent, int desiredVendorID, qboolean random, qboolean refreshStock)
{
	//Adds vendor properties to an NPC
	/*
	// UQ1: Moved to G_Damage with civies... I don't want them in this BS_
	if(ent->NPC)
	{
		//Protip: don't be an idiot and shoot at potential customers :P
		ent->NPC->defaultBehavior = BS_CINEMATIC;
		ent->NPC->behaviorState = BS_CINEMATIC;
	}
	*/
	if(desiredVendorID == -1)
	{
		//Use new vendor ID
		desiredVendorID = lastUsedVendorID;
		lastUsedVendorID++;
		refreshStock = qtrue;
	}
	else if(desiredVendorID >= 32)
	{
		Com_Printf("^1WARNING: Max vendor IDs (32) reached.\n");
		lastUsedVendorID = desiredVendorID = 0;
		refreshStock = qfalse;
	}

	// Give the ent god mode, etc
	ent->flags |= FL_GODMODE;
	ent->flags |= FL_NOTARGET;
	ent->flags |= FL_NO_KNOCKBACK;
	//FIXME: Getting a Error in this swift, npc files need a scripts on its own. --Stoiss
	switch (ent->client->NPC_class)
	{// UQ1: Need to change these in the actual NPC script files...
	case CLASS_GENERAL_VENDOR:
	case CLASS_WEAPONS_VENDOR:
	case CLASS_ARMOR_VENDOR:
	case CLASS_SUPPLIES_VENDOR:
	case CLASS_FOOD_VENDOR:
	case CLASS_MEDICAL_VENDOR:
	case CLASS_GAMBLER_VENDOR:
	case CLASS_TRADE_VENDOR:
	case CLASS_ODDITIES_VENDOR:
	case CLASS_DRUG_VENDOR:
	case CLASS_TRAVELLING_VENDOR:
		break;
	default:
		G_Printf("FIXME: NPC Vendor FORCED to CLASS_GENERAL_VENDOR. Vendors should have their own NPC file with a vendor class.\n");
		ent->client->NPC_class = CLASS_GENERAL_VENDOR;
		ent->s.NPC_class = CLASS_GENERAL_VENDOR;
		break;
	}

	ent->use = JKG_target_vendor_use; // why not? separate trigger seems a waste...
	ent->r.svFlags |= SVF_PLAYER_USABLE;
	ent->client->enemyTeam = NPCTEAM_NEUTRAL;
	ent->client->playerTeam = NPCTEAM_NEUTRAL;

	//refreshStock overrides random
	if(refreshStock && random)
	{
		//Generate random loot -- TODO: Fix this ugly, ugly mess
		int randomLoot = Q_irand(0, randomStock.numItems-1);
		qboolean alreadyUsed[256];		// capped at max number of weapons? or items? unsure.
		int i;
		int iterator = 0, numRare = 0;
		int rarityTable[2048];
		srand(time(NULL));
		ent->vendorData.sale = qfalse; //Can't be on sale when we start
		ent->vendorData.numItemsInStock = randomStock.numPickedItems;
		//Null some datas!
		memset(&ent->vendorData.itemsInStock, 0, sizeof(ent->vendorData.itemsInStock));
		memset(&ent->vendorData.itemsOnSale, 0, sizeof(ent->vendorData.itemsOnSale));
		memset(&ent->vendorData.priceReductions, 0, sizeof(ent->vendorData.priceReductions));
		memset(&alreadyUsed, 0, sizeof(alreadyUsed));
		//Build a rarity table
		for(i = 0; i < randomStock.numItems; i++)
		{
			int j;
			for(j = 0; j < randomStock.rarity[i]; j++)
			{
				rarityTable[numRare++] = i;
			}
		}
		i = 0;
#define MAX_ITERATIONS 320
		while(i < ent->vendorData.numItemsInStock && iterator < MAX_ITERATIONS)
		{
			int randNum, randRarity;
			iterator++;
			randRarity = (rand() % numRare); //Q_irand(0, numRare-1);
			randNum = randomStock.itemId[rarityTable[randRarity]];
			if(!alreadyUsed[rarityTable[randRarity]])
			{
				//Copy this item into our stock, because it's not used
				//ent->vendorData.itemsInStock[i] = randomStock.itemId[randRarity];
				ent->vendorData.itemsInStock[i] = randNum;
				alreadyUsed[rarityTable[randRarity]] = qtrue;
			}
			else
			{
				continue;
			}
			i++;
		}
		//Reset our number of items in stock to reflect how many items actually are in stock
		ent->vendorData.numItemsInStock = i;
		//Add us to the vendor lookup table
		ent->vendorData.ourID = desiredVendorID;
		vendorLookupTable[desiredVendorID] = &ent->vendorData;
		level.vendors[desiredVendorID] = ent->s.number;
		if(desiredVendorID == lastUsedVendorID)
		{
			lastUsedVendorID++;
		}
	}
	else if(!refreshStock)
	{
		ent->vendorData = *vendorLookupTable[desiredVendorID];
	}
	else
	{
		//Don't generate loot?? predetermined loot isn't available yet..
		return;
	}
}

void JKG_DeleteVendor(gentity_t *ent)
{
	//Removes vendor properties from an NPC
	//FIXME: Dunno if it will be in phase 1 yet..
}

void JKG_RefreshClientVendorStock( gentity_t *client, gentity_t *vendor )
{
	int i;
	char buffer[1024];
	
	if (g_entities[client->s.number].r.svFlags & SVF_BOT)
		return; // Bot's dont need this...

	/*if(client->NPC)
	{
		//Why should NPCs have their clientside updated?
		return;
	}*/
	strcpy(buffer, "shoprefresh");
	for(i = 0; i < vendor->vendorData.numItemsInStock; i++)
	{
		strcat(buffer, va(" %i", vendor->vendorData.itemsInStock[i]));
	}

	trap_SendServerCommand(client->client->ps.clientNum, buffer);
}

void JKG_RefreshVendorStockForAll( gentity_t *vendor )
{
	int i;
	char buffer[1024];

	strcpy(buffer, "shoprefresh");
	for(i = 0; i < vendor->vendorData.numItemsInStock; i++)
	{
		strcat(buffer, va(" %i", vendor->vendorData.itemsInStock[i]));
	}

	trap_SendServerCommand(-1, buffer);
}

extern gentity_t	*NPC;
extern gNPC_t		*NPCInfo;
extern usercmd_t	ucmd;
extern qboolean NPC_FaceEntity( gentity_t *ent, qboolean doPitch );
extern void G_SoundOnEnt( gentity_t *ent, soundChannel_t channel, const char *soundPath );
extern qboolean NPC_VendorHasConversationSounds(gentity_t *conversationalist);
extern qboolean NPC_VendorHasVendorSound(gentity_t *conversationalist, char *name);

void JKG_target_vendor_use(gentity_t *ent, gentity_t *other, gentity_t *activator)
{
	gentity_t		*vendorTarget = NULL;
	char			filename[256];
	//Checks

	if (activator->r.svFlags & SVF_BOT)
		return; // Bot's dont need this...

	if(!activator->client)
	{
		//G_Printf("VENDOR DEBUG: Vendor activator is not a client!\n");
		return;	//Check #1 - Is this a client?
	}

	if(activator->NPC)
	{
		//G_Printf("VENDOR DEBUG: Vendor has no NPC information!\n");
		return; //Check #2 - And not an NPC?
	}

	if (activator->client->ps.useDelay > level.time)
		return; // wait... dont spam shopopen!

	vendorTarget = G_Find(vendorTarget, FOFS(targetname), ent->target);

	if(!vendorTarget)
	{
		//G_Printf("VENDOR DEBUG: Vendor has no vendorTarget!\n");
		//return;
		// UQ1: Perfectly valid now - NPC's are now useable themselves (without a trigger ent)...
		vendorTarget = ent;
	}

	if(!vendorTarget->vendorData.numItemsInStock)
	{
		JKG_CreateNewVendor(vendorTarget, -1, qtrue, qtrue);
	}

	vendorTarget->flags |= FL_GODMODE;
	vendorTarget->flags |= FL_NOTARGET;
	vendorTarget->flags |= FL_NO_KNOCKBACK;

	// UQ1: Face the customer...
	NPC = vendorTarget;
	NPCInfo = NPC->NPC;
	ucmd = NPC->client->pers.cmd;
	NPC_FaceEntity( activator, qfalse );

	if (NPC_VendorHasVendorSound(vendorTarget, "welcome00"))
	{// This NPC has it's own vendor specific sound(s)...
		char	filename[256];
		int		max = 1;

		while (NPC_VendorHasVendorSound(vendorTarget, va("welcome0%i", max))) max++;

		strcpy(filename, va("sound/vendor/%s/welcome0%i.mp3", vendorTarget->NPC_type, irand(0, max-1)));
		NPC_ConversationAnimation(vendorTarget);
		G_SoundOnEnt( vendorTarget, CHAN_VOICE_ATTEN, filename );
	}
	else if (NPC_VendorHasConversationSounds(vendorTarget))
	{// Override with generic chat sounds for this specific NPC...
		strcpy(filename, va("sound/conversation/%s/conversation00.mp3", vendorTarget->NPC_type));
		NPC_ConversationAnimation(vendorTarget);
		G_SoundOnEnt( vendorTarget, CHAN_VOICE_ATTEN, filename );
	}
	else
	{// Use generic shop open sound (english)... Meh! Couldn't find any... Needs to be looked into...
		strcpy(filename, va("sound/vendor/generic/welcome0%i.mp3", irand(0,1)));
		NPC_ConversationAnimation(vendorTarget);
		G_SoundOnEnt( vendorTarget, CHAN_VOICE_ATTEN, filename );
	}

	//G_Printf("Shop opened.\n");
	//Actual meat of the function: activate the vendor menu for this client
	JKG_RefreshClientVendorStock(activator, vendorTarget);
	trap_SendServerCommand(activator->s.number, "shopopen");

	if (vendorTarget->client->NPC_class == CLASS_TRAVELLING_VENDOR)
		vendorTarget->NPC->walkDebounceTime = level.time + 60000; // UQ1: Wait 60 seconds before moving...

	activator->client->ps.useDelay = level.time + 500;
}

void JKG_SP_target_vendor(gentity_t *ent)
{
	gentity_t *targetVendor = NULL;
	ent->use = JKG_target_vendor_use;
	targetVendor = G_Find(targetVendor, FOFS(targetname), ent->target);

	if(targetVendor)
	{
		JKG_CreateNewVendor(targetVendor, -1, qtrue, qtrue);

		if (targetVendor->client->NPC_class == CLASS_TRAVELLING_VENDOR)
		{// Remove the trigger, it will be left behind...
			G_FreeEntity(ent);
		}
	}
}

void JKG_Vendor_Buy(gentity_t *ent, gentity_t *targetVendor, int item)
{
	//Okay. We first need to acquire the entity that the targetVendor is pointing at
	gentity_t *vendorEnt = G_Find(NULL, FOFS(targetname), targetVendor->target);
	int itemID = -1;
	int i;

	if(!vendorEnt)
	{
		//Not pointing at anything. ABORT! --eez
		//Com_Printf("^3WARNING: jkg_target_vendor with no target\n");
		//return;

		// UQ1: Perfectly valid now - NPC's are now useable themselves (without a trigger ent)...
		vendorEnt = targetVendor;
	}

	// K, the itemID is actually now the "item" arg, which should hopefully correct the little mistake of shops not giving out correct items --eez
	for(i = 0; i < vendorEnt->vendorData.numItemsInStock; i++)
	{
		if(vendorEnt->vendorData.itemsInStock[i] == item)
		{
			itemID = item;
			break;
		}
	}

	// itemID = vendorEnt->vendorData.itemsInStock[item];
	// alright, now let's do the check! --eez
	if( itemID == -1 )
	{
		//Com_Printf("^3WARNING: Invalid vendor item.\n");
		return;
	}

	if(!ent->client)
	{
		return;	// Not a client
	}

	if( ent->client->ps.stats[STAT_HEALTH] <= 0 )
	{
		return; // DEAD??
	}

	// No longer a valid check, matter of factually --eez
	/*if(item > vendorEnt->vendorData.numItemsInStock)
	{
		//Invalid item
		//Com_Printf("^3WARNING: Invalid vendor item.\n");
		//assert(0);
		return;
	}*/

	// UQ1: Face the customer...
	NPC = vendorEnt;
	NPCInfo = NPC->NPC;
	ucmd = NPC->client->pers.cmd;
	NPC_FaceEntity( ent, qfalse );

	if(ent->client->ps.persistant[PERS_CREDITS] < itemLookupTable[itemID].baseCost)	//TODO: add proper cost here
	{
		trap_SendServerCommand(ent->s.number, "print \"You do not have enough money for this item.\n\"");

		if (NPC_VendorHasVendorSound(vendorEnt, "purchasefail00"))
		{// This NPC has it's own vendor specific sound(s)...
			char	filename[256];
			int		max = 1;

			while (NPC_VendorHasVendorSound(vendorEnt, va("purchasefail0%i", max))) max++;

			strcpy(filename, va("sound/vendor/%s/purchasefail0%i.mp3", vendorEnt->NPC_type, irand(0, max-1)));
			NPC_ConversationAnimation(vendorEnt);
			G_SoundOnEnt( vendorEnt, CHAN_VOICE_ATTEN, filename );
		}
		else if (NPC_VendorHasConversationSounds(vendorEnt))
		{// Override with generic chat sounds for this specific NPC...
			char	filename[256];
			
			strcpy(filename, va("sound/conversation/%s/conversation02.mp3", vendorEnt->NPC_type));
			NPC_ConversationAnimation(vendorEnt);
			G_SoundOnEnt( vendorEnt, CHAN_VOICE_ATTEN, filename );
		}
		else
		{// Use generic shop buy sound...
			char	filename[256];

			strcpy(filename, va("sound/vendor/generic/purchasefail0%i.mp3", irand(0,5)));
			NPC_ConversationAnimation(vendorEnt);
			G_SoundOnEnt( vendorEnt, CHAN_VOICE_ATTEN, filename );
		}

		return;
	}

	//Okay, we've run through all the checks. Let's add the item to our inventory.
	{
		if(!itemLookupTable[itemID].itemID)
		{
			trap_SendServerCommand(ent->s.number, "print \"The item ID for this item is not valid.\n\"");
			return;	//itemID isn't valid
		}
		ent->client->ps.persistant[PERS_CREDITS] -= itemLookupTable[itemID].baseCost;	//TODO: add proper cost here
		JKG_A_GiveEntItem(itemID, IQUAL_NORMAL, ent->inventory, ent->client);
		// eez: TEMP: retrofitted shopupdate into a new servercommand which confirms the order and sends it to the ACI (if appropriate)
		// this is obviously temporary until we get a new shop setup. But for randomized shops, this works just fine.
		trap_SendServerCommand(ent->s.number, va("shopconfirm %i %i", ent->client->ps.persistant[PERS_CREDITS], itemID));
		
		//TODO: add sound
		if (NPC_VendorHasVendorSound(vendorEnt, "purchase00"))
		{// This NPC has it's own vendor specific sound(s)...
			char	filename[256];
			int		max = 1;

			while (NPC_VendorHasVendorSound(vendorEnt, va("purchase0%i", max))) max++;

			strcpy(filename, va("sound/vendor/%s/purchase0%i.mp3", vendorEnt->NPC_type, irand(0, max-1)));
			NPC_ConversationAnimation(vendorEnt);
			G_SoundOnEnt( vendorEnt, CHAN_VOICE_ATTEN, filename );
		}
		else if (NPC_VendorHasConversationSounds(vendorEnt))
		{// Override with generic chat sounds for this specific NPC...
			char	filename[256];
			
			strcpy(filename, va("sound/conversation/%s/conversation03.mp3", vendorEnt->NPC_type));
			NPC_ConversationAnimation(vendorEnt);
			G_SoundOnEnt( vendorEnt, CHAN_VOICE_ATTEN, filename );
		}
		else
		{// Use generic shop buy sound...
			char	filename[256];

			strcpy(filename, va("sound/vendor/generic/purchase0%i.mp3", irand(0,2)));
			NPC_ConversationAnimation(vendorEnt);
			G_SoundOnEnt( vendorEnt, CHAN_VOICE_ATTEN, filename );
		}

		// Add the ammo that we should get for this item
		if( itemLookupTable[itemID].itemType == ITEM_WEAPON )
		{
			weaponData_t *wep = GetWeaponData( itemLookupTable[itemID].weapon, itemLookupTable[itemID].variation );
			int weaponIndex = BG_GetWeaponIndex( itemLookupTable[itemID].weapon, itemLookupTable[itemID].variation );
			//if(wep->ammoIndex < MAX_WEAPONS)
			{
				if(ent->client->ammoTable[wep->ammoIndex] < wep->ammoOnPickup)
				{
					ent->client->ammoTable[wep->ammoIndex] = wep->ammoOnPickup;
				}
				ent->client->clipammo[weaponIndex] = GetWeaponAmmoClip( wep->weaponBaseIndex, wep->weaponModIndex );
			}
		}
	}
}

void JKG_RefreshVendorStock(gentity_t *vendor)
{
	JKG_CreateNewVendor(vendor, vendor->vendorData.ourID, qtrue, qtrue);
	JKG_RefreshVendorStockForAll(vendor);
}

// Okay, I promised I wouldn't make any special features for Versus, but this can be ported to an RPG setup with relative ease.
// Run this function every so often to ensure that vendors are replenished on demand.
void JKG_CheckVendorReplenish(void)
{
	if(level.lastVendorCheck < level.time - 300 && level.lastVendorUpdateTime < level.time - (jkg_shop_replenish_time.integer*1000))
	{	// Don't update all of the shops at once..do them in a rolling update.
		level.lastUpdatedVendor++;
		/*if(level.lastUpdatedVendor == 0)
		{
			// Uh..don't ask.
			level.lastUpdatedVendor++;
		}*/
		if(level.lastUpdatedVendor >= lastUsedVendorID)
		{
			// Hit the last of the vendor list
			level.lastVendorUpdateTime = level.lastVendorCheck = level.time;
			level.lastUpdatedVendor = -1;
			return;
		}
		// K...all should be fine. Issue an update for this targetted vendor.
		JKG_RefreshVendorStock(&g_entities[level.vendors[level.lastUpdatedVendor]]);
		level.lastVendorCheck = level.time;
	}
}