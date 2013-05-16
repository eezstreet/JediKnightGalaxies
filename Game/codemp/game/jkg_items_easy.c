#include "jkg_easy_items.h"
#include "jkg_dynarrays.h"
#include "g_local.h"

extern itemData_t itemLookupTable[MAX_ITEM_TABLE_SIZE];

void JKG_Easy_AddItemToInventory(itemInstance_t *buffer, inv_t *inventory, unsigned int itemID)
{
	unsigned int i = 0;
	JKG_A_RollItem(itemID, 0, inventory);
}

void JKG_Easy_RemoveItemFromInventory(int number, itemInstance_t **inventory)
{
	int i;
	if(number <= 0)
		return;
	for(i = number; i < MAX_INVENTORY_ITEMS; i++)
	{
		if(!inventory[number]->id->itemID)
			break;
		inventory[number] = inventory[number+1];
		i++;
	}
}

int JKG_Easy_FindItemByInternal(char *internalName)
{
	int i;
	for(i = 1; i < MAX_ITEM_TABLE_SIZE; i++)
	{
		if(itemLookupTable[i].itemID)
		{
			if(Q_stricmp(itemLookupTable[i].internalName, internalName) == 0)
			{
				return i;
			}
		}
	}
	return 0;
}

//Init and cleanup funcs

void JKG_Easy_DIMA_Init(inv_t *inventory)
{
	inventory->elements = 0;
	inventory->items = malloc(sizeof(itemInstance_t) * DIMA_PREALLOC);
	inventory->size = sizeof(itemInstance_t) * DIMA_PREALLOC;
}

void JKG_Easy_DIMA_GlobalInit(void)
{
	int i = 0;
	for(; i < MAX_GENTITIES; i++)
	{
		g_entities[i].inventory = malloc(sizeof(inv_t));
		JKG_Easy_DIMA_Init(g_entities[i].inventory);
	}
}

void JKG_Easy_DIMA_CleanEntity(int entNum)
{
	g_entities[entNum].inventory->elements = 0;
	g_entities[entNum].inventory->items = realloc(g_entities[entNum].inventory->items, sizeof(itemInstance_t) * DIMA_PREALLOC);
	g_entities[entNum].inventory->size = sizeof(itemInstance_t) * DIMA_PREALLOC;
}

void JKG_Easy_DIMA_Cleanup(void)
{
	int i = 0;
	for(i; i < MAX_GENTITIES; i++)
	{
		free(g_entities[i].inventory);
	}
}

//JKG_Easy_DIMA_Add
//Param: inv_t *inventory, itemInstance_t item
//Purpose: Add an item to a DIMA inventory

void JKG_Easy_DIMA_Add(inv_t *inventory, itemInstance_t item)
{
	unsigned int newElements = inventory->elements + 1;
	if(((inventory->elements + 1) * sizeof(itemInstance_t)) > sizeof(inventory->size))
	{
		inventory->size += (sizeof(itemInstance_t) * DIMA_PREALLOC);
		inventory->items = realloc(inventory->items, inventory->size);
		if(!inventory->items)
		{
			Com_Error(ERR_FATAL, "JKG_Easy_DIMA_Add: Out of memory.");
			return;
		}
	}
	inventory->items[inventory->elements++] = item;
}

//JKG_Easy_DIMA_Remove
//Params: inv_t *inventory, unsigned int invID
//Purpose: Remove an item from a DIMA inventory

void JKG_Easy_DIMA_Remove(inv_t *inventory, unsigned int invID)
{
	if(invID > inventory->elements || invID < 0)
		return;
	if(invID != inventory->elements-1)
	{
		int i = invID + 1;
		for(i; i < inventory->elements - 1; i++)
		{
			inventory->items[i] = inventory->items[i-1];
		}
	}
	inventory->elements--;
}

//JKG_Easy_DIMA_CMPInternal
//Params: inv_t *inventory, char *c1
//Returns: int
//Purpose: Returns inventory ID of matching item

int JKG_Easy_DIMA_CMPInternal(inv_t *inventory, char *c1)
{
	int i = 0;

	for(i; i < inventory->elements; i++)
	{
		if(!Q_stricmp(c1, inventory->items[i].id->internalName))
			return i;
	}
	return -1;
}

//JKG_Easy_DIMA_CMPItemID
//Params: inv_t *inventory, unsigned int itemID
//Returns: int
//Purpose: Returns inventory ID of matching item

int JKG_Easy_DIMA_CMPItemID(inv_t *inventory, unsigned int itemID)
{
	int i = 0;
	if(!itemID)
		return -1;

	for(i; i < inventory->elements; i++)
	{
		if(inventory->items[i].id->itemID == itemID)
			return i;
	}
	return -1;
}

unsigned int JKG_Easy_GetItemIDFromInternal(const char *internalName)
{
	unsigned int i;
	for(i = 0; i < MAX_ITEM_TABLE_SIZE; i++)
	{
		if(!itemLookupTable[i].itemID)
			return -1;
		if(Q_stricmp(itemLookupTable[i].internalName, internalName) == 0)
			return i;
	}
	return -1;
}