//       ____ ___________________   ___           ____  __ _______   ___  ________  ___ ______________
//      |    |\_   _____/\______ \ |   |         |    |/ _|\      \ |   |/  _____/ /   |   \__    ___/
//      |    | |    __)_  |    |  \|   |         |      <  /   |   \|   /   \  ___/    ~    \|    |   
//  /\__|    | |        \ |    `   \   |         |    |  \/    |    \   \    \_\  \    Y    /|    |   
//  \________|/_______  //_______  /___|         |____|__ \____|__  /___|\______  /\___|_  / |____|   
//                    \/         \/                      \/       \/            \/       \/           
//                         ________    _____   ____       _____  ____  ___ ______________ _________   
//                        /  _____/   /  _  \ |    |     /  _  \ \   \/  /|   \_   _____//   _____/   
//                       /   \  ___  /  /_\  \|    |    /  /_\  \ \     / |   ||    __)_ \_____  \    
//                       \    \_\  \/    |    \    |___/    |    \/     \ |   ||        \/        \   
//                        \______  /\____|__  /_______ \____|__  /___/\  \|___/_______  /_______  /   
//                               \/         \/        \/	   \/	   \_/			  \/        \/ (c)
// jkg_cg_items.h
// Jedi Knight Galaxies (c) 2011
// File by eezstreet
// jkg_cg_items.h - Defines clientside code for items.
// Note that this file is needed by the entire clientside, so cg_local.h actually #includes this file.
// Copyright (c) 2011 Jedi Knight Galaxies


/* Global Definitions */
#ifndef CGITEMH
#define CGITEMH

//#include "../game/q_shared.h"
#include "../game/bg_items.h"

#define MAX_ARMOR_PIECES		1024

// UQ1: Moved to bg_items.c to stop compile header issues...
extern int shopItems[128];
extern int numShopItems;

/* Structure Definitions */
typedef struct
{

    // LOOK AT ME, I'M A COMMENT.
    // Okay, I have your attention now. If you edit this struct, make sure you
    // update the struct in ui/jkg_inventory.c too.


	//oboi, clientside time!
	//Basic Item Characteristics
	char displayName[MAX_ITEM_NAME];
	char itemIcon [MAX_QPATH];
	char xml[MAX_QPATH];
	unsigned int itemID;
	jkgItemType_t itemType;
	unsigned int weight;
	unsigned int cost;

	//pSpell data needed by clientside
	unsigned int pSpell[MAX_PSPELLS];
	int affector[MAX_PSPELLS];
	unsigned int duration[MAX_PSPELLS];

	//Equipment/ACI information
	unsigned int weapon;
	unsigned int variation;
	int varID;
} cgItemData_t;

typedef struct
{

    // LOOK AT ME, I'M A COMMENT.
    // Okay, I have your attention now. If you edit this struct, make sure you
    // update the struct in ui/jkg_inventory.c too.


	cgItemData_t *id;
	unsigned int itemQuality;

	//pSpell stuff
	unsigned int amount[MAX_PSPELLS]; //NOTENOTE: Need to make a variable which holds all final values of stats

	//ACI stuff
	qboolean equipped;
} cgItemInstance_t;

typedef struct
{
	unsigned int id;
	char model[MAX_QPATH];
	char skin[MAX_QPATH];
	unsigned int slot;
	qboolean surfOff;
	char surfOffThisString[512];
	char surfOffLowerString[512];
	char surfOnThisString[512];
} cgArmorData_t;

/* Enum Definitions */
typedef enum
{ //NOTENOTE: Either edit this on this server each time, or make a bg_ file...god...so many decisions!
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
	IPPARSE_CMD,
	IPPARSE_MODE,
	IPPARSE_ITEMNUM,
	IPPARSE_ID, //This must always come first.
	IPPARSE_QUALITY,
	IPPARSE_AMOUNT1,
	IPPARSE_AMOUNT2,
	IPPARSE_AMOUNT3,
	IPPARSE_AMOUNT4,
	IPPARSE_AMOUNT5,
	IPPARSE_AMOUNT6,
	IPPARSE_AMOUNT7,
	IPPARSE_AMOUNT8,
	IPPARSE_AMOUNT9,
	IPPARSE_AMOUNT10,
	IPPARSE_EQUIPPED,
	IPPARSE_MAX
} itemPacketParse_t;

typedef enum
{
	IPPARSEX_CMD,
	IPPARSEX_MODE,
	IPPARSEX_ITEMNUM,
	IPPARSEX_ID, //This must always come first.
	IPPARSEX_ACISLOT,
	IPPARSEX_QUALITY,
	IPPARSEX_AMOUNT1,
	IPPARSEX_AMOUNT2,
	IPPARSEX_AMOUNT3,
	IPPARSEX_AMOUNT4,
	IPPARSEX_AMOUNT5,
	IPPARSEX_AMOUNT6,
	IPPARSEX_AMOUNT7,
	IPPARSEX_AMOUNT8,
	IPPARSEX_AMOUNT9,
	IPPARSEX_AMOUNT10,
	IPPARSEX_EQUIPPED,
	IPPARSEX_MAX
} itemPacketParse_modified_t;

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

/* Function Declarations */

void JKG_CG_FillACISlot ( int itemNum, int slot );
void JKG_CG_ClearACISlot ( int slot );

void JKG_CG_EquipItem ( int newItem, int oldItem );
void JKG_CG_UnequipItem ( int inventorySlot );
void JKG_CG_EquipArmor( void );

void JKG_CG_DeltaFeed ( const char *mode );

void JKG_CG_Armor_GetPartFromSurfString(int token, const char *string, char *buffer);
int JKG_G2_GetNumberOfSurfaces ( const char *modelPath );

void JKG_CG_ShowOnlySelectedSurfaces ( void *g2, const char *modelPath, const char *visibleSurfaces );
void JKG_CG_SetModelSurfacesFlags ( void *g2, const char *surfaces, int flags );

/* Trailing Data */
#endif