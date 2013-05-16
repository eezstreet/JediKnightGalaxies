// Copyright (C) 2011 Jedi Knight Galaxies
// bg_equip.c: Handles weapon/armor procedures.
// File by eezstreet

#include "jkg_items.h"
#include "g_local.h"
#include <json/cJSON.h>

void initACI(gclient_t *client)
{
    memset (&client->coreStats.ACISlots, 0, sizeof (client->coreStats.ACISlots));
    client->coreStats.aciSlotsUsed = 0;
}

void JKG_EquipItem(gentity_t *ent, int iNum)
{
	if(!ent->client)
		return;
		
	if ( iNum < 0 || iNum >= MAX_INVENTORY_ITEMS )
	{
		trap_SendServerCommand(ent->client->ps.clientNum, "print \"Invalid inventory slot.\n\"");
	    return;
	}
		
	if ( !ent->inventory->items[iNum].id )
	{
	    return;
	}

	if( iNum >= ent->inventory->elements )
	{
		trap_SendServerCommand(ent->client->ps.clientNum, "print \"You do not have an item in that slot.\n\"");
		return;
	}

	if(ent->inventory->items[iNum].equipped)
	{
		//trap_SendServerCommand(ent->client->ps.clientNum, "print \"That item is already equipped.\n\"");
		return;
	}

	if(ent->inventory->items[iNum].id->itemType == ITEM_WEAPON)
	{
	    int i = 0;
	    int prevEquipped = -1;
	    
		while(ent->inventory->items[i].id && i < ent->inventory->elements)
		{
			if(i == iNum)
			{
				i++;
				continue;
			}
			if( ent->inventory->items[i].id->itemType == ITEM_WEAPON &&
				ent->inventory->items[i].equipped )
			{
				ent->inventory->items[i].equipped = qfalse;
				prevEquipped = i;
				break;
			}
			i++;
		}
	    
	    //ent->inventory[iNum].equipped = qtrue;
		ent->inventory->items[iNum].equipped = qtrue;
	    trap_SendServerCommand (ent->s.number, va ("ieq %d %d", iNum, prevEquipped));
		trap_SendServerCommand (ent->s.number, va ("chw %d", ent->inventory->items[iNum].id->varID));
	}
	else if(ent->inventory->items[iNum].id->itemType == ITEM_ARMOR){
	    // Unequip the armor which is currently equipped at the slot the new armor will use.
	    int i = 0;
	    int prevEquipped = -1;
	    
	    /*for ( i = 0; i < MAX_INVENTORY_ITEMS; i++ )
	    {
	        if ( !ent->inventory[i].id )
	        {
	            break;
	        }
	        
	        if ( i == iNum )
	        {
	            continue;
	        }
	        
	        if ( ent->inventory[i].id->itemType == ITEM_ARMOR && ent->inventory[i].equipped &&
	            ent->inventory[iNum].id->armorSlot == ent->inventory[i].id->armorSlot )
	        {
	            // There should only be one armor equipped at this slot.
	            ent->inventory[i].equipped = qfalse;
	            prevEquipped = i;
	            break;
	        }
	    }*/
		while( ent->inventory->items[i].id && i < ent->inventory->elements )
		{
			if( i == iNum )
			{
				i++;
				continue;
			}

			if( ent->inventory->items[i].id->itemType == ITEM_ARMOR && ent->inventory->items[i].equipped &&
				ent->inventory->items[iNum].id->armorSlot == ent->inventory->items[i].id->armorSlot )
			{
				ent->inventory->items[i].equipped = qfalse;
				prevEquipped = i;
				break;
			}
			i++;
		}
	    
		/*ent->inventory[iNum].equipped = qtrue;
		ent->client->armorItems[ent->inventory[iNum].id->armorSlot] = iNum;
		
		trap_SendServerCommand (ent->s.number, va ("ieq %d %d", iNum, prevEquipped));
		trap_SendServerCommand(-1, va("aequi %i %i %i", ent->client->ps.clientNum, ent->inventory[iNum].id->armorSlot, ent->inventory[iNum].id->armorID));*/
		ent->inventory->items[iNum].equipped = qtrue;
		ent->client->armorItems[ent->inventory->items[iNum].id->armorSlot] = iNum;

		trap_SendServerCommand( ent->s.number, va("ieq %d %d", iNum, prevEquipped ));
		trap_SendServerCommand( -1, va("aequi %i %i %i", ent->client->ps.clientNum, ent->inventory->items[iNum].id->armorSlot, ent->inventory->items[iNum].id->armorID));
	}
	else
	{
		trap_SendServerCommand(ent->client->ps.clientNum, "print \"You cannot equip that item.\n\"");
	}
}

void JKG_UnequipItem(gentity_t *ent, int iNum)
{
	if(!ent->client)
		return;
		
	if ( iNum < 0 || iNum >= MAX_INVENTORY_ITEMS )
	{
		trap_SendServerCommand(ent->client->ps.clientNum, "print \"Invalid inventory slot.\n\"");
	    return;
	}

	if( iNum >= ent->inventory->elements )
	{
		trap_SendServerCommand(ent->client->ps.clientNum, "print \"You do not have an item in that slot.\n\"");
		return;
	}
		
	if ( !ent->inventory->items[iNum].id || ent->inventory->items[iNum].id == (itemData_t *)0xCDCDCDCD ) //fixme: bad hack here
	{
#ifdef DEBUG
		trap_SendServerCommand(ent->client->ps.clientNum, "print \"^3WARNING: Attempted to access uninitialized heap memory\n\"");
#endif
	    return;
	}
	
	if(!ent->inventory->items[iNum].equipped)
	{
		//trap_SendServerCommand(ent->client->ps.clientNum, "print \"That item is not equipped.\n\"");
		return;
	}

	if(ent->inventory->items[iNum].id->itemType == ITEM_WEAPON)
	{
		ent->inventory->items[iNum].equipped = qfalse;
	    trap_SendServerCommand (ent->s.number, va ("iueq %i", iNum));
	    trap_SendServerCommand (ent->s.number, "chw 0");
	}
	else if(ent->inventory->items[iNum].id->itemType == ITEM_ARMOR)
	{
		ent->inventory->items[iNum].equipped = qfalse;
		ent->client->armorItems[ent->inventory->items[iNum].id->armorSlot] = 0;
		trap_SendServerCommand (ent->s.number, va ("iueq %i", iNum));
		trap_SendServerCommand(-1, va("aequi %i %i 0", ent->client->ps.clientNum, ent->inventory->items[iNum].id->armorSlot));
	}
	else
	{
		//trap_SendServerCommand(ent->client->ps.clientNum, "print \"You cannot unequip that item.\n\"");
	}
}

