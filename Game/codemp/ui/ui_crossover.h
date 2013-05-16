// UI Crossover defines
// Jedi Knight Galaxies
//
// By BobaFett

void CO_SendClientCommand(const char *cmd);
void CO_InitCrossover();
qboolean CO_EscapeTrapped();
void *CO_PartyMngtDataRequest(int data);
void *CO_InventoryDataRequest ( int data );
void CO_InventoryAttachToACI(int itemNum, int slot, int attach);
qboolean CO_IsWeaponInACI(int weaponId);

void CO_SysCall_UI();
void CO_SysCall_CG();