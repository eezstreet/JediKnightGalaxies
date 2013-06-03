#include "q_shared.h"
#include "bg_items.h"
#include "bg_weapons.h"
#ifdef QAGAME
    #include "g_local.h"
    #include "jkg_items.h"
    typedef itemData_t bgItemData_t;
    extern itemData_t itemLookupTable[MAX_ITEM_TABLE_SIZE];
    #define itemTable itemLookupTable
#else
    #include "../cgame/cg_local.h"
    #include "../cgame/jkg_cg_items.h"
    typedef cgItemData_t bgItemData_t;
    extern cgItemData_t CGitemLookupTable[MAX_ITEM_TABLE_SIZE];
    #define itemTable CGitemLookupTable
#endif

int shopItems[128];
int numShopItems;

static int BG_GetNextFreeItemSlot ( void )
{
    int i = 1;
    for ( i = 1; i < MAX_ITEM_TABLE_SIZE; i++ )
    {
        if ( !itemTable[i].itemID )
        {
            return i;
        }
    }
    
    return 0;
}

qboolean BG_HasWeaponItem ( int clientNum, int weaponId )
{
    int i;
    // Hate having to do it like this, but seems like the only way atm...
#ifdef CGAME
    for ( i = 0; i < cg.numItemsInInventory; i++ )
    {
		int weaponNum;
		if(!cg.playerInventory[i].id || !cg.playerInventory[i].id->itemID)
			break;
		if(cg.playerInventory[i].id->itemType != ITEM_WEAPON)
			continue;
		weaponNum = BG_GetWeaponIndex(cg.playerInventory[i].id->weapon, cg.playerInventory[i].id->variation);
		if( weaponNum == weaponId )
			return qtrue;
    }
#else
    gentity_t *ent = &g_entities[clientNum];
	//begin DIMA correction
	i = 0;
    while( 1 )
    {
		int weaponNum;
		if ( !ent->inventory->items[i].id )
        {
            break;
        }
		if( i > ent->inventory->elements )
			break;
		if(ent->inventory->items[i].equipped < 0)
			break; //This is a VERY hacky check..proceed with caution.
		if(!ent->inventory->items[i].id->itemID)
		{
			break;
		}
		if( ent->inventory->items[i].id->itemType != ITEM_WEAPON)
		{
			i++;
			continue;
		}
		weaponNum = BG_GetWeaponIndex(ent->inventory->items[i].id->weapon, ent->inventory->items[i].id->variation);
		if(weaponNum == weaponId)
			return qtrue;
		i++;
    }
	//end DIMA correction
#endif

    return qfalse;
}

// Creates weightless weapon items if no item files
// exist for items.
void BG_LoadDefaultWeaponItems ( void )
{
    int i = 0;
    int end = BG_NumberOfLoadedWeapons();
    qboolean weaponHasItem[MAX_WEAPON_TABLE_SIZE] = { qfalse };
    
    for ( i = 1; i < MAX_ITEM_TABLE_SIZE; i++ )
    {
        bgItemData_t *item = &itemTable[i];
        if ( !item->itemID )
        {
            continue;
        }
        
        if ( item->itemType != ITEM_WEAPON )
        {
            continue;
        }
        
        weaponHasItem[item->varID] = qtrue;
    }
    
    for ( i = 0; i < end; i++ )
    {
        weaponData_t *weaponData;
        int weapon, variation;
        int itemID;
        bgItemData_t item;
        
        if ( weaponHasItem[i] )
        {
            continue;
        }
        
        if ( !BG_GetWeaponByIndex (i, &weapon, &variation) )
        {
            break;
        }
        
        memset (&item, 0, sizeof (item));
        weaponData = GetWeaponData (weapon, variation);
        Q_strncpyz (item.displayName, weaponData->displayName, sizeof (item.displayName));
        itemID = BG_GetNextFreeItemSlot();
        if ( !itemID )
        {
            Com_Printf ("Ran out of item space for weapons.\n");
            break;
        }
        
        item.itemID = itemID;
        item.itemType = ITEM_WEAPON;
        item.weapon = weapon;
        item.variation = variation;
        item.varID = i;
#ifdef CGAME
		strcpy(item.itemIcon, weaponData->visuals.icon);
#endif
        
        itemTable[itemID] = item;
    }
}
