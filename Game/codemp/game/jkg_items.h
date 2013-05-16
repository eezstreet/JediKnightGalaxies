// Copyright (C) 2011 Jedi Knight Galaxies
// bg_items.h: Contains all major item definitions.
#pragma once

#include "q_shared.h"
#include "bg_public.h"
#include "bg_strap.h"

#ifndef BGITEMH
#define BGITEMH

#include "bg_items.h"

#define MAX_LOOT_ITEMS			10
#define MAX_LOOT_DISTANCE		256
#define MAX_LOOT_TABLE_SIZE		512

 //eezstreet - doubled this limit
#define MAX_INVENTORY_WEIGHT	50

typedef enum
{
	ARMTYPE_LIGHT,
	ARMTYPE_MEDIUM,
	ARMTYPE_HEAVY,
	ARMTYPE_MAX
} armorTypes_t;

typedef struct
{
	//Basic Item Information
	char displayName[MAX_ITEM_NAME];
	char internalName[MAX_ITEM_NAME];
	unsigned int itemID;
	jkgItemType_t itemType;
	unsigned int weight;

	//pSpell data
	unsigned int pSpell[MAX_PSPELLS];
	int affector[MAX_PSPELLS];
	unsigned int amountBase[MAX_PSPELLS];
	unsigned int duration[MAX_PSPELLS];
	qboolean cntDown[MAX_PSPELLS];

	//Equipment Data
	unsigned int weapon;		//Weapons: What weapon this item links to
	unsigned int variation;		//Weapons: What variation this item links to
	int varID;					//Weapons: Generated at runtime. Grabs the IoV
	unsigned int armorID;		//Armor: What armor this item links to (todo: port this to client only)
	unsigned int armorSlot;		//Armor: What slot this armor fills.

	//Stats
	unsigned int baseDefense;
	armorTypes_t armorType;
	unsigned int baseDurabilityMax;
	unsigned int averageDurability;
} itemData_t;

typedef struct
{
	//Basic Item Information
	itemData_t	*id;
	unsigned int itemQuality; //quality of this instance

	//pSpell data
	unsigned int amount[MAX_PSPELLS];
	unsigned int calc1;
	unsigned int calc2;

	//Equipped Item Stuff
	qboolean equipped;
	unsigned int defense;
	unsigned int durabilityCurrent;
} itemInstance_t;
typedef itemInstance_t inventory_t[MAX_INVENTORY_ITEMS];

typedef struct
{
	unsigned int lootItemID[MAX_LOOT_ITEMS];
	unsigned int lootItemChance[MAX_LOOT_ITEMS];	//chance in % for this loot to spawn
	unsigned int lootID;							//loot table ID
	unsigned int numItems;
} lootTable_t;

typedef enum
{ //NOTENOTE: Either edit this on this client each time, or make a bg_ file...god...so many decisions!
	IQUAL_LOWQUAL,
	IQUAL_NORMAL,
	IQUAL_HIQUAL,
	IQUAL_SOCKETED,
	IQUAL_UNCOMMON,
	IQUAL_RARE,
	IQUAL_UNIQUE,
	IQUAL_EPIC,
	IQUAL_MAX
} itemQuality_t;

typedef enum
{
    PSPELL_NONE,
	PSPELL_ADD_PS,
	PSPELL_SUB_PS,
	PSPELL_MUL_PS,
	PSPELL_DIV_PS,
	PSPELL_EXP_PS,
	PSPELL_SQT_PS,
	PSPELL_ADD_CS,
	PSPELL_SUB_CS,
	PSPELL_MUL_CS,
	PSPELL_DIV_CS,
	PSPELL_EXP_CS,
	PSPELL_SQT_CS,
	PSPELL_MAX
} pSpell_t;

typedef enum
{
	ARMSLOT_HEAD,
	ARMSLOT_NECK,
	ARMSLOT_TORSO,
	ARMSLOT_ROBE,
	ARMSLOT_LEGS,
	ARMSLOT_GLOVES,
	ARMSLOT_BOOTS,
	ARMSLOT_SHOULDER,
	ARMSLOT_IMPLANTS,
	ARMSLOT_MAX
} armorSlots_t;

typedef enum
{
	CORESTAT_WEIGHT,
	CORESTAT_MAX
} coreStatConversion_t;

typedef struct
{
	itemInstance_t *items;
	size_t size;
	unsigned int elements;
} inv_t;

itemData_t *JKG_GetItemByWeaponIndex ( int weaponIndex );

void JKG_A_RollItem( unsigned int itemIndex, int qualityOverride, inv_t *inventory );
#endif
