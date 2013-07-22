//Jedi Knight Galaxies (c) 2011
//File by eezstreet
//jkg_cg_items.c: Describes clientside interactions with items, including but not limited to inventory.
//This file may be used later for the loot dialogue as well.
#include <json/cJSON.h>
//This file kinda has an odd way of retrieving cg_itemData_t.
//cg_local.h actually #includes jkg_cg_items.h, so actually this file inherits everything from cg_local.h.
//In the future, I might move all that to cg_public.h or jkg_cg_public.h if the higher-ups decide it's too messy.
//Keep that in mind, also keep in mind the changed naming conventions between client<->server
#include "cg_local.h"

cgItemData_t CGitemLookupTable[MAX_ITEM_TABLE_SIZE];
cgArmorData_t armorMasterTable[MAX_ARMOR_PIECES];

static qboolean JKG_CG_ItemInInventory ( int itemNum )
{
    int i;
    for ( i = 0; i < cg.numItemsInInventory; i++ )
    {
        cgItemInstance_t *item = &cg.playerInventory[i];
        if ( item->id->itemID == itemNum )
        {
            return qtrue;
        }
    }
    
    return qfalse;
}

void JKG_CG_FillACISlot ( int itemNum, int slot )
{
    cgItemData_t *item;
    
    if ( itemNum < 0 || itemNum >= MAX_INVENTORY_ITEMS )
    {
        return;
    }
    
    if ( slot < 0 || slot >= MAX_ACI_SLOTS )
    {
        return;
    }
    
	item = cg.playerInventory[itemNum].id;
	if(!item)
	{
		return;
	}
    if ( !item->itemID )
    {
        return;
    }
    
    
    cg.playerACI[slot] = itemNum;
}

void JKG_CG_ClearACISlot ( int slot )
{
    if ( slot < 0 || slot >= MAX_ACI_SLOTS )
    {
        return;
    }
    
    cg.playerACI[slot] = -1;
}

void JKG_CG_ACICheckRemoval ( int itemNumber )
{
	int i;
	for(i = 0; i < MAX_ACI_SLOTS; i++)
	{
		if(cg.playerACI[i] < 0)
		{
			continue;
		}
		else if(cg.playerACI[i] > itemNumber)
		{
			cg.playerACI[i]--;
		}
	}
}

void JKG_CG_EquipItem ( int newItem, int oldItem )
{
    cg.playerInventory[newItem].equipped = qtrue;
    if ( oldItem != -1 )
    {
        cg.playerInventory[oldItem].equipped = qfalse;
    }
}

void JKG_CG_UnequipItem ( int inventorySlot )
{
    cg.playerInventory[inventorySlot].equipped = qfalse;
}

static void JKG_CompactInventory ( cgItemInstance_t *inventory )
{
    int i;
	// We can make this even more efficient, but not really much point.
	for(i = 0; i < MAX_INVENTORY_ITEMS; i++)
	{
	    int j;
	    
	    if ( inventory[i].id )
	    {
	        continue;
	    }
	    
	    for ( j = i + 1; j < MAX_INVENTORY_ITEMS; j++ )
	    {
	        if ( inventory[j].id )
	        {
	            inventory[i] = inventory[j];
	            memset (&inventory[j], 0, sizeof (inventory[j]));
	            
	            break;
	        }
	    }
	    
	    if ( j == MAX_INVENTORY_ITEMS )
	    {
	        // Current slot is null, but can find no other items above this one.
	        // Must be the end of the list.
	        break;
	    }
	}
}

void JKG_CG_DeltaFeed ( const char *mode )
{
	//This is part of something greater, which Boba calls a "delta feed".
	//Basically, a reliable command is sent to the client whenever our
	//inventory situation changes, i.e. we receive a new item, or stats
	//on the item change for whatever reason, or the item gets dropped or
	//destroyed. We send this information to the client as it comes. On
	//load, each item gets sent over on each frame.
	int id, i;

	id = atoi(CG_Argv(2));

	if(!Q_stricmp(mode, "add"))
	{
		for(i = IPPARSE_ID; i < IPPARSE_MAX; i++)
		{
		    int n = atoi (CG_Argv (i));
			switch(i)
			{
				case IPPARSE_ID:
					cg.playerInventory[id].id = &CGitemLookupTable[n];
					break;
				case IPPARSE_QUALITY:
					cg.playerInventory[id].itemQuality = n;
					break;
				case IPPARSE_AMOUNT1:
				case IPPARSE_AMOUNT2:
				case IPPARSE_AMOUNT3:
				case IPPARSE_AMOUNT4:
				case IPPARSE_AMOUNT5:
				case IPPARSE_AMOUNT6:
				case IPPARSE_AMOUNT7:
				case IPPARSE_AMOUNT8:
				case IPPARSE_AMOUNT9:
				case IPPARSE_AMOUNT10:
					cg.playerInventory[id].amount[i - IPPARSE_AMOUNT1] = n;
					break;
				case IPPARSE_EQUIPPED:
					cg.playerInventory[id].equipped = (n > 0);
					break;
			}
		}
		
		cg.numItemsInInventory++;
		return;
	}
	else if(!Q_stricmp(mode, "addfrc"))
	{
		for(i = IPPARSEX_ID; i < IPPARSEX_MAX; i++)
		{
		    int n = atoi (CG_Argv (i));
			switch(i)
			{
				case IPPARSEX_ID:
					cg.playerInventory[id].id = &CGitemLookupTable[n];
					break;
				case IPPARSEX_QUALITY:
					cg.playerInventory[id].itemQuality = n;
					break;
				case IPPARSEX_ACISLOT:
					JKG_CG_FillACISlot(id, n);
					break;
				case IPPARSEX_AMOUNT1:
				case IPPARSEX_AMOUNT2:
				case IPPARSEX_AMOUNT3:
				case IPPARSEX_AMOUNT4:
				case IPPARSEX_AMOUNT5:
				case IPPARSEX_AMOUNT6:
				case IPPARSEX_AMOUNT7:
				case IPPARSEX_AMOUNT8:
				case IPPARSEX_AMOUNT9:
				case IPPARSEX_AMOUNT10:
					cg.playerInventory[id].amount[i - IPPARSEX_AMOUNT1] = n;
					break;
				case IPPARSEX_EQUIPPED:
					cg.playerInventory[id].equipped = (n > 0);
					break;
			}
		}
		
		cg.numItemsInInventory++;
		return;
	}
	else if(!Q_stricmp(mode, "rem"))
	{
		//Remove an item
		JKG_CG_ACICheckRemoval ( id );
		memset(&cg.playerInventory[id], 0, sizeof(cgItemInstance_t));
		cg.numItemsInInventory--;
		assert (cg.numItemsInInventory >= 0);
		JKG_CompactInventory (cg.playerInventory);
	}
	else if(!Q_stricmp(mode, "chng"))
	{
		//Change a stat on the item.
		switch(atoi(CG_Argv(IPPARSE_ITEMNUM+1)))
		{
			case IPPARSE_ID:
				CG_Printf("^3Warning: Attempted to change ID of item %i (%s)\n", id, cg.playerInventory[id].id->displayName);
				CG_Printf("^3Report this to the developers immediately.\n");
				return;
			case IPPARSE_QUALITY:
				cg.playerInventory[id].itemQuality = atoi(CG_Argv(IPPARSE_ITEMNUM+2));
				return;
			case IPPARSE_AMOUNT1:
			case IPPARSE_AMOUNT2:
			case IPPARSE_AMOUNT3:
			case IPPARSE_AMOUNT4:
			case IPPARSE_AMOUNT5:
			case IPPARSE_AMOUNT6:
			case IPPARSE_AMOUNT7:
			case IPPARSE_AMOUNT8:
			case IPPARSE_AMOUNT9:
			case IPPARSE_AMOUNT10:
				cg.playerInventory[id].amount[atoi(CG_Argv(IPPARSE_ITEMNUM+1))-IPPARSE_AMOUNT1] = atoi(CG_Argv(IPPARSE_ITEMNUM+2));
				return;
			case IPPARSE_EQUIPPED:
				cg.playerInventory[id].equipped = (atoi(CG_Argv(IPPARSE_ITEMNUM+2)) > 0);
				return;
			default:
				CG_Printf("^3Warning: Invalid 'chng' arg #2 (%i)\n", atoi(CG_Argv(IPPARSE_ITEMNUM+1)));
				CG_Printf("^3Report this to the developers ASAP.\n");
				return;
		}
		//What the switch above does is actually pretty simple.
		//You'll get a command that looks something like this:
		//"pInv chng <iID> <int that corresponds to itemPacketParse_t> <amount>
	}
	else if(!Q_stricmp(mode, "open"))
	{
		// Opens the inventory. Self-explanatory.
		uiImports->InventoryNotify( 0 );
		return;
	}
	else
		CG_Printf("unknown mode parameter %s\n", mode);
}


stringID_table_t WPTable[]; // From bg_saga.c
static qboolean JKG_CG_ParseItem ( const char *itemFilePath, cgItemData_t *itemData )
{
	cJSON *json = NULL;
	cJSON *jsonNode = NULL;

	char error[MAX_STRING_CHARS];
	const char *str = NULL;
	int item, i;

	char itemFileData[MAX_ITEM_FILE_LENGTH];
	fileHandle_t f;
	int fileLen = trap_FS_FOpenFile(itemFilePath, &f, FS_READ);

	if(!f || fileLen == -1)
	{
		CG_Printf("^1Unreadable or empty file: %s. Please report this to the developers.\n", itemFilePath);
		return qfalse;
	}

	if( (fileLen + 1) >= MAX_ITEM_FILE_LENGTH )
	{
		trap_FS_FCloseFile(f);
		CG_Printf("^1%s item file too large. Please report this to the developers.\n", itemFilePath);
		return qfalse;
	}

	trap_FS_Read(&itemFileData, fileLen, f);
	itemFileData[fileLen] = '\0';

	trap_FS_FCloseFile(f);

	json = cJSON_ParsePooled (itemFileData, error, sizeof (error));
	if ( json == NULL )
	{
		CG_Printf ("^1%s: %s\n", itemFilePath, error);
		return qfalse;
	}

	//Initialize some things
	strcpy(itemData->itemIcon, "gfx/item_icons/default"); //BlasTech will hate me for this..

	//Basic Item Information
	jsonNode = cJSON_GetObjectItem (json, "name");
	str = cJSON_ToString (jsonNode);
	Q_strncpyz (itemData->displayName, str, sizeof (itemData->displayName));

	//not used
	/*jsonNode = cJSON_GetObjectItem (json, "internal");
	str = cJSON_ToString (jsonNode);
	strcpy(itemData->internalName, str);*/

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

	itemData->cost = 100;
	jsonNode = cJSON_GetObjectItem (json, "cost");
	item = cJSON_ToNumber (jsonNode);
	if(item > 0)
		itemData->cost = item;

	// Item Icon (for inventory)
	jsonNode = cJSON_GetObjectItem (json, "itemIcon");
	str = cJSON_ToString(jsonNode);
	if(str)
		Q_strncpyz (itemData->itemIcon, str, sizeof (itemData->itemIcon));

	// XML (examine menu)
	jsonNode = cJSON_GetObjectItem (json, "xml");
	str = cJSON_ToString(jsonNode);
	if(str)
		Q_strncpyz (itemData->xml, str, sizeof (itemData->xml));
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

		//not used
		/*if(i == 0)
			jsonNode = cJSON_GetObjectItem (json, "amount");
		else
			jsonNode = cJSON_GetObjectItem (json, va("amount%i", i));
		item = cJSON_ToNumber (jsonNode);
		itemData->amountBase[i] = item;*/

		if(i == 0)
			jsonNode = cJSON_GetObjectItem (json, "dur");
		else
			jsonNode = cJSON_GetObjectItem (json, va("dur%i", i));
		item = cJSON_ToNumber (jsonNode);
		itemData->duration[i] = item;

		/*if(i == 0)
			jsonNode = cJSON_GetObjectItem (json, "cnt");
		else
			jsonNode = cJSON_GetObjectItem (json, va("cnt%i", i));
		item = cJSON_ToNumber (jsonNode);
		if(item > 1)
			item = 1;
		else if(item < 0)
			item = 0;
		itemData->cntDown[i] = item;*/
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
	}

    cJSON_Delete (json);

	return qtrue;
}

extern int strap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize );
static qboolean JKG_CG_LoadItems ( void )
{
	int i, j;
	char itemFiles[8192];
	int numFiles = strap_FS_GetFileList("ext_data/items/", ".itm", itemFiles, sizeof(itemFiles));
	const char *itemFile = itemFiles;
	int successful = 0;
	int failed = 0;

	CG_Printf("Loading items...\n");
	for(i = 0; i < numFiles; i++ )
	{
	    cgItemData_t dummy;
		if ( !JKG_CG_ParseItem(va("ext_data/items/%s", itemFile), &dummy) )
		{
		    failed++;
		    continue;
		}

		if(dummy.itemID)
		{
			if(dummy.itemID < MAX_ITEM_TABLE_SIZE)
				successful++;

			else
			{
				CG_Printf("^1ERROR: item ID out of range!\nItem: ext_data/items/%s, ID: %d\n", itemFile, dummy.itemID);
				CG_Printf("^1Please report this to the developers immediately.\n");
				CG_Printf("^1Jedi Knight Galaxies will now try to correct this automatically\n"); //Referring to ourselves in third person again, hm?
				for(j = 1; j < MAX_ITEM_TABLE_SIZE; j++)
				{
					if(!CGitemLookupTable[j].itemID)
					{
						dummy.itemID = j;
						CG_Printf("^2Issue corrected.\n");
						break;
					}
				}
			}

		}
		else
		{
			failed++;
			continue;
		}

		if(CGitemLookupTable[dummy.itemID].itemID)
			CG_Printf("^3Duplicate item ID: %d. Please report this to the developers.\n", dummy.itemID);
		CGitemLookupTable[dummy.itemID] = dummy;

		itemFile += strlen(itemFile)+1;
	}
	CG_Printf("Items loaded: %d successful, %d failed\n", successful, failed);

	return (qboolean)(successful > 0);
}
void JKG_CG_InitItems( void )
{
	int i;
	memset(CGitemLookupTable, 0, sizeof(CGitemLookupTable));

	if(!JKG_CG_LoadItems())
	{
		Com_Error(ERR_FATAL, "Items could not be loaded. Possible corrupt database.");
		return;
	}

	for(i = 0; i < MAX_INVENTORY_ITEMS; i++)
	{
		memset(&cg.playerInventory[i], 0, sizeof(cgItemInstance_t));
	}

	//Init the shop stuff ~eez
	numShopItems = 0;
	memset(shopItems, 0, sizeof(shopItems));
}

//.armour files -- These are purely clientside files that determine rendering data
static qboolean JKG_CG_ParseArmorFile ( const char *armorFilePath, cgArmorData_t *armorData )
{
	cJSON *json = NULL;
	cJSON *jsonNode = NULL;
	char error[MAX_STRING_CHARS];
	const char *str = NULL;
	int item;

	char armorFileData[MAX_ITEM_FILE_LENGTH];
	fileHandle_t f;
	int fileLen = trap_FS_FOpenFile(armorFilePath, &f, FS_READ);

	if(!f || fileLen == -1)
	{
		CG_Printf("^1Unreadable or empty file: %s. Please report this to the developers.\n", armorFilePath);
		return qfalse;
	}

	if( (fileLen + 1) >= MAX_ITEM_FILE_LENGTH )
	{
		trap_FS_FCloseFile(f);
		CG_Printf("^1%s item file too large. Please report this to the developers.\n", armorFilePath);
		return qfalse;
	}

	trap_FS_Read(&armorFileData, fileLen, f);
	armorFileData[fileLen] = '\0';

	trap_FS_FCloseFile(f);

	json = cJSON_ParsePooled (armorFileData, error, sizeof (error));
	if ( json == NULL )
	{
		CG_Printf ("^1%s: %s\n", armorFilePath, error);
		return qfalse;
	}
	
	memset (armorData, 0, sizeof (*armorData));

	jsonNode = cJSON_GetObjectItem(json, "id");
	item = cJSON_ToNumber(jsonNode);
	armorData->id = item;

	jsonNode = cJSON_GetObjectItem(json, "model");
	str = cJSON_ToString(jsonNode);
	if ( str && str[0] ) Q_strncpyz (armorData->model, str, sizeof (armorData->model));

	jsonNode = cJSON_GetObjectItem(json, "skin");
	str = cJSON_ToString(jsonNode);
	if ( str && str[0] ) Q_strncpyz (armorData->skin, str, sizeof (armorData->skin));

	jsonNode = cJSON_GetObjectItem(json, "surf");
	armorData->surfOff = (qboolean)cJSON_ToBoolean(jsonNode);

	jsonNode = cJSON_GetObjectItem(json, "slot");
	str = cJSON_ToString(jsonNode);
	if(!Q_stricmp(str, "head")){
		armorData->slot = ARMSLOT_HEAD;
	} else if(!Q_stricmp(str, "neck")){
		armorData->slot = ARMSLOT_NECK;
	} else if(!Q_stricmp(str, "body") || !Q_stricmp(str, "torso")){
		armorData->slot = ARMSLOT_TORSO;
	} else if(!Q_stricmp(str, "robe")){
		armorData->slot = ARMSLOT_ROBE;
	} else if(!Q_stricmp(str, "legs")){
		armorData->slot = ARMSLOT_LEGS;
	} else if(!Q_stricmp(str, "hands") || !Q_stricmp(str, "hand") || !Q_stricmp(str, "gloves")){
		armorData->slot = ARMSLOT_GLOVES;
	} else if(!Q_stricmp(str, "boots") || !Q_stricmp(str, "foot") || !Q_stricmp(str, "feet")){
		armorData->slot = ARMSLOT_BOOTS;
	} else if(!Q_stricmp(str, "shoulder") || !Q_stricmp(str, "pauldron") || !Q_stricmp(str, "pauldrons")){
		armorData->slot = ARMSLOT_SHOULDER;
	} else if(!Q_stricmp(str, "implant") || !Q_stricmp(str, "implants")){
		armorData->slot = ARMSLOT_IMPLANTS;
	}

	jsonNode = cJSON_GetObjectItem(json, "surfLower");
	str = cJSON_ToString(jsonNode);
	if ( str && str[0] ) Q_strncpyz (armorData->surfOffLowerString, str, sizeof (armorData->surfOffLowerString));

	jsonNode = cJSON_GetObjectItem(json, "surfUpper");
	str = cJSON_ToString(jsonNode);
	if ( str && str[0] ) Q_strncpyz (armorData->surfOffThisString, str, sizeof (armorData->surfOffThisString));

	jsonNode = cJSON_GetObjectItem(json, "surfOnUpper");
	str = cJSON_ToString(jsonNode);
	if ( str && str[0] ) Q_strncpyz (armorData->surfOnThisString, str, sizeof (armorData->surfOnThisString));

    cJSON_Delete (json);

	return qtrue;
}

static qboolean JKG_CG_LoadArmor(void)
{
	int i = 0;
	char armorFiles[8192];
	int numFiles = strap_FS_GetFileList("ext_data/armor/", ".armour", armorFiles, sizeof(armorFiles));
	const char *armorFile = armorFiles;
	int successful = 0, failed = 0;

	CG_Printf("Loading armor data...\n");
	for( ; i < numFiles; i++)
	{
	    cgArmorData_t armor;
		if ( !JKG_CG_ParseArmorFile(va("ext_data/armor/%s", armorFile), &armor) )
		{
		    failed++;
		    continue;
		}

		if(armor.id > 0 && armor.id < MAX_ARMOR_PIECES)
		{
			successful++;
		}
		else
		{
			failed++;
			continue;
		}

		if(armorMasterTable[armor.id].id)
			CG_Printf("^3Warning: Duplicate armor ID %i\n", armor.id);
		armorMasterTable[armor.id] = armor;

		armorFile += strlen(armorFile)+1;
	}
	CG_Printf("Armor Loaded: %i successful, %i failed\n", successful, failed);

	return (qboolean)(successful > 0);
}

void JKG_CG_InitArmor( void )
{
	int i = 0, j = 0;
	memset(armorMasterTable, 0, sizeof(armorMasterTable));

	if(!JKG_CG_LoadArmor()){
		Com_Error(ERR_FATAL, "armorMasterTable NULL");
		return;
	}

    memset (&cgs.armorInformation, 0, sizeof (cgs.armorInformation));
	for(i = 0; i < MAX_CLIENTS; i++)
	{
		for(j = 0; j < ARMSLOT_MAX; j++)
			cg_entities[i].previousEquippedArmor[j] = 0;
	}
}

void JKG_CG_Armor_GetPartFromSurfString(int token, const char *string, char *buffer)
{
	//eezstreet - This grabs a section of a comma-delimited string.
	//For example: JKG_CG_Armor_GetPartFromSurfString(0, "blah,blah2") passes "blah" into buffer
	int i = 0, count = 0, j = 0;
	if(string)
	{
		while(string[i] != '\0')
		{
			if(string[i] == ','){
				count++;
				if(count > token)
					break;
			}
			if(count == token && string[i] != ',')
			{
				buffer[j] = string[i];
				j++;
			}
			i++;
		}
	}
	buffer[j] = '\0'; //NULL terminate it just to be safe -- eez
}

void JKG_CG_SetModelSurfacesFlags ( void *g2, const char *surfaces, int flags )
{
    char surfaceName[MAX_QPATH];
    int i = 0;
    
    if ( !surfaces || !surfaces[0] )
    {
        return;
    }
    
    while ( 1 )
    {
        JKG_CG_Armor_GetPartFromSurfString (i, surfaces, surfaceName);
		if(!surfaceName[0])
			break;
			
		trap_G2API_SetSurfaceOnOff (g2, surfaceName, flags);
		i++;
	}
}

#ifndef __cplusplus
typedef struct mdxmHeader_s {
	// 
	// ( first 3 fields are same format as MD3/MDR so we can apply easy model-format-type checks )
	//
	int			ident;				// "IDP3" = MD3, "RDM5" = MDR, "2LGM"(GL2 Mesh) = MDX   (cruddy char order I know, but I'm following what was there in other versions)
	int			version;			// 1,2,3 etc as per format revision
	char		name[MAX_QPATH];	// model name (eg "models/players/marine.glm")	// note: extension supplied
	char		animName[MAX_QPATH];// name of animation file this mesh requires	// note: extension missing
	int			animIndex;			// filled in by game (carcass defaults it to 0)

	int			numBones;			// (for ingame version-checks only, ensure we don't ref more bones than skel file has)

	int			numLODs;
	int			ofsLODs;

	int			numSurfaces;		// now that surfaces are drawn hierarchically, we have same # per LOD
	int			ofsSurfHierarchy;

	int			ofsEnd;				// EOF, which of course gives overall file size
} mdxmHeader_t;
#endif

//=========================================================
// JKG_G2_GetNumberOfSurfaces
//---------------------------------------------------------
// Description: This retrieves the number of surfaces in
// a GLM model. Use in conjunction with the G2 function,
// trap_G2API_GetSurfaceName to get all the surface names
// in the model. Surface names have a max length of
// MAX_QPATH.
//=========================================================
int JKG_G2_GetNumberOfSurfaces ( const char *modelPath )
{
    mdxmHeader_t header;
    fileHandle_t f;
    int fileLen = trap_FS_FOpenFile (modelPath, &f, FS_READ);
    if ( fileLen == -1 || !f )
    {
#ifdef _DEBUG
        CG_Printf ("Failed to open the model %s.\n", modelPath);
#endif
        return 0;
    }
    
    if ( fileLen < sizeof (mdxmHeader_t) )
    {
#ifdef _DEBUG
        CG_Printf ("Invalid model file %s.\n", modelPath);
#endif
        return 0;
    }
    
    trap_FS_Read (&header, sizeof (mdxmHeader_t), f);
    trap_FS_FCloseFile (f);
    
    return header.numSurfaces;
}

void JKG_CG_ShowOnlySelectedSurfaces ( void *g2, const char *modelPath, const char *visibleSurfaces )
{
    int i;
	char surfaceName[MAX_QPATH];
	int numSurfaces;
	
	if ( !visibleSurfaces || !visibleSurfaces[0] )
	{
	    return;
	}
	
	numSurfaces = JKG_G2_GetNumberOfSurfaces(modelPath);

	for(i = 0; i < numSurfaces; i++)
	{
		trap_G2API_GetSurfaceName( g2, i, 0, surfaceName );
		if ( surfaceName[0] == '*' )
		{
		    // this is actually the triangle for a bolt. Don't poke it! D;
		    continue;
		}
		trap_G2API_SetSurfaceOnOff (g2, surfaceName, 0x2);
	}

	i = 0;
	//Now turn on the relevant surfaces
	while( 1 )
	{
		JKG_CG_Armor_GetPartFromSurfString(i, visibleSurfaces, surfaceName);
		if(!surfaceName[0])
			break;
			
		trap_G2API_SetSurfaceOnOff (g2, surfaceName, 0);
		i++;
	}
}

qboolean CG_RegisterClientArmorModelname( centity_t *cent, int armorNum, int clientNum, int slot);
void JKG_CG_EquipArmor( void )
{
	int client = atoi(CG_Argv(1));
	int slot = atoi(CG_Argv(2));
	int armor = atoi(CG_Argv(3));
	
	centity_t *cent = &cg_entities[client];

	cgs.armorInformation[client][slot] = armor;
	cent->equippedArmor[slot] = armor;
	
	if ( !armor )
	{
	    // Show surfaces which were hidden on the player model for the previous armor.
	    JKG_CG_SetModelSurfacesFlags (cent->ghoul2, armorMasterTable[cent->previousEquippedArmor[slot]].surfOffLowerString, 0);
	    cent->previousEquippedArmor[slot] = armor;
	    
	    if ( cent->armorGhoul2[slot] && trap_G2_HaveWeGhoul2Models (cent->armorGhoul2[slot]) )
	    {
	        trap_G2API_CleanGhoul2Models(&cent->armorGhoul2[slot]);
	    }
	    return;
	}
	
	if(!CG_RegisterClientArmorModelname(cent, armor, client, slot))
	{
		CG_Printf("Failed to equip armor model %s in slot %i", armorMasterTable[armor].model, slot);
		return;
	}

    // Show surfaces which were hidden on the player model for the previous armor.
    JKG_CG_SetModelSurfacesFlags (cent->ghoul2, armorMasterTable[cent->previousEquippedArmor[slot]].surfOffLowerString, 0);
    
    // Now disable surfaces on the player model which need to been hidden on the new armor.
    JKG_CG_SetModelSurfacesFlags (cent->ghoul2, armorMasterTable[armor].surfOffLowerString, 0x2);

	// Disable surfaces on the upper model as well
	JKG_CG_SetModelSurfacesFlags (cent->armorGhoul2, armorMasterTable[armor].surfOffThisString, 0x2);

	cent->previousEquippedArmor[slot] = armor;
}

void JKG_CG_SetACISlot(const unsigned short slot)
{
	if(slot < 0 || slot >= MAX_ACI_SLOTS)
	{
		return;
	}

	if(!cg.playerACI[slot] || cg.playerACI[slot] < 0)
	{
		return;
	}

	if(cg.playerInventory[cg.playerACI[slot]].id->itemType != ITEM_WEAPON)
	{
		return;
	}

	cg.weaponSelect = slot;
}
