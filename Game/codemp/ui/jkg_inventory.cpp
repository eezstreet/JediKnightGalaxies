#include "ui_local.h"
#include "jkg_inventory.h"
#include <expat.h>
//Just for this file since it has a lot of strcpy to const types ~eezstreet
#pragma warning (disable:4090)

static cgItemInstance_t itemsInFilter[256];

void JKG_Inventory_UpdateVisuals( void );
void JKG_Inventory_Examine_Button ( int forceOff );

struct
{
    qboolean active;
	qboolean inShop;
	qboolean ACIopen;
    menuDef_t *menu;
    cgItemInstance_t *selectedItem;
	int selectedItemIndex;
	qboolean examineMenuOpen;
} inventoryState;

#define MAX_INVENTORY_WEIGHT	50 //NOTENOTE: if you change this here, be sure to do the same in game

/*
======================
BG_GetWeaponDataFromStr

Given an index, variation and string, it will attempt to find that information
======================
*/
#ifndef QAGAME
#define DWORD unsigned long
weaponDataGrab_t BG_GetWeaponDataFromStr(int weapon, int variation, char *text)
{
	weaponData_t *weaponData = cgImports->GetWeaponDatas( weapon, variation );
	weaponDataGrab_t retVal;
	memset(&retVal, 0, sizeof(weaponDataGrab_t));


	//Next, check for various text strings
	if(!Q_stricmp(text, "name"))
	{
		//retVal.data = weaponData->displayName;
		retVal.data.a = (char *)malloc(256);
		strcpy(retVal.data.a, weaponData->displayName);
		retVal.isString = qtrue;
		return retVal;
	}
	else if(!Q_stricmp(text, "classname"))
	{
		retVal.data.a = (char *)malloc(256);
		strcpy(retVal.data.a, weaponData->classname);
		retVal.isString = qtrue;
		return retVal;
	}
	else if(!Q_stricmp(text, "ammoIndex"))
	{
//		retVal.data = (void *)&weaponData->ammoIndex;
		retVal.data.uc = weaponData->ammoIndex;
		retVal.byteCount = 1;
		return retVal;
	}
	else if(!Q_stricmp(text, "hasCookAbility"))
	{
		//retVal.data = (void *)&weaponData->hasCookAbility;
		retVal.data.uc = weaponData->hasCookAbility;
		retVal.byteCount = 1;
		return retVal;
	}
	else if(!Q_stricmp(text, "hasKnockBack"))
	{
		//retVal.data = (void *)&weaponData->hasKnockBack;
		retVal.data.uc = weaponData->hasKnockBack;
		retVal.byteCount = 1;
		return retVal;
	}
	else if(!Q_stricmp(text, "hasRollAbility"))
	{
		//retVal.data = (void *)&weaponData->hasRollAbility;
		retVal.data.uc = weaponData->hasRollAbility;
		retVal.byteCount = 1;
		return retVal;
	}
	/*else if(!Q_stricmp(text, "hasSecondary"))
	{
		//retVal.data = (void *)&weaponData->hasSecondary;
		retVal.data.uc = weaponData->hasSecondary;
		retVal.byteCount = 1;
		return retVal;
	}*/
	else if(!Q_stricmp(text, "zoomType"))
	{
		//retVal.data = (void *)&weaponData->zoomType;
		retVal.data.uc = weaponData->zoomType;
		retVal.byteCount = 1;
		return retVal;
	}
	else if(!Q_stricmp(text, "startZoomFov"))
	{
		/*float fData = weaponData->startZoomFov;
		DWORD dwVal = *(DWORD *)&fData;
		retVal.data = (void *)fData;*/
		retVal.data.f = weaponData->startZoomFov;
		retVal.isFloat = qtrue;
		return retVal;
	}
	else if(!Q_stricmp(text, "endZoomFov"))
	{
		/*float fData = weaponData->endZoomFov;
		DWORD dwVal = *(DWORD *)&fData;
		retVal.data = (void *)fData;*/
		retVal.data.f = weaponData->endZoomFov;
		retVal.isFloat = qtrue;
		return retVal;
	}
	else if(!Q_stricmp(text, "zoomTime"))
	{
		/*float fData = weaponData->zoomTime;
		DWORD dwVal = *(DWORD *)&fData;
		retVal.data = (void *)fData;*/
		retVal.data.f = weaponData->zoomTime;
		retVal.byteCount = 1;
		return retVal;
	}
	else if(!Q_stricmp(text, "speedModifier"))
	{
		/*float fData = weaponData->speedModifier;
		DWORD dwVal = *(DWORD *)&fData;
		retVal.data = (void *)fData;*/
		retVal.data.f = weaponData->speedModifier;
		retVal.isFloat = qtrue;
		return retVal;
	}
	else if(!Q_stricmp(text, "reloadModifier"))
	{
		/*float fData = weaponData->reloadModifier;
		DWORD dwVal = *(DWORD *)&fData;
		retVal.data = (void *)fData;*/
		retVal.data.f = weaponData->reloadModifier;
		retVal.isFloat = qtrue;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm1-baseDamage"))
	{
		//retVal.data = (void *)&weaponData->firemodes[0].baseDamage;
		retVal.data.i = weaponData->firemodes[0].baseDamage;
		retVal.byteCount = 2;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm2-baseDamage"))
	{
		//retVal.data = (void *)&weaponData->firemodes[1].baseDamage;
		retVal.data.i = weaponData->firemodes[1].baseDamage;
		retVal.byteCount = 2;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm1-applyGravity"))
	{
		//retVal.data = (void *)&weaponData->firemodes[0].applyGravity;
		retVal.data.uc = weaponData->firemodes[0].applyGravity;
		retVal.byteCount = 1;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm2-applyGravity"))
	{
		//retVal.data = (void *)&weaponData->firemodes[1].applyGravity;
		retVal.data.uc = weaponData->firemodes[1].applyGravity;
		retVal.byteCount = 1;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm1-hitscan"))
	{
		//retVal.data = (void *)&weaponData->firemodes[0].hitscan;
		retVal.data.uc = weaponData->firemodes[0].hitscan;
		retVal.byteCount = 1;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm2-hitscan"))
	{
		//retVal.data = (void *)&weaponData->firemodes[1].hitscan;
		retVal.data.uc = weaponData->firemodes[1].hitscan;
		retVal.byteCount = 1;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm1-shotCount"))
	{
		//retVal.data = (void *)&weaponData->firemodes[0].shotCount;
		retVal.data.uc = weaponData->firemodes[0].shotCount;
		retVal.byteCount = 1;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm2-shotCount"))
	{
		//retVal.data = (void *)&weaponData->firemodes[1].shotCount;
		retVal.data.uc = weaponData->firemodes[1].shotCount;
		retVal.byteCount = 1;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm1-cost"))
	{
		//retVal.data = (void *)&weaponData->firemodes[0].cost;
		retVal.data.uc = weaponData->firemodes[0].cost;
		retVal.byteCount = 1;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm2-cost"))
	{
		//retVal.data = (void *)&weaponData->firemodes[1].cost;
		retVal.data.uc = weaponData->firemodes[1].cost;
		retVal.byteCount = 1;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm1-delay"))
	{
		//retVal.data = (void *)&weaponData->firemodes[0].delay;
		retVal.data.i = weaponData->firemodes[0].delay;
		retVal.byteCount = 2;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm2-delay"))
	{
		//retVal.data = (void *)&weaponData->firemodes[1].delay;
		retVal.data.i = weaponData->firemodes[1].delay;
		retVal.byteCount = 2;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm1-delaypm"))
	{
		float pm = (float)(1000 / weaponData->firemodes[0].delay) * 6;
		//strcpy((char *)retVal.data, va("%i", (short)pm));
		//retVal.data = (void *)(((float)weaponData->firemodes[0].delay / 1000)*60);
		retVal.data.f = pm;
		retVal.isFloat = qtrue;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm2-delaypm"))
	{
		float pm = (float)(1000 / weaponData->firemodes[1].delay) * 6;
		//strcpy((char *)retVal.data, va("%i", (short)pm));
		//retVal.data = (void *)&(((float)weaponData->firemodes[1].delay / 1000)*60);
		retVal.data.f = pm;
		retVal.byteCount = 2;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm1-range"))
	{
		retVal.data.f = weaponData->firemodes[0].range;
		retVal.isFloat = qtrue;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm2-range"))
	{
		retVal.data.f = weaponData->firemodes[1].range;
		retVal.isFloat = qtrue;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm1-rangeSplash"))
	{
		retVal.data.f = weaponData->firemodes[0].rangeSplash;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm2-rangeSplash"))
	{
		retVal.data.f = weaponData->firemodes[1].rangeSplash;
		retVal.isFloat = qtrue;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm1-recoil"))
	{
		retVal.data.f = weaponData->firemodes[0].recoil;
		retVal.isFloat = qtrue;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm2-recoil"))
	{
		retVal.data.f = weaponData->firemodes[1].recoil;
		retVal.isFloat = qtrue;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm1-spread"))
	{
		//retVal.data.f = weaponData->firemodes[0].spread;
		retVal.data.f = 0;
		retVal.isFloat = qtrue;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm2-spread"))
	{
		//retVal.data.f = weaponData->firemodes[1].spread;
		retVal.data.f = 0;
		retVal.isFloat = qtrue;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm1-vel"))
	{
		retVal.data.f = weaponData->firemodes[0].speed/48;
		retVal.isFloat = qtrue;
		return retVal;
	}
	else if(!Q_stricmp(text, "fm2-vel"))
	{
		retVal.data.f = weaponData->firemodes[1].speed/48;
		retVal.isFloat = qtrue;
		return retVal;
	}
	return retVal;
}
#endif

void JKG_Inventory_ConstructCreditsText( void )
{
	itemDef_t *creditItem = Menu_FindItemByName(inventoryState.menu, "invmain_credits");
	int credits = (int)cgImports->InventoryDataRequest( 3 );
	sprintf(creditItem->text, "%i", credits);
}

void JKG_Inventory_ConstructWeightText( void )
{
	cgItemInstance_t *inventory = (cgItemInstance_t *)cgImports->InventoryDataRequest( 1 );
	itemDef_t *weightItem = Menu_FindItemByName(inventoryState.menu, "invmain_weight");
	int numItems = *(int *)cgImports->InventoryDataRequest( 0 );
	int i, weight=0;
	char completeString[10];

	if(!weightItem)
	{
		return;
	}

	for(i = 0; i < numItems; i++)
	{
		if(!inventory[i].id)
		{
			break;
		}
		weight += inventory[i].id->weight;
	}
	sprintf(completeString, "%i / %i", weight, MAX_INVENTORY_WEIGHT);

	strcpy((char *)weightItem->text, completeString); //Bad const, I know.

	JKG_Inventory_ConstructCreditsText();
}

void JKG_Inventory_OpenDialog ( char **args )
{
    trap_Cvar_Set ("ui_hidehud", "1");
    inventoryState.active = qtrue;
	inventoryState.ACIopen = qtrue;
    
    Menu_ClearFocus (inventoryState.menu);
    Menu_ItemDisable (inventoryState.menu, "main_dialog", qfalse);

	Menu_ShowGroup (inventoryState.menu, "aci_selection_na", qfalse);
	Menu_ShowGroup (inventoryState.menu, "destroyMenu", qfalse);
	JKG_Inventory_ConstructWeightText();

	//Do special stuff depending on whether we're opening it from a shop menu
	if(inventoryState.inShop)
	{
		Menu_ShowItemByName (inventoryState.menu, "inv_exit", qfalse);
		Menu_ShowItemByName (inventoryState.menu, "inv_exit_alt", qtrue);
		Menu_ShowGroup(inventoryState.menu, "shoptab", qtrue);
		Menu_ShowItemByName (inventoryState.menu, "shoptab_highlight", qfalse);
		Menu_ShowItemByName (inventoryState.menu, "btn_destroy", qfalse);
		Menu_ShowItemByName (inventoryState.menu, "btn_sell", qtrue);
	}
	else
	{
		Menu_ShowItemByName (inventoryState.menu, "inv_exit", qtrue);
		Menu_ShowItemByName (inventoryState.menu, "inv_exit_alt", qfalse);
		Menu_ShowGroup(inventoryState.menu, "shoptab", qfalse);
		Menu_ShowItemByName (inventoryState.menu, "btn_destroy", qtrue);
		Menu_ShowItemByName (inventoryState.menu, "btn_sell", qfalse);
	}
	// Begone, examine menu!
	JKG_Inventory_Examine_Button(qtrue);
}

void JKG_Inventory_CloseFromShop( char **args )
{
	inventoryState.ACIopen = qfalse;
	inventoryState.examineMenuOpen = qfalse;
	Menus_CloseByName(inventoryState.menu->window.name);
}

void JKG_Inventory_CloseDialog ( char **args )
{
	if(!inventoryState.inShop)
	{
		trap_Cvar_Set ("ui_hidehud", "0");
	}
    inventoryState.active = qfalse;
	inventoryState.examineMenuOpen = qfalse;

	Menu_ShowGroup (inventoryState.menu, "aci_selection_na", qfalse);
	Menu_ShowGroup (inventoryState.menu, "destroyMenu", qfalse);

	// Do special stuff if we were in a shop menu before
	if(inventoryState.inShop)
	{
		JKG_Shop_RestoreShopMenu();
	}
	inventoryState.inShop = qfalse;
	inventoryState.ACIopen = qfalse;
	//JKG_Shop_UpdateNotify(1);
}

extern int Item_ListBox_MaxScroll ( itemDef_t *item );
extern displayContextDef_t *DC;
void JKG_Inventory_Arrow ( char **args )
{
	const char *name;
	itemDef_t *item = Menu_FindItemByName(inventoryState.menu, "inventory_feederstuff");
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
	int max = Item_ListBox_MaxScroll(item);
	int viewmax = (item->window.rect.w / listPtr->elementWidth);
	int numItems = *(int *)cgImports->InventoryDataRequest( 0 );
	int amount = atoi(args[0]);

	if(numItems <= 0)
	{
		//This doesn't work if we have no items in our inventory ~eez
		return;
	}

	if(amount >= 2)
	{
		amount = -1;
	}

	if (String_Parse(args, &name))
	{
		if (( item->window.rect.h > (listPtr->elementHeight*2)) &&  (listPtr->elementStyle == LISTBOX_IMAGE))
		{
			viewmax = (item->window.rect.h / listPtr->elementHeight);
		}
		else 
		{
			viewmax = (item->window.rect.w / listPtr->elementWidth);
		}
		if(amount < 0)
		{
			listPtr->cursorPos = listPtr->startPos;
			listPtr->cursorPos += amount;
		}
		else if(amount > 0)
		{
			listPtr->startPos += amount;
			listPtr->cursorPos = listPtr->startPos;
		}
		if (listPtr->cursorPos < 0) {
			listPtr->cursorPos = 0;
			return;
		}
		if (listPtr->cursorPos < listPtr->startPos) {
			listPtr->startPos = listPtr->cursorPos;
		}
		if (listPtr->cursorPos >= listPtr->startPos + viewmax) {
			listPtr->startPos = listPtr->cursorPos - viewmax + 1;
		}
		if (listPtr->startPos >= DC->feederCount(item->special))
		{
			listPtr->startPos = DC->feederCount(item->special)-1;
		}
		if (listPtr->cursorPos > DC->feederCount(item->special))
		{
			listPtr->cursorPos = DC->feederCount(item->special);
		}
		item->cursorPos = listPtr->cursorPos;
	}
	JKG_Inventory_UpdateVisuals();
}

void JKG_Inventory_Arrow_New ( itemDef_t *item, int amount )
{
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
	int max = Item_ListBox_MaxScroll(item);
	int viewmax = (item->window.rect.w / listPtr->elementWidth);
	int numItems = *(int *)cgImports->InventoryDataRequest( 0 );

	if(numItems <= 0)
	{
		//This doesn't work if we have no items in our inventory ~eez
		return;
	}

	{
		if (( item->window.rect.h > (listPtr->elementHeight*2)) &&  (listPtr->elementStyle == LISTBOX_IMAGE))
		{
			viewmax = (item->window.rect.h / listPtr->elementHeight);
		}
		else 
		{
			viewmax = (item->window.rect.w / listPtr->elementWidth);
		}
		if(amount < 0)
		{
			listPtr->cursorPos = listPtr->startPos;
			listPtr->cursorPos += amount;
		}
		else if(amount > 0)
		{
			listPtr->startPos += amount;
			listPtr->cursorPos = listPtr->startPos;
		}
		if (listPtr->cursorPos < 0) {
			listPtr->cursorPos = 0;
			return;
		}
		if (listPtr->cursorPos < listPtr->startPos) {
			listPtr->startPos = listPtr->cursorPos;
		}
		if (listPtr->cursorPos >= listPtr->startPos + viewmax) {
			listPtr->startPos = listPtr->cursorPos - viewmax + 1;
		}
		if (listPtr->startPos >= DC->feederCount(item->special))
		{
			listPtr->startPos = DC->feederCount(item->special)-1;
		}
		if (listPtr->cursorPos > DC->feederCount(item->special))
		{
			listPtr->cursorPos = DC->feederCount(item->special);
		}
		item->cursorPos = listPtr->cursorPos;
	}
	JKG_Inventory_UpdateVisuals();
}

int JKG_Inventory_FeederCount ( void )
{
	int numItems = *(int *)cgImports->InventoryDataRequest( 0 );
	cgItemInstance_t *inventory = (cgItemInstance_t *)cgImports->InventoryDataRequest( 1 );
	int c = 0;
	int i;
	memset(itemsInFilter, 0, sizeof(itemsInFilter));
	if(ui_inventoryFilter.integer == JKGIFILTER_ALL)
	{
		return numItems;
	}
	else
	{
		for(i = 0; i < numItems; i++)
		{
			if(!inventory[i].id)
			{
				return 0;
			}
			switch(ui_inventoryFilter.integer)
			{
				case JKGIFILTER_ARMOR:
					if(inventory[i].id->itemType == ITEM_ARMOR || inventory[i].id->itemType == ITEM_CLOTHING)
					{
						itemsInFilter[c] = inventory[i];
						itemsInFilter[c].amount[0] = i; //HAX
						c++;
					}
					break;
				case JKGIFILTER_WEAPONS:
					if(inventory[i].id->itemType == ITEM_WEAPON)
					{
						itemsInFilter[c] = inventory[i];
						itemsInFilter[c].amount[0] = i; //HAX
						c++;
					}
					break;
				case JKGIFILTER_CONSUMABLES:
					if(inventory[i].id->itemType == ITEM_BUFF)
					{
						itemsInFilter[c] = inventory[i];
						itemsInFilter[c].amount[0] = i; //HAX
						c++;
					}
					break;
				case JKGIFILTER_MISC:
					if(inventory[i].id->itemType == ITEM_UNKNOWN)
					{
						itemsInFilter[c] = inventory[i];
						itemsInFilter[c].amount[0] = i; //HAX
						c++;
					}
					break;
			}
		}
	}
	return c;
}

static int GetACISlotForWeapon ( int weaponId )
{
	cgItemInstance_t *inventory = (cgItemInstance_t *)cgImports->InventoryDataRequest( 1 );
	int *playerACI = (int *)cgImports->InventoryDataRequest( 2 );
    int i;
    for ( i = 0; i < MAX_ACI_SLOTS; i++ )
    {
		if (playerACI[i] >= 0)
		{
			if(inventory[playerACI[i]].id)
			{
				if(inventory[playerACI[i]].id->varID == weaponId)
				{
					return i;
				}
			}
		}
    }
    
    return -1;
}

static qboolean IsWeaponInACI ( int weaponId )
{
    return GetACISlotForWeapon (weaponId) != -1;
}

static int GetACISlotForItem ( int itemID )
{
	cgItemInstance_t *inventory = (cgItemInstance_t *)cgImports->InventoryDataRequest( 1 );
	int *playerACI = (int *)cgImports->InventoryDataRequest( 2 );
	int i;
    for ( i = 0; i < MAX_ACI_SLOTS; i++ )
    {
		if ( playerACI[i] >= 0 )
        {
			if( inventory[playerACI[i]].id->itemID == itemID )
			{
				return i;
			}
        }
    }
    
    return -1;
}

static qboolean IsItemInACI ( int weaponId )
{
    return GetACISlotForItem (weaponId) != -1;
}

void UpdateButtonStates ( void )
{
    qboolean equipped;

	if ( !inventoryState.menu )
	{
		return;
	}
    
	if ( !inventoryState.active || !inventoryState.selectedItem || !inventoryState.selectedItem->id )
    {
		if(inventoryState.inShop)
		{
			Menu_ItemDisable (inventoryState.menu, "btn_sell", qtrue);
		}
		inventoryState.selectedItem = NULL;
		inventoryState.selectedItemIndex = -1;
		Menu_ShowGroup(inventoryState.menu, "itemselections", qfalse); //Hide currently selected item
		Menu_ItemDisable (inventoryState.menu, "btn_equip", qtrue);
		Menu_ItemDisable (inventoryState.menu, "btn_unequip", qtrue);
		Menu_ItemDisable (inventoryState.menu, "btn_useitem", qtrue);
		Menu_ItemDisable (inventoryState.menu, "btn_aci", qtrue);
		Menu_ItemDisable (inventoryState.menu, "btn_examine", qtrue);
        return;
    }
    
    equipped = inventoryState.selectedItem->equipped;

	if( !inventoryState.selectedItem->id->xml )
	{
		Menu_ShowItemByName (inventoryState.menu, "btn_na1", qtrue);
		Menu_ItemDisable (inventoryState.menu, "btn_examine", 1);
	}
	else
	{
		Menu_ShowItemByName (inventoryState.menu, "btn_na1", qfalse);
		Menu_ItemDisable (inventoryState.menu, "btn_examine", 0);
	}

    switch ( inventoryState.selectedItem->id->itemType )
    {
        case ITEM_ARMOR:
            Menu_ShowItemByName (inventoryState.menu, "btn_equip", !equipped);
            Menu_ShowItemByName (inventoryState.menu, "btn_unequip", equipped);
            
            Menu_ItemDisable (inventoryState.menu, "btn_equip", equipped);
            Menu_ItemDisable (inventoryState.menu, "btn_unequip", !equipped);
            
			Menu_ShowItemByName (inventoryState.menu, "btn_na3", qtrue);
            Menu_ItemDisable (inventoryState.menu, "btn_useitem", 1);
            Menu_ItemDisable (inventoryState.menu, "btn_aci", 1);
            
			Menu_ShowItemByName (inventoryState.menu, "btn_na2", qtrue);
            Menu_ItemDisable (inventoryState.menu, "btn_aci2", 1);
            Menu_ShowItemByName (inventoryState.menu, "btn_aci2", qfalse);
			if(inventoryState.inShop)
			{
				Menu_ItemDisable (inventoryState.menu, "btn_sell", equipped);
			}
			else
			{
				Menu_ItemDisable (inventoryState.menu, "btn_destroy", equipped);
			}
        break;
        
        case ITEM_WEAPON:
        {
            qboolean weaponInACI = IsWeaponInACI (inventoryState.selectedItem->id->varID);
        
			Menu_ShowItemByName (inventoryState.menu, "btn_na4", qtrue);
            Menu_ShowItemByName (inventoryState.menu, "btn_equip", qtrue);
            Menu_ShowItemByName (inventoryState.menu, "btn_unequip", qfalse);
            
            Menu_ItemDisable (inventoryState.menu, "btn_equip", qtrue);
            Menu_ItemDisable (inventoryState.menu, "btn_unequip", qtrue);
            
			Menu_ShowItemByName (inventoryState.menu, "btn_na3", qtrue);
            Menu_ItemDisable (inventoryState.menu, "btn_useitem", 1);
            
            Menu_ShowItemByName (inventoryState.menu, "btn_aci", !weaponInACI);
            Menu_ShowItemByName (inventoryState.menu, "btn_aci2", weaponInACI);
            
            Menu_ItemDisable (inventoryState.menu, "btn_aci", weaponInACI);
            Menu_ItemDisable (inventoryState.menu, "btn_aci2", !weaponInACI);

			if(inventoryState.inShop)
			{
				Menu_ItemDisable (inventoryState.menu, "btn_sell", weaponInACI);
			}
			else
			{
				Menu_ItemDisable (inventoryState.menu, "btn_destroy", weaponInACI);
			}
        }
        break;
        
        default:
			{
				qboolean itemInACI = IsItemInACI (inventoryState.selectedItem->id->itemID);
				qboolean itemACIAble = (inventoryState.selectedItem->id->pSpell > 0);
				//TODO: add consumable stuff
				Menu_ItemDisable (inventoryState.menu, "btn_useitem", 0);
				Menu_ItemDisable (inventoryState.menu, "btn_equip", 1);
				Menu_ItemDisable (inventoryState.menu, "btn_unequip", 1);
				Menu_ShowItemByName (inventoryState.menu, "btn_equip", qtrue);
				Menu_ShowItemByName (inventoryState.menu, "btn_unequip", qfalse);

				if(itemInACI)
				{
					Menu_ShowItemByName (inventoryState.menu, "btn_aci", qfalse);
					Menu_ItemDisable (inventoryState.menu, "btn_aci2", qfalse);
					Menu_ItemDisable (inventoryState.menu, "btn_aci", qtrue);
					Menu_ShowItemByName (inventoryState.menu, "btn_aci2", qtrue);
				}
				else
				{
					Menu_ShowItemByName (inventoryState.menu, "btn_aci", qtrue);
					Menu_ShowItemByName (inventoryState.menu, "btn_aci2", qfalse);
					Menu_ItemDisable (inventoryState.menu, "btn_aci", !itemACIAble);
					Menu_ItemDisable (inventoryState.menu, "btn_aci2", qtrue);
				}

				//Menu_ShowItemByName (inventoryState.menu, "btn_aci", qtrue);
				//Menu_ShowItemByName (inventoryState.menu, "btn_aci2", qfalse);
				//Menu_ItemDisable (inventoryState.menu, "btn_aci", 1);
				if(inventoryState.inShop)
				{
					Menu_ItemDisable (inventoryState.menu, "btn_sell", qfalse);
				}
				else
				{
					Menu_ItemDisable (inventoryState.menu, "btn_destroy", qfalse);
				}
			}
        break;
    }
}

qboolean JKG_Inventory_FeederSelection ( int index )
{
    int numItems = JKG_Inventory_FeederCount();
	itemDef_t *item = Menu_FindItemByName(inventoryState.menu, "inventory_feederstuff");
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
    cgItemInstance_t *inventory = NULL;
    
	//Sanity check
    if ( index < 0 || index >= numItems )
    {
        return qfalse;
    }
    
	// Get information on the item
	if(ui_inventoryFilter.integer == JKGIFILTER_ALL)
	{
		inventory = (cgItemInstance_t *)cgImports->InventoryDataRequest( 1 );
		inventoryState.selectedItem = &inventory[index];
		inventoryState.selectedItemIndex = index;
	}
	else
	{
		cgItemInstance_t *inventory2 = (cgItemInstance_t *)cgImports->InventoryDataRequest( 1 );
		inventory = itemsInFilter;
		inventoryState.selectedItem = &itemsInFilter[index];
		inventoryState.selectedItemIndex = index;
	}

	// Hide NA buttons
	Menu_ShowGroup(inventoryState.menu, "btn_nas", qfalse);
    
	// Update the "equip", "destroy" etc buttons to more appropriate stylings
    UpdateButtonStates();

	// Show the special selection graphic that BlasTech made ~eezstreet
	Menu_ShowGroup(inventoryState.menu, "itemselections", qfalse); //Hide currently selected item
	Menu_ShowItemByName(inventoryState.menu, va("feeder_selection%i", index-listPtr->startPos), qtrue);
	// Hack: hide the button highlights (this doesn't bring them out of focus however, which is good)
	Menu_ShowGroup(inventoryState.menu, "btn_hilights", qfalse);

    
    return qtrue;
}

const char *JKG_Inventory_FeederItemText ( int index, int column, qhandle_t *handle1, qhandle_t *handle2, qhandle_t *handle3 )
{
	int numItems = *(int *)cgImports->InventoryDataRequest( 0 );
    cgItemInstance_t *inventory;
    
    if ( index < 0 || index >= numItems )
    {
        return NULL;
    }
    
	inventory = (cgItemInstance_t *)cgImports->InventoryDataRequest( 1 );
    switch ( column )
    {
        case 0:
            return inventory[index].id->displayName;
        case 1:
            return inventory[index].equipped ? "Y" : NULL;
        case 2:
            return va ("%i", inventory[index].id->weight);
        default:
            return NULL;
    }
}

void JKG_Inventory_UpdateNotify(int msg) {
    switch ( msg )
    {
        case 0: // open
            memset (&inventoryState, 0, sizeof (inventoryState));
            inventoryState.active = qtrue;
			inventoryState.inShop = qfalse;
            
            trap_Syscall_UI();
            inventoryState.menu = Menus_FindByName ("jkg_inventory");
            if ( inventoryState.menu && Menus_ActivateByName ("jkg_inventory") )
            {
                trap_Key_SetCatcher (trap_Key_GetCatcher() | KEYCATCH_UI & ~KEYCATCH_CONSOLE);
            }
			trap_Syscall_CG();
        break;
        
        case 1: // update!
            UpdateButtonStates();
			JKG_Inventory_UpdateVisuals();
			JKG_Inventory_ConstructCreditsText();
			break;
		case 2:	// open as shop menu
			memset (&inventoryState, 0, sizeof (inventoryState));
            inventoryState.active = qtrue;
			inventoryState.inShop = qtrue;
            
			trap_Syscall_UI();
            inventoryState.menu = Menus_FindByName ("jkg_inventory");
            if ( inventoryState.menu && Menus_ActivateByName ("jkg_inventory") )
            {
                trap_Key_SetCatcher (trap_Key_GetCatcher() | KEYCATCH_UI & ~KEYCATCH_CONSOLE);
            }
			trap_Syscall_CG();
        break;
    }
}

enum inventoryButtons_e
{
    INV_BTN_USEITEM,
    INV_BTN_EQUIP,
    INV_BTN_ASSIGN2ACI,
    INV_BTN_UNEQUIP,
    INV_BTN_UNASSIGN4ACI,
	INV_BTN_DESTROYITEM,
	INV_BTN_DESTROYITEM_CONFIRM,
	INV_BTN_DESTROYITEM_DENY,
	INV_BTN_EXAMINE,
	INV_BTN_SELL,
	INV_BTN_SELL_CONFIRM,
	INV_BTN_SELL_DENY,
};

void JKG_Inventory_ACI_Button ( char **args )
{
    int slot;
	cgItemInstance_t *inventory = (cgItemInstance_t *)cgImports->InventoryDataRequest( 1 );
    if ( !Int_Parse (args, &slot) )
    {
        return;
    }
    
    if ( slot < 0 || slot >= MAX_ACI_SLOTS )
    {
        return;
    }


	if(!inventory)
	{
		return;
	}

	if(!inventoryState.selectedItem)
	{
		return;
	}

    if(ui_inventoryFilter.integer != JKGIFILTER_ALL)
	{
		cgImports->InventoryAttachToACI (inventoryState.selectedItem->amount[0], slot, 1);	// eezstreet: cannot simply do item-inventory because filters use different pointers!
	}
	else
	{
		cgImports->InventoryAttachToACI (inventoryState.selectedItem-inventory, slot, 1);
	}
    
    Menu_ShowGroup (inventoryState.menu, "aci_selection", qfalse);
    Menu_ClearFocus (inventoryState.menu);
    
    Menu_ItemDisable (inventoryState.menu, "main_dialog", qfalse);
	Menu_ItemDisable (inventoryState.menu, "arrows", qfalse);

	Menu_ShowGroup (inventoryState.menu, "aci_selection_na", qfalse);
	inventoryState.ACIopen = qfalse;
    UpdateButtonStates();
	JKG_Inventory_UpdateVisuals();
}
void JKG_Inventory_ACI_Button_Clean(int slot)
{
	cgItemInstance_t *inventory = (cgItemInstance_t *)cgImports->InventoryDataRequest( 1 );
    
    if ( slot < 0 || slot >= MAX_ACI_SLOTS )
    {
        return;
    }

	if(!inventory)
	{
		return;
	}

	if(!inventoryState.selectedItem)
	{
		return;
	}
    
	if(ui_inventoryFilter.integer)
		cgImports->InventoryAttachToACI (inventoryState.selectedItem->amount[0], slot, 1);	// eezstreet: cannot simply do item-inventory because filters use different pointers!
	else
		cgImports->InventoryAttachToACI (inventoryState.selectedItem-inventory, slot, 1);
    
    Menu_ShowGroup (inventoryState.menu, "aci_selection", qfalse);
    Menu_ClearFocus (inventoryState.menu);
    
    Menu_ItemDisable (inventoryState.menu, "main_dialog", qfalse);
	Menu_ItemDisable (inventoryState.menu, "arrows", qfalse);

	Menu_ShowGroup (inventoryState.menu, "aci_selection_na", qfalse);
	inventoryState.ACIopen = qfalse;
    UpdateButtonStates();
	JKG_Inventory_UpdateVisuals();
}

//eezstreet: New function for showing NA greyovers and disabling NA buttons
static void JKG_ACI_GreyBlast(void)
{
	int i;
	cgItemInstance_t *inventory = (cgItemInstance_t *)cgImports->InventoryDataRequest( 1 );
	int *playerACI = (int *)cgImports->InventoryDataRequest( 2 );

	if(!inventory)
	{
		return;
	}

	Menu_ShowGroup (inventoryState.menu, "aci_selection_na", qfalse);
	for(i = 0; i < MAX_ACI_SLOTS; i++)
	{
		if(playerACI[i] < 0 || !inventory[playerACI[i]].id || !inventory[playerACI[i]].id->itemID)
		{
			//No item in slot
			Menu_ItemDisable (inventoryState.menu, va("btn_aci_slot%i", i), qfalse);
			continue; //Unsure..I think this means when there's no item in slot, but could be a null ptr..
		}
		//Special case for slot number 10..it uses 0s
		/*if(i == 9)
		{
			Menu_ShowItemByName(inventoryState.menu, "btn_aci_slot0_na", qtrue);
			Menu_ItemDisable (inventoryState.menu, "btn_aci_slot0", qtrue);
		}
		else
		{*/
			Menu_ShowItemByName(inventoryState.menu, va("btn_aci_slot%i_na", i), qtrue);
			Menu_ItemDisable (inventoryState.menu, va("btn_aci_slot%i", i), qtrue);
		//}
			// Wow, was I drunk when I wrote the above, or what?
	}
}


#pragma region XML
#pragma region XMLdecl
//===============
// XML Handling
//===============

static int		last_tableline;
#pragma endregion

#pragma region XML Element Start
static void XMLCALL JKGXML_Start_Element(void *userData, const char *name, const char **atts)
{
	int *depthPtr = (int *)userData;
	int itemType = inventoryState.selectedItem->id->itemType;
	if(!Q_stricmp(name, "test"))
	{
		Com_Printf("^2XML ok\n");
		*depthPtr += 1;
		return;
	}
	if(*depthPtr == 0)
	{
		if(!Q_stricmp(name, "ItemXML"))
		{
			*depthPtr += 1;
			return;
		}
	}
	else if(*depthPtr == 1)
	{
		//Global scope
		containingCat++;
		lastDepth2 = 1;
		if(containingCat == 3)
		{
			//There are special graphics on the last category, attributes providing.
			if(!Q_stricmp(atts[0], "type"))
			{
				if(!Q_stricmp(atts[1], "twocoltable"))
				{
					//Loop through next attributes
					int i;
					for(i = 2; atts[i]; i += 2)
					{
						if(!Q_stricmp(atts[i], "col1"))
						{
							itemDef_t *item = Menu_FindItemByName(inventoryState.menu, va("examine%iCat3_tableHeaders",
								itemType));
							if(!item)
							{
								return;
							}

							strcpy(item->text, atts[i+1]);
						}
						else if(!Q_stricmp(atts[i], "col2"))
						{
							itemDef_t *item = Menu_FindItemByName(inventoryState.menu, va("examine%iCat3_tableHeaders",
								itemType));
							if(!item)
							{
								return;
							}

							strcpy(item->text2, atts[i+1]);
						}
					}
				}
				last_tableline = 1;
			}
		}
		/*if(!Q_stricmp(name, "ProductionInfo"))
		{
			containingCat = CONTAINCAT_PRODUCTIONINFO;
		}
		else if(!Q_stricmp(name, "Usage"))
		{
			containingCat = CONTAINCAT_USAGE;
		}
		else if(!Q_stricmp(name, "TechnicalSpecifications"))
		{
			containingCat = CONTAINCAT_TECHSPEC;
		}*/
	}
	else if(*depthPtr == 2 || *depthPtr == 3)
	{
		int numAttributes = 0;
		int wp;
		int variation;
		int i;
		weaponDataGrab_t wpG;
		for(i = 0; atts[i]; i += 2)
		{
			numAttributes++;
		}
		memset(&wpG, 0, sizeof(weaponDataGrab_t));
		wp = inventoryState.selectedItem->id->weapon;
		variation = inventoryState.selectedItem->id->variation;
		if(numAttributes > 0)
		{
			wpG = BG_GetWeaponDataFromStr(wp, variation, const_cast<char *>(atts[1]));
		}
		formatStr[0] = '\0';

		if(!Q_stricmp(atts[0], "filler"))
		{
			//if(wpG.data)
			{
				if(!Q_stricmp(atts[2], "format"))
				{
					strcpy(formatStr, atts[3]);
				}
			}
		}
		//Within another tag - first level
		if(!containingCat)
		{
			//Hmmm..Are you using the <test> tags wrong??
			*depthPtr += 1;
			return;
		}
		// Check the itemType
		if(numAttributes > 0)
		{
			useAutoFill = qtrue;
			autoFill = wpG;
		}
		else
		{
			useAutoFill = qfalse;

		}
	}
	*depthPtr += 1;
}
#pragma endregion

#pragma region JKG_Inventory_ExamineMenuNumForItemType
int JKG_Inventory_ExamineMenuNumForItemType(const int itemType)
{
	switch(itemType)
	{
		case ITEM_WEAPON:
			return 1;
			break;
		case ITEM_ARMOR:
		case ITEM_CLOTHING:
			return 2;
			break;
		case ITEM_BUFF:
		case ITEM_UNKNOWN:
			return 3;
			break;
	}
	return 0;
}
#pragma endregion

#pragma region XML Element End
static void XMLCALL JKGXML_End_Element(void *userData, const char *name)
{
	int *depthPtr = (int *)userData;
	itemDef_t *item = Menu_FindItemByName(inventoryState.menu, va("examine%iCat%i_text%i", JKG_Inventory_ExamineMenuNumForItemType(inventoryState.selectedItem->id->itemType),
		lastCat, lastDepth2));
	
	if(*depthPtr == 2)
	{
		item = Menu_FindItemByName(inventoryState.menu, va("examine%iCat%i_title", JKG_Inventory_ExamineMenuNumForItemType(inventoryState.selectedItem->id->itemType),
					lastCat));
		/*switch(containingCat)
		{
			case CONTAINCAT_PRODUCTIONINFO:
				strcpy(item->text, "Production Information");
				break;
			case CONTAINCAT_USAGE:
				strcpy(item->text, "Usage");
				break;
			case CONTAINCAT_TECHSPEC:
				strcpy(item->text, "Technical Specs");
				break;
		}*/
		if(item)
			strcpy(item->text, name);

		lastDepth2 = 1;
		lastCat++;
	}
	else
	{
		if(!item && containingCat != CONTAINCAT_TECHSPEC)
		{
			*depthPtr -= 1;
			return;
		}
		switch(containingCat)
		{
			case CONTAINCAT_PRODUCTIONINFO:
				{
					// Cool side effect: virtually infinite possibilities!
					strcpy(item->text, name);
					if(useAutoFill)
					{
						if(autoFill.byteCount == 1)
						{
							if(formatStr[0] != '\0')
							{
								sprintf(item->text2, formatStr, (int)autoFill.data.uc);
							}
							else
							{
								strcpy(item->text2, va("%i", (int)autoFill.data.uc));
							}
						}
						else if(autoFill.isFloat)
						{
							if(formatStr[0] != '\0')
							{
								sprintf(item->text2, formatStr, autoFill.data.f);
							}
							else
							{
								strcpy(item->text2, va("%f", autoFill.data.f));
							}
						}
						else if(autoFill.isString)
						{
							if(formatStr[0] != '\0')
							{
								sprintf(item->text2, formatStr, autoFill.data.a);
							}
							else
							{
								strcpy(item->text2, va("%s", autoFill.data.a));
							}
							free(autoFill.data.a);
						}
						else
						{
							if(formatStr[0] != '\0')
							{
								sprintf(item->text2, formatStr, autoFill.data.i);
							}
							else
							{
								strcpy(item->text2, va("%i", autoFill.data.i));
							}
						}
					}
					else
					{
						strcpy(item->text2, last_content);
					}
				}
				break;
			case CONTAINCAT_USAGE:
				{
					strcpy(item->text, name);
					if(useAutoFill)
					{
						if(autoFill.byteCount == 1)
						{
							if(formatStr[0] != '\0')
							{
								sprintf(item->text2, formatStr, (int)autoFill.data.uc);
							}
							else
							{
								strcpy(item->text2, va("%i", (int)autoFill.data.uc));
							}
						}
						else if(autoFill.isFloat)
						{
							if(formatStr[0] != '\0')
							{
								sprintf(item->text2, formatStr, autoFill.data.f);
							}
							else
							{
								strcpy(item->text2, va("%f", autoFill.data.f));
							}
						}
						else if(autoFill.isString)
						{
							if(formatStr[0] != '\0')
							{
								sprintf(item->text2, formatStr, autoFill.data.a);
							}
							else
							{
								strcpy(item->text2, va("%s", autoFill.data.a));
							}
							free(autoFill.data.a);
						}
						else
						{
							if(formatStr[0] != '\0')
							{
								sprintf(item->text2, formatStr, autoFill.data.i);
							}
							else
							{
								strcpy(item->text2, va("%i", autoFill.data.i));
							}
						}
					}
					else
					{
						strcpy(item->text2, last_content);
					}
				}
				break;
			case CONTAINCAT_TECHSPEC:
				{
					if(*depthPtr == 3)
					{
						//Technical specification
						itemDef_t *item = Menu_FindItemByName(inventoryState.menu, va("examine%iCat%i_text%i", JKG_Inventory_ExamineMenuNumForItemType(inventoryState.selectedItem->id->itemType), containingCat,
							last_tableline));
						strcpy(item->text, name);
						last_tableline++;
					}
					else if(*depthPtr == 4)
					{
						itemDef_t *item = Menu_FindItemByName(inventoryState.menu, va("examine%iCat%i_text%idata", JKG_Inventory_ExamineMenuNumForItemType(inventoryState.selectedItem->id->itemType), containingCat,
							last_tableline));
						if(!Q_stricmp(name, "Primary"))
						{
							if(useAutoFill)
							{
								if(formatStr[0]  == '\0')
								{
									if(autoFill.isFloat)
									{
										sprintf(item->text, "%f", autoFill.data.f);
									}
									else if(autoFill.isString)
									{
										strcpy(item->text, autoFill.data.a);
										free(autoFill.data.a);
									}
									else if(autoFill.byteCount == 1)
									{
										sprintf(item->text, "%i", (int)autoFill.data.uc);
									}
									else
									{
										sprintf(item->text, "%i", autoFill.data.i);
									}
								}
								else
								{
									if(autoFill.isFloat)
									{
										sprintf(item->text, formatStr, autoFill.data.f);
									}
									else if(autoFill.isString)
									{
										sprintf(item->text, formatStr, autoFill.data.a);
										free(autoFill.data.a);
									}
									else if(autoFill.byteCount == 1)
									{
										sprintf(item->text, formatStr, (int)autoFill.data.uc);
									}
									else
									{
										sprintf(item->text, formatStr, autoFill.data.i);
									}
								}
							}
							else
							{
								strcpy(item->text, last_content);
							}
						}
						else if(!Q_stricmp(name, "Secondary"))
						{
							if(useAutoFill)
							{
								if(formatStr[0]  == '\0')
								{
									if(autoFill.isFloat)
									{
										sprintf(item->text2, "%f", autoFill.data.f);
									}
									else if(autoFill.isString)
									{
										strcpy(item->text2, autoFill.data.a);
										free(autoFill.data.a);
									}
									else if(autoFill.byteCount == 1)
									{
										sprintf(item->text2, "%i", (int)autoFill.data.uc);
									}
									else
									{
										sprintf(item->text2, "%i", autoFill.data.i);
									}
								}
								else
								{
									if(autoFill.isFloat)
									{
										sprintf(item->text2, formatStr, autoFill.data.f);
									}
									else if(autoFill.isString)
									{
										sprintf(item->text2, formatStr, autoFill.data.a);
										free(autoFill.data.a);
									}
									else if(autoFill.byteCount == 1)
									{
										sprintf(item->text2, formatStr, (int)autoFill.data.uc);
									}
									else
									{
										sprintf(item->text2, formatStr, autoFill.data.i);
									}
								}
							}
							else
							{
								strcpy(item->text2, last_content);
							}
						}
					}
					
				}
				break;
		}
		lastDepth2++;
	}
	formatStr[0] = '\0';
	*depthPtr -= 1;
}
#pragma endregion
static void XMLCALL JKGXML_ParseCharacters(void *data, const char *content, int length)
{
	char           *tmp = (char *)malloc(length);
	strncpy(tmp, content, length);
	tmp[length] = '\0';
	data = (void *) tmp;
	last_content = tmp;
}

void JKG_Inventory_Examine_ParseXML ( void )
{
	XML_Parser parse = XML_ParserCreate(NULL);
	int depth = 0;
	int len;
	fileHandle_t f;
	char buffer[MAX_XML_BUFFER_SIZE];
	if(!inventoryState.selectedItem->id->xml)
		return;
	//Before we go off doing XML related stuff, let's make sure the file exists!
	len = trap_FS_FOpenFile(inventoryState.selectedItem->id->xml, &f, FS_READ);
	if(!f)
	{
		Com_Printf("^3WARNING: JKG_Inventory_Examine_ParseXML: %s NULL handle\n", inventoryState.selectedItem->id->xml);
		return;
	}
	if(!len || len == -1)
	{
		Com_Printf("^3WARNING: JKG_Inventory_Examine_ParseXML: %s NULL len\n", inventoryState.selectedItem->id->xml);
		trap_FS_FCloseFile(f);
		return;
	}
	if(len >= MAX_XML_BUFFER_SIZE)
	{
		Com_Printf("^3WARNING: JKG_Inventory_Examine_ParseXML: %s large len\n", inventoryState.selectedItem->id->xml);
		trap_FS_FCloseFile(f);
		return;
	}
	lastCat = 1;
	lastDepth2 = 1;
	containingCat = 0;
	//Set the XML related information
	XML_SetUserData(parse, &depth);
	XML_SetElementHandler(parse, JKGXML_Start_Element, JKGXML_End_Element);
	XML_SetCharacterDataHandler(parse, JKGXML_ParseCharacters);
	//Read the file and parse the data
	trap_FS_Read(buffer, MAX_XML_BUFFER_SIZE, f);
	trap_FS_FCloseFile(f);
	buffer[len] = '\0';
	if(XML_Parse(parse, buffer, (int)strlen(buffer), qtrue) == XML_STATUS_ERROR)
	{
		Com_Printf("^3WARNING: XML: %s at line %i\n",
			XML_ErrorString(XML_GetErrorCode(parse)),
			XML_GetCurrentLineNumber(parse));
		return;
	}
	XML_ParserFree(parse);
}

#pragma endregion
void JKG_Inventory_Examine_Button ( int forceOff )
{
	int i;
	qboolean examineMenuUp = qfalse;
	if(!forceOff)
	{
		for(i = 1; i < 4; i++)
		{
			itemDef_t *item = Menu_FindItemByName(inventoryState.menu, va("examine%i_bg", i));
			if(item->window.flags & WINDOW_VISIBLE)
			{
				examineMenuUp = qtrue;
				break;
			}
		}
	}
	if(!examineMenuUp && (forceOff == 2 || forceOff == 0))
	{
		char actualTextCheck[256];
		Menu_ItemDisable (inventoryState.menu, "main_dialog", qtrue);
		Menu_ItemDisable (inventoryState.menu, "arrows", qtrue);
		Menu_ItemDisable (inventoryState.menu, "btn_examine", qfalse);
		if(!inventoryState.selectedItem)
		{
			// ERR what. Should have an item selected!
			JKG_Inventory_Examine_Button(qtrue);
			return;
		}
		if(inventoryState.selectedItem->id->displayName[0] == '@')
			trap_SP_GetStringTextString(inventoryState.selectedItem->id->displayName+1, actualTextCheck, sizeof(actualTextCheck));
		else
			strcpy(actualTextCheck, inventoryState.selectedItem->id->displayName);
		//OK, next we have to update the background object to reflect the different item types
		switch(inventoryState.selectedItem->id->itemType)
		{
			case ITEM_WEAPON:
				{
					itemDef_t *item = Menu_FindItemByName(inventoryState.menu, "examine1_title");
					strcpy(item->text, actualTextCheck);
					item->textRect.w = 0; //Hack to recalculate centering
					Menu_ShowGroup (inventoryState.menu, "examine1Menu", qtrue);
					Menu_ShowGroup (inventoryState.menu, "examine2Menu", qfalse);
					Menu_ShowGroup (inventoryState.menu, "examine3Menu", qfalse);
				}
				break;
			case ITEM_CLOTHING:
			case ITEM_ARMOR:
				{
					itemDef_t *item = Menu_FindItemByName(inventoryState.menu, "examine2_title");
					item->textRect.w = 0;
					Menu_ShowGroup (inventoryState.menu, "examine1Menu", qfalse);
					Menu_ShowGroup (inventoryState.menu, "examine2Menu", qtrue);
					Menu_ShowGroup (inventoryState.menu, "examine3Menu", qfalse);
					strcpy(item->text, actualTextCheck);
				}
				break;
			case ITEM_BUFF:
			case ITEM_UNKNOWN:
			case ITEM_CONSUMABLE:
				{
					itemDef_t *item = Menu_FindItemByName(inventoryState.menu, "examine3_title");
					item->textRect.w = 0;
					Menu_ShowGroup (inventoryState.menu, "examine1Menu", qfalse);
					Menu_ShowGroup (inventoryState.menu, "examine2Menu", qfalse);
					Menu_ShowGroup (inventoryState.menu, "examine3Menu", qtrue);
					strcpy(item->text, actualTextCheck);
				}
				break;
		}
		JKG_Inventory_Examine_ParseXML();
		inventoryState.examineMenuOpen = qtrue;
	}
	else if(examineMenuUp || forceOff == 1)
	{
		Menu_ShowGroup (inventoryState.menu, "examine1Menu", qfalse);
		Menu_ShowGroup (inventoryState.menu, "examine2Menu", qfalse);
		Menu_ShowGroup (inventoryState.menu, "examine3Menu", qfalse);
		Menu_ItemDisable (inventoryState.menu, "main_dialog", qfalse);
		Menu_ItemDisable (inventoryState.menu, "arrows", qfalse);
		UpdateButtonStates();
		inventoryState.examineMenuOpen = qfalse;
	}
}
void JKG_Inventory_Script_Button ( char **args )
{
    int button;
    cgItemInstance_t *inventory;
	int itemSlot;
	itemDef_t *textItem = Menu_FindItemByName(inventoryState.menu, "destroy_text");
    
    if ( !Int_Parse (args, &button) )
    {
        return;
    }
    
	if ( !inventoryState.selectedItem || !inventoryState.selectedItem->id )
    {
        return;
    }
    
	inventory = (cgItemInstance_t *)cgImports->InventoryDataRequest( 1 );
	itemSlot = (ui_inventoryFilter.integer == JKGIFILTER_ALL) ? inventoryState.selectedItem - inventory : inventoryState.selectedItem->amount[0];
    switch ( button )
    {
        case INV_BTN_USEITEM:
			cgImports->SendClientCommand(va ("inventoryUse %d", itemSlot));
            UpdateButtonStates();
			JKG_Inventory_UpdateVisuals();
			break;
        
        case INV_BTN_EQUIP:
			cgImports->SendClientCommand(va ("equip %d", itemSlot));
            UpdateButtonStates();
			break;
        
        case INV_BTN_ASSIGN2ACI:
			{
				itemDef_t *item = Menu_FindItemByName(inventoryState.menu, "aci_background"); //Kind of a hacky approach but whatever
				if(item->window.flags & WINDOW_VISIBLE)
				{
					    Menu_ShowGroup (inventoryState.menu, "aci_selection", qfalse);
						Menu_ItemDisable (inventoryState.menu, "arrows", qfalse);
						Menu_ClearFocus (inventoryState.menu);
    
						Menu_ItemDisable (inventoryState.menu, "main_dialog", qfalse);

						Menu_ShowGroup (inventoryState.menu, "aci_selection_na", qfalse);
						UpdateButtonStates();
						inventoryState.ACIopen = qfalse;
				}
				else
				{
					Menu_ItemDisable (inventoryState.menu, "main_dialog", qtrue);
					Menu_ItemDisable (inventoryState.menu, "arrows", qtrue);
					Menu_ItemDisable (inventoryState.menu, "btn_aci", qfalse);
					Menu_ShowGroup (inventoryState.menu, "aci_selection", qtrue);
					JKG_ACI_GreyBlast();
					inventoryState.ACIopen = qtrue;
				}
			}
			break;
        
        case INV_BTN_UNEQUIP:
			cgImports->SendClientCommand(va ("unequip %d", itemSlot));
            UpdateButtonStates();
			break;
        
        case INV_BTN_UNASSIGN4ACI:
			cgImports->InventoryAttachToACI( itemSlot, GetACISlotForItem( inventoryState.selectedItem->id->itemID ), 0 );
            UpdateButtonStates();
			break;

		case INV_BTN_DESTROYITEM:
			{
				itemDef_t *item = Menu_FindItemByName(inventoryState.menu, "destroy_bg"); //Kind of a hacky approach but whatever
				if(item->window.flags & WINDOW_VISIBLE)
				{
					Menu_ItemDisable (inventoryState.menu, "main_dialog", qfalse);
					Menu_ItemDisable (inventoryState.menu, "arrows", qfalse);
					Menu_ShowGroup (inventoryState.menu, "destroyMenu", qfalse);
					UpdateButtonStates();
				}
				else
				{
					Menu_ItemDisable (inventoryState.menu, "main_dialog", qtrue);
					Menu_ItemDisable (inventoryState.menu, "arrows", qtrue);
					Menu_ItemDisable (inventoryState.menu, "btn_destroy", qfalse);
					Menu_ShowGroup (inventoryState.menu, "destroyMenu", qtrue);
					strcpy(textItem->text, inventoryState.selectedItem->id->displayName);
				}
			}
			break;

		case INV_BTN_DESTROYITEM_CONFIRM:
			cgImports->SendClientCommand(va ("inventoryDestroy %d", itemSlot));
			Menu_ItemDisable (inventoryState.menu, "main_dialog", qfalse);
			Menu_ShowGroup (inventoryState.menu, "destroyMenu", qfalse);
			JKG_Inventory_ConstructWeightText(); // Item has been removed, so update weight text
			break;
		case INV_BTN_DESTROYITEM_DENY:
			//Same as above, but without the actual killing of the item
			Menu_ItemDisable (inventoryState.menu, "main_dialog", qfalse);
			Menu_ShowGroup (inventoryState.menu, "destroyMenu", qfalse);
			break;
		case INV_BTN_SELL:
			{
				{
					itemDef_t *item = Menu_FindItemByName(inventoryState.menu, "sell_bg"); //Kind of a hacky approach but whatever
					textItem = Menu_FindItemByName(inventoryState.menu, "sell_text");
					if(item->window.flags & WINDOW_VISIBLE)
					{
						Menu_ItemDisable (inventoryState.menu, "main_dialog", qfalse);
						Menu_ItemDisable (inventoryState.menu, "arrows", qfalse);
						Menu_ShowGroup (inventoryState.menu, "sellMenu", qfalse);
						UpdateButtonStates();
					}
					else
					{
						Menu_ItemDisable (inventoryState.menu, "main_dialog", qtrue);
						Menu_ItemDisable (inventoryState.menu, "arrows", qtrue);
						Menu_ItemDisable (inventoryState.menu, "btn_sell", qfalse);
						Menu_ShowGroup (inventoryState.menu, "sellMenu", qtrue);
						strcpy(textItem->text, inventoryState.selectedItem->id->displayName);
					}
				}
			}
			break;
		case INV_BTN_SELL_CONFIRM:
			cgImports->SendClientCommand(va ("inventorySell %d", itemSlot));
			Menu_ItemDisable (inventoryState.menu, "main_dialog", qfalse);
			Menu_ShowGroup (inventoryState.menu, "sellMenu", qfalse);
			JKG_Inventory_ConstructWeightText(); // Item has been removed, so update weight text
			break;
		case INV_BTN_SELL_DENY:
			Menu_ItemDisable (inventoryState.menu, "main_dialog", qfalse);
			Menu_ShowGroup (inventoryState.menu, "sellMenu", qfalse);
			break;
		case INV_BTN_EXAMINE:
			JKG_Inventory_Examine_Button(qfalse);
			break;
    }
}

//Add-on for making weapon icons work ~eezstreet
qhandle_t JKG_GetInventoryIcon(unsigned int index)
{
	if(ui_inventoryFilter.integer == 0)
	{
		if(index < MAX_INVENTORY_ITEMS)
		{
			return trap_R_RegisterShaderNoMip((char *)cgImports->InventoryDataRequest( index + 50 ) ); //HACK
		}
		else
		{
			return (qhandle_t)NULL;
		}
	}
	else
	{
		if(index < MAX_INVENTORY_ITEMS)
		{
			return trap_R_RegisterShaderNoMip((char *)cgImports->InventoryDataRequest( itemsInFilter[index].amount[0] + 50 ));
		}
		else
		{
			return (qhandle_t)NULL;
		}
	}
}

// Add-on for tooltips ~eezstreet
qboolean JKG_CursorInItem(float cx, float cy, unsigned int feederPos, itemDef_t *item)
{
	//cx/y = cursor x/y
	//feederPos = visual position in the feeder (NOT ABSOLUTE)
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
	int maxW = (int)(item->window.rect.w / (listPtr->elementWidth+listPtr->elementSpacingW)); //FIXED
	int maxH = (int)(item->window.rect.h / (listPtr->elementHeight+listPtr->elementSpacingH));
	cgItemInstance_t *inventory = NULL;

	//Basic check #1: Divide by zero
	if(maxW <= 0 || maxH <= 0)
	{
		return qfalse;
	}
	maxW += 1;
	maxH += 1;

	//Basic checks->is the cursor within the feeder's rect?
	if(cx > (item->window.rect.x + item->window.rect.w))
	{
		return qfalse;
	}
	else if(cx < item->window.rect.x)
	{
		return qfalse;
	}
	if(cy > (item->window.rect.y + item->window.rect.h))
	{
		return qfalse;
	}
	else if(cy < item->window.rect.y)
	{
		return qfalse;
	}

	cx -= item->window.rect.x;
	cy -= item->window.rect.y;
	if(ui_inventoryFilter.integer == JKGIFILTER_ALL)
	{
		inventory = (cgItemInstance_t *)cgImports->InventoryDataRequest( 1 );
	}
	else
	{
		inventory = itemsInFilter;
	}
	if(!inventory[(listPtr->startPos > 0) ? (listPtr->startPos+feederPos) : feederPos].id)
	{
		return qfalse;
	}
	
	if(feederPos >= maxH)
	{
		//Vertical rows
		if(cy < (((listPtr->elementSpacingH+listPtr->elementHeight)*((feederPos % maxH)+1))-listPtr->elementSpacingH))
		{
			if(cy > ((listPtr->elementSpacingH+listPtr->elementHeight)*((feederPos % maxH))))
			{
				if(cx > (int)((listPtr->elementSpacingW+listPtr->elementWidth)*(feederPos / maxW)-1))
				{
					if(cx < (int)((listPtr->elementSpacingW+listPtr->elementWidth)*(((feederPos+maxH) / maxH))-listPtr->elementSpacingW))
					{
						return qtrue;
					}
					else
					{
						return qfalse;
					}
				}
				else
				{
					return qfalse;
				}
			}
			else
			{
				return qfalse;
			}
		}
		else
		{
			return qfalse;
		}
	}
	else
	{
		//First column
		if(cx < listPtr->elementWidth)
		{
			if(cy < ((listPtr->elementHeight+listPtr->elementSpacingH)*(feederPos+1)-listPtr->elementSpacingH))
			{
				if(cy > ((listPtr->elementWidth+listPtr->elementSpacingW)*feederPos))
				{
					return qtrue;
				}
				else
				{
					return qfalse;
				}
			}
			else
			{
				return qfalse;
			}
		}
		else
		{
			return qfalse;
		}
	}

	return qfalse;
}

#define MAX_TOOLTIP_LINES		32
#define MAX_TOOLTIP_LINELEN		64

static char toolTextLines[MAX_TOOLTIP_LINES][MAX_TOOLTIP_LINELEN];	//TODO: dynamic allocation
static char toolText[(MAX_TOOLTIP_LINES*MAX_TOOLTIP_LINELEN)+MAX_TOOLTIP_LINES+1];
static int topTextLength = 0;
static int toolLines = 0;
static int biggestLine = 0;
static float toolAlignment[MAX_TOOLTIP_LINES]; // HAX

void JKG_Inventory_Tooltip_AddLine(itemDef_t *toolTip, const char *text)
{
	//TODO: add coloring
	int lineLength;
	strcpy(toolTextLines[toolLines], text);
	toolAlignment[toolLines] = (toolTip->window.rect.w - (trap_R_Font_StrLenPixels(toolTextLines[toolLines], toolTip->iMenuFont, 1.0f)*toolTip->textscale))/2;
	lineLength = strlen(text);
	if(lineLength > strlen(toolTextLines[biggestLine]))
	{
		biggestLine = toolLines;
	}
	toolLines++;
}

//This gets called every time you move the mouse. Surprisingly, this does not affect FPS too much
void JKG_Inventory_ConstructToolTip ( int itemNumber, float cX, float cY )
{
	//Gather our variables
	char actualTextCheck[256];
	int i;
	int totalTTLength = 1;
	float lineWidth = 0.0f, lineHeight = 0.0f;
	cgItemInstance_t *inventory = NULL;
	itemDef_t *toolTipItem = Menu_FindItemByName(inventoryState.menu, "inventory_tooltest");
	//Check for filtering
	if(ui_inventoryFilter.integer == JKGIFILTER_ALL)
	{
		inventory = (cgItemInstance_t *)cgImports->InventoryDataRequest( 1 );
	}
	else
	{
		inventory = itemsInFilter;
	}

	//Check for length
	if( itemNumber >= JKG_Inventory_FeederCount() )
	{
		return;
	}

	//Reset all previous data
	for(i = 0; i < toolLines; i++)
	{
		toolTextLines[i][0] = 0;
	}
	toolLines = 0;

	//Build the actual text on the tooltip
	//Name
	strcpy(toolTextLines[toolLines], inventory[itemNumber].id->displayName);
	trap_SP_GetStringTextString(inventory[itemNumber].id->displayName+1, actualTextCheck, sizeof(actualTextCheck));
	topTextLength = strlen(actualTextCheck); //NOTENOTE: Whenever we add a new possible line to the tooltip, we check the length (so we don't loop through every line again)
	if(topTextLength > 0)
	{
		strcpy(toolTextLines[toolLines], actualTextCheck);
	}
	else
	{
		strcpy(toolTextLines[toolLines], inventory[itemNumber].id->displayName);
	}
	toolAlignment[toolLines] = (toolTipItem->window.rect.w - (trap_R_Font_StrLenPixels(toolTextLines[toolLines], toolTipItem->iMenuFont, 1.0f)*toolTipItem->textscale))/2;
	toolLines++;
	biggestLine = 0;
	//The biggest line is always assumed to be this line in this case, because the first line is...AWESOME!
	switch(inventory[itemNumber].id->itemType)
	{
		case ITEM_WEAPON:
			JKG_Inventory_Tooltip_AddLine(toolTipItem,  va("Weapon"));
			break;
		case ITEM_ARMOR:
			JKG_Inventory_Tooltip_AddLine(toolTipItem,  va("Armor"));
			break;
		case ITEM_CLOTHING:
			JKG_Inventory_Tooltip_AddLine(toolTipItem,  va("Clothing"));
			break;
		case ITEM_BUFF:
		case ITEM_CONSUMABLE:
			JKG_Inventory_Tooltip_AddLine(toolTipItem,  va("Consumable"));
			break;
	}
	if(inventory[itemNumber].id->weight)
	{
		JKG_Inventory_Tooltip_AddLine(toolTipItem,  va("Weight: %i", inventory[itemNumber].id->weight));
	}
	if(IsWeaponInACI(inventory[itemNumber].id->varID))
	{
		JKG_Inventory_Tooltip_AddLine(toolTipItem, va("Equipped to ACI Slot %i", GetACISlotForWeapon(inventory[itemNumber].id->varID)));
	}
	//Grab the width of the biggest line
	if(!toolTipItem)
	{
		//Item not found, ABORT ABORT ABORT
		return;
	}
	else
	{
		/*if(actualTextCheck[0] != 0)
			lineWidth = (DC->textWidth(toolTextLines[biggestLine], 1, toolTipItem->iMenuFont) * toolTipItem->textscale);
		else
			lineWidth = (DC->textWidth(inventory[itemNumber].id->displayName, 1, toolTipItem->iMenuFont) * toolTipItem->textscale);*/
		lineWidth = (DC->textWidth(toolTextLines[biggestLine], 1, toolTipItem->iMenuFont) * toolTipItem->textscale);
		toolTipItem->window.rect.w = lineWidth+70; //35px padding on each side, 18pt font //this is a really absurd measurement but ok
	}
	if(toolLines <= 0)
	{ //Check: Do we have any tooltip lines?
		return;
	}

	//Grab the height of the combined lines
	{
		lineHeight = DC->textHeight(toolTextLines[biggestLine], toolTipItem->textscale, toolTipItem->iMenuFont);
		toolTipItem->window.rect.h = (lineHeight*toolLines)+40; //20px padding on each side
	}

	//Change the position of the toolTipItem
	//TODO: Perform OOB checks here
	toolTipItem->window.rect.x = cX + 10;
	toolTipItem->window.rect.y = cY - 10;
	//strcpy(toolTipItem->text, toolText);
	toolTipItem->textRect.x = cX + 10;
	toolTipItem->textRect.y = cY + 30;
	//HACK: Align the text properly
	//toolTipItem->textalignx = toolTipItem->window.rect.w/2 - (((int)strlen(toolTextLines[biggestLine]) > 8) ? ((strlen(toolTextLines[biggestLine])/5)) : 0); //HACK for non monospace fonts
	toolTipItem->textalignx = (toolTipItem->window.rect.w - (trap_R_Font_StrLenPixels(toolTextLines[biggestLine], toolTipItem->iMenuFont, 1.0f)*toolTipItem->textscale))/2;
	toolTipItem->textRect.w = toolTipItem->window.rect.w;
	toolTipItem->textRect.h = toolTipItem->window.rect.h;
	toolTipItem->textaligny = (toolTipItem->textRect.h/2)-4;
}

void JKG_Inventory_CheckTooltip ( char **args )
{
	int i = 0;
	itemDef_t *item = Menu_FindItemByName(inventoryState.menu, "inventory_feederstuff");
	itemDef_t *toolTipItem = Menu_FindItemByName(inventoryState.menu, "inventory_tooltest");
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
	int numWidth = item->window.rect.w/(listPtr->elementSpacingW+listPtr->elementWidth)+1;
	int numHeight = item->window.rect.h/(listPtr->elementSpacingH+listPtr->elementHeight)+1;
	qboolean showToolTip = qfalse;
	for(i=0; i < (numWidth*numHeight); i++)
	{
		if(JKG_CursorInItem(DC->cursorx, DC->cursory, i, item))
		{
			//Do fun stuff here
			showToolTip = qtrue;
			break;
		}
	}
	if(showToolTip)
	{
		cgItemInstance_t *inventory = NULL;
		inventory = (cgItemInstance_t *)cgImports->InventoryDataRequest( 1 );
		//Show the tooltip
		Menu_ShowItemByName(inventoryState.menu, "inventory_tooltest", qtrue);
		JKG_Inventory_ConstructToolTip((listPtr->startPos > 0) ? (listPtr->startPos+i) : i, DC->cursorx, DC->cursory);
	}
	else
	{
		Menu_ShowItemByName(inventoryState.menu, "inventory_tooltest", qfalse);
	}
}

qboolean IsWithinCursor(float x, float y, float w, float h)
{
	if(DC->cursorx > x+w)
	{
		return qfalse;
	}
	else if(DC->cursorx < x)
	{
		return qfalse;
	}
	else if(DC->cursory > y+h)
	{
		return qfalse;
	}
	else if(DC->cursory < y)
	{
		return qfalse;
	}
	return qtrue;
}

void JKG_Inventory_DrawTooltip()
{
	int i;
	itemDef_t *item = Menu_FindItemByName(inventoryState.menu, "inventory_tooltest");
	int txtHeight = trap_R_Font_HeightPixels(item->iMenuFont, item->textscale);
	itemDef_t *itemCheck = Menu_FindItemByName(inventoryState.menu, "inventory_feederstuff");
	if(!IsWithinCursor(itemCheck->window.rect.x, itemCheck->window.rect.y, itemCheck->window.rect.w, itemCheck->window.rect.h))
	{
		//HACK: Hide the tooltip item altogether
		Menu_ShowItemByName(inventoryState.menu, "inventory_tooltest", qfalse);
		return;
	}
	//OK. Draw the text. Rest -should- be handled by default ownerdraw stuff, but you never know.
	for(i = 0; i < toolLines; i++)
	{
		//FIXMEFIXED: Show the correct text (display name)
		DC->drawText(item->textRect.x + toolAlignment[i], item->textRect.y+(i*txtHeight)-item->textaligny, item->textscale, item->window.foreColor, toolTextLines[i], 0, -1, ITEM_TEXTSTYLE_NORMAL, item->iMenuFont);
		//trap_R_Font_DrawString(item->textRect.x, item->textRect.y+(i*txtHeight), toolTextLines[i], colorWhite, 1, -1, item->textscale);
	}
}
void JKG_Inventory_CheckACIKeyStroke(int key)
{
	if(!inventoryState.ACIopen)
	{
		return;
	}
	else
	{
		JKG_Inventory_ACI_Button_Clean(key-A_0);
	}
}
void JKG_Inventory_UpdateVisuals( void )
{
	int i;
	itemDef_t *item = Menu_FindItemByName(inventoryState.menu, "inventory_feederstuff");
	cgItemInstance_t *inventory = (cgItemInstance_t *)cgImports->InventoryDataRequest( 1 );
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
	int numItems = JKG_Inventory_FeederCount();
	int maxW = (int)(item->window.rect.w / (listPtr->elementWidth+listPtr->elementSpacingW)); //FIXED
	int maxH = (int)(item->window.rect.h / (listPtr->elementHeight+listPtr->elementSpacingH));
	for(i = listPtr->startPos; i < listPtr->startPos + ((maxW+1)*(maxH+1)) && i < numItems; i++)
	{
		if(!inventory[i].id)
		{
			break;
		}
		if(ui_inventoryFilter.integer != JKGIFILTER_ALL)
		{
			if(IsItemInACI(itemsInFilter[i].id->itemID))
			{
				Menu_ShowItemByName(inventoryState.menu, va("feeder_tag%i", i-listPtr->startPos+1), qtrue);
			}
			else
			{
				Menu_ShowItemByName(inventoryState.menu, va("feeder_tag%i", i-listPtr->startPos+1), qfalse);
			}
		}
		else
		{
			if(IsItemInACI(inventory[i].id->itemID))
			{
				Menu_ShowItemByName(inventoryState.menu, va("feeder_tag%i", i-listPtr->startPos+1), qtrue);
			}
			else
			{
				Menu_ShowItemByName(inventoryState.menu, va("feeder_tag%i", i-listPtr->startPos+1), qfalse);
			}
		}
	}
	for(; i < listPtr->startPos + ((maxW+1)*(maxH+1)); i++)
	{
		Menu_ShowItemByName(inventoryState.menu, va("feeder_tag%i", i-listPtr->startPos+1), qfalse);
	}
}