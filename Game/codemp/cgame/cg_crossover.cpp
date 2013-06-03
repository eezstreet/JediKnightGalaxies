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
// cg_crossover.c -- Crossover API module for CGame
// Copyright (c) 2013 Jedi Knight Galaxies

#include "cg_local.h"
#include "../ui/ui_shared.h"

uiCrossoverExports_t *uiImports;

cgCrossoverExports_t co;

int CO_GetRedTeam( void )
{
	return cgs.redTeam;
}

int CO_GetBlueTeam( void )
{
	return cgs.blueTeam;
}

qboolean CO_EscapeTrapped( void )
{
	if( cg.trapEscape )
	{
		trap_Syscall_CG();
		trap_SendClientCommand("~esc");
		trap_Syscall_UI();
		return qtrue;
	}
	return qfalse;
}

// TODO: put inventory/shop/pazaak crap into appropriate files, this is really bad --eez
static void CO_InventoryAttachToACI ( int itemNum, int slot, int attach )
{
    if ( attach )
    {
        JKG_CG_FillACISlot (itemNum, slot);
    }
    else
    {
        JKG_CG_ClearACISlot (slot);
    }
}

extern cgItemData_t CGitemLookupTable[MAX_ITEM_TABLE_SIZE];
static void *CO_InventoryDataRequest ( int data )
{
	if(data >= 50)
	{
		//HACK ALERT
		if(cg.playerInventory[data-50].id && (data-50) >= 0 && (data-50) < MAX_INVENTORY_ITEMS)
		{
			return (void *)cg.playerInventory[data-50].id->itemIcon;
		}
		else
		{
			return NULL;
		}
	}
    switch ( data )
    {	// FIXME: enumerable --eez
        case 0: // inventory count
            return (void *)&cg.numItemsInInventory;
        case 1: // inventory list
            return (void *)cg.playerInventory;
        case 2:
            return (void *)cg.playerACI;
		case 3:
			return (void *)cg.predictedPlayerState.persistant[PERS_CREDITS];
		case 4:
			return (void *)shopItems;
		case 5:
			return (void *)numShopItems;
		case 6:
			return (void *)CGitemLookupTable;
        default:
            return NULL;
    }
}

static void *CO_PartyMngtDataRequest(int data) {
	// FIXME: enumerable, this needs moved elsewhere also --eez
	// Team Management data request from UI
	// Data 0 = Current party/invitations
	// Data 1 = Seeking players
	// Data 2 = Last seeking players refresh time (needed for delta feed)
	if (data == 0) {
		return &cgs.party;
	} else if (data == 1) {
		return &cgs.partyList;
	} else if (data == 2) {
		return (void *)cgs.partyListTime;
	} else {
		return 0;
	}
}

void CO_SendClientCommand( const char *command )
{
	trap_Syscall_CG();
	trap_SendClientCommand( command );
	trap_Syscall_UI();
}

extern uiCrossoverExports_t *trap_CO_InitCrossover( cgCrossoverExports_t *uiImport );
void CG_InitializeCrossoverAPI( void )
{
	co.GetBlueTeam = CO_GetBlueTeam;
	co.GetRedTeam = CO_GetRedTeam;
	co.EscapeTrapped = CO_EscapeTrapped;
	co.GetWeaponDatas = GetWeaponData;
	co.InventoryAttachToACI = CO_InventoryAttachToACI;
	co.InventoryDataRequest = CO_InventoryDataRequest;
	co.PartyMngtDataRequest = CO_PartyMngtDataRequest;
	co.SendClientCommand = CO_SendClientCommand;

	uiImports = trap_CO_InitCrossover( &co );
}