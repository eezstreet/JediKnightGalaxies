#include "../game/bg_items.h"
#include "ui_local.h"
#include "ui_crossover.h"

/* Structure Definitions */
typedef struct
{

    // LOOK AT ME, I'M A COMMENT.
    // Okay, I have your attention now. If you edit this struct, make sure you
    // update the struct in cgame/jkg_cg_items.h too.


	//oboi, clientside time!
	//Basic Item Characteristics
	char displayName[MAX_ITEM_NAME];
	unsigned int itemID;
	jkgItemType_t itemType;
	unsigned int weight;

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
    // update the struct in cgame/jkg_cg_items.h too.


	cgItemData_t *id;
	unsigned int itemQuality;

	//pSpell stuff
	unsigned int amount[MAX_PSPELLS]; //NOTENOTE: Need to make a variable which holds all final values of stats

	//ACI stuff
	qboolean equipped;
} cgItemInstance_t;

struct
{
    qboolean active;
    menuDef_t *menu;
    cgItemInstance_t *selectedItem;
} inventoryState;

void JKG_Inventory_OpenDialog ( char **args )
{
    trap_Cvar_Set ("ui_hidehud", "1");
    inventoryState.active = qtrue;
    
    Menu_ClearFocus (inventoryState.menu);
    Menu_ItemDisable (inventoryState.menu, "main_dialog", qfalse);
}

void JKG_Inventory_CloseDialog ( char **args )
{
    trap_Cvar_Set ("ui_hidehud", "0");
    inventoryState.active = qfalse;
}

int JKG_Inventory_FeederCount ( void )
{
    return *(int *)CO_InventoryDataRequest (0);
}

static int GetACISlotForWeapon ( int weaponId )
{
    int *playerACI = (int *)CO_InventoryDataRequest (2);
    int i;
    for ( i = 0; i < MAX_ACI_SLOTS; i++ )
    {
        if ( playerACI[i] == weaponId )
        {
            return i;
        }
    }
    
    return -1;
}

static qboolean IsWeaponInACI ( int weaponId )
{
    return GetACISlotForWeapon (weaponId) != -1;
}

void UpdateButtonStates ( void )
{
    qboolean equipped;
    
    if ( !inventoryState.active || !inventoryState.selectedItem )
    {
        return;
    }
    
    equipped = inventoryState.selectedItem->equipped;
    switch ( inventoryState.selectedItem->id->itemType )
    {
        case ITEM_ARMOR:
            Menu_ShowItemByName (inventoryState.menu, "btn_equip", !equipped);
            Menu_ShowItemByName (inventoryState.menu, "btn_unequip", equipped);
            
            Menu_ItemDisable (inventoryState.menu, "btn_equip", equipped);
            Menu_ItemDisable (inventoryState.menu, "btn_unequip", !equipped);
            
            Menu_ItemDisable (inventoryState.menu, "btn_useitem", 1);
            Menu_ItemDisable (inventoryState.menu, "btn_aci", 1);
            
            Menu_ItemDisable (inventoryState.menu, "btn_aci2", 1);
            Menu_ShowItemByName (inventoryState.menu, "btn_aci2", qfalse);
        break;
        
        case ITEM_WEAPON:
        {
            qboolean weaponInACI = IsWeaponInACI (inventoryState.selectedItem->id->varID);
        
            Menu_ShowItemByName (inventoryState.menu, "btn_equip", !equipped);
            Menu_ShowItemByName (inventoryState.menu, "btn_unequip", equipped);
            
            Menu_ItemDisable (inventoryState.menu, "btn_equip", equipped);
            Menu_ItemDisable (inventoryState.menu, "btn_unequip", !equipped);
            
            Menu_ItemDisable (inventoryState.menu, "btn_useitem", 1);
            
            Menu_ShowItemByName (inventoryState.menu, "btn_aci", !weaponInACI);
            Menu_ShowItemByName (inventoryState.menu, "btn_aci2", weaponInACI);
            
            Menu_ItemDisable (inventoryState.menu, "btn_aci", weaponInACI);
            Menu_ItemDisable (inventoryState.menu, "btn_aci2", !weaponInACI);
        }
        break;
        
        default:
            Menu_ItemDisable (inventoryState.menu, "btn_useitem", 0);
            Menu_ItemDisable (inventoryState.menu, "btn_equip", 1);
            Menu_ItemDisable (inventoryState.menu, "btn_unequip", 1);
            Menu_ItemDisable (inventoryState.menu, "btn_aci", 1);
            Menu_ItemDisable (inventoryState.menu, "btn_aci2", 1);
            Menu_ShowItemByName (inventoryState.menu, "btn_equip", qtrue);
            Menu_ShowItemByName (inventoryState.menu, "btn_unequip", qfalse);
            Menu_ShowItemByName (inventoryState.menu, "btn_aci", qtrue);
            Menu_ShowItemByName (inventoryState.menu, "btn_aci2", qfalse);
        break;
    }
}

qboolean JKG_Inventory_FeederSelection ( int index )
{
    int numItems = *(int *)CO_InventoryDataRequest (0);
    cgItemInstance_t *inventory = NULL;
    
    if ( index < 0 || index >= numItems )
    {
        return qfalse;
    }
    
    inventory = (cgItemInstance_t *)CO_InventoryDataRequest (1);
    inventoryState.selectedItem = &inventory[index];
    
    UpdateButtonStates();
    
    return qtrue;
}

const char *JKG_Inventory_FeederItemText ( int index, int column, qhandle_t *handle1, qhandle_t *handle2, qhandle_t *handle3 )
{
    int numItems = *(int *)CO_InventoryDataRequest (0);
    cgItemInstance_t *inventory;
    
    if ( index < 0 || index >= numItems )
    {
        return NULL;
    }
    
    inventory = (cgItemInstance_t *)CO_InventoryDataRequest (1);
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
            
            CO_SysCall_UI();
            inventoryState.menu = Menus_FindByName ("jkg_inventory");
            if ( inventoryState.menu && Menus_ActivateByName ("jkg_inventory") )
            {
                trap_Key_SetCatcher (trap_Key_GetCatcher() | KEYCATCH_UI & ~KEYCATCH_CONSOLE);
            }
            CO_SysCall_CG();
        break;
        
        case 1: // update!
            UpdateButtonStates();
        break;
    }
}

enum inventoryButtons_e
{
    INV_BTN_USEITEM,
    INV_BTN_EQUIP,
    INV_BTN_ASSIGN2ACI,
    INV_BTN_UNEQUIP,
    INV_BTN_UNASSIGN4ACI
};

void JKG_Inventory_ACI_Button ( char **args )
{
    int slot;
    if ( !Int_Parse (args, &slot) )
    {
        return;
    }
    
    if ( slot < 0 || slot >= MAX_ACI_SLOTS )
    {
        return;
    }
    
    CO_InventoryAttachToACI (inventoryState.selectedItem->id->itemID, slot, 1);
    
    Menu_ShowGroup (inventoryState.menu, "aci_selection", qfalse);
    Menu_ClearFocus (inventoryState.menu);
    
    Menu_ItemDisable (inventoryState.menu, "main_dialog", qfalse);
    UpdateButtonStates();
}

void JKG_Inventory_Script_Button ( char **args )
{
    int button;
    cgItemInstance_t *inventory;
    
    if ( !Int_Parse (args, &button) )
    {
        return;
    }
    
    if ( !inventoryState.selectedItem )
    {
        return;
    }
    
    inventory = (cgItemInstance_t *)CO_InventoryDataRequest (1);
    switch ( button )
    {
        case INV_BTN_USEITEM:
            CO_SendClientCommand (va ("inventoryUse %d", inventoryState.selectedItem - inventory));
            UpdateButtonStates();
        break;
        
        case INV_BTN_EQUIP:
            CO_SendClientCommand (va ("equip %d", inventoryState.selectedItem - inventory));
            UpdateButtonStates();
        break;
        
        case INV_BTN_ASSIGN2ACI:
            Menu_ItemDisable (inventoryState.menu, "main_dialog", qtrue);
            Menu_ShowGroup (inventoryState.menu, "aci_selection", qtrue);
        break;
        
        case INV_BTN_UNEQUIP:
            CO_SendClientCommand (va ("unequip %d", inventoryState.selectedItem - inventory));
            UpdateButtonStates();
        break;
        
        case INV_BTN_UNASSIGN4ACI:
            CO_InventoryAttachToACI (inventoryState.selectedItem->id->itemID, GetACISlotForWeapon (inventoryState.selectedItem->id->varID), 0);
            UpdateButtonStates();
        break;
    }
}
