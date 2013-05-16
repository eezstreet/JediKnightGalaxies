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
weaponData_t *GetWeaponDatas ( int weapon, int variation );
qboolean CO_IsWeaponInACI(int weaponId);
int CO_GetRedTeam(void);
int CO_GetBlueTeam(void);
networkState_t	*CO_GetNetworkState(void);
extraState_t	*CO_GetExtraState(int clientNum);
extraState_t	*CO_GetOldExtraState(int clientNum);

void CO_SysCall_UI();
void CO_SysCall_CG();