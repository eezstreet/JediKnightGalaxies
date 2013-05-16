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
// bg_items.h
// Loader for the .itm and .loot (obsolete) formats
// (c) 2013 Jedi Knight Galaxies

#ifndef BG_ITEMS_H
#define BG_ITEMS_H

#include "q_shared.h"

#define MAX_ITEM_TABLE_SIZE     (65535)
#define MAX_ITEM_FILE_LENGTH    (16384)
#define MAX_ITEM_NAME			(64)
#define MAX_INVENTORY_ITEMS		(256)

#define MAX_PSPELLS				(10)
#define MAX_ACI_SLOTS			(10)

typedef enum jkgItemType_e
{
    ITEM_UNKNOWN,
    ITEM_WEAPON,
    ITEM_ARMOR,
    ITEM_CLOTHING,
    ITEM_BUFF,
	ITEM_CONSUMABLE
} jkgItemType_t;

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
	PSPELL_SEEKER,
	PSPELL_PLAYSOUND,
	PSPELL_DESTROYITEM,
	PSPELL_CND_GT_PS,
	PSPELL_CND_EQ_PS,
	PSPELL_CND_LT_PS,
	PSPELL_CND_GT_CS,
	PSPELL_CND_EQ_CS,
	PSPELL_CND_LT_CS,
	PSPELL_DO_EVENT,
	PSPELL_MAX
} pSpell_t;

void BG_LoadDefaultWeaponItems ( void );
qboolean BG_HasWeaponItem ( int clientNum, int weaponId );

#endif
