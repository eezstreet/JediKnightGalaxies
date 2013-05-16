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
    ITEM_BUFF
} jkgItemType_t;

void BG_LoadDefaultWeaponItems ( void );
qboolean BG_HasWeaponItem ( int clientNum, int weaponId );

#endif
