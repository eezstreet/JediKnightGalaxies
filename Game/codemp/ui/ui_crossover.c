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
// ui_crossover.c -- Crossover API module for UI
// Copyright (c) 2013 Jedi Knight Galaxies

#include "ui_local.h"

cgCrossoverExports_t *cgImports;

uiCrossoverExports_t ui;

static qboolean coTrapEscape = qfalse;

qboolean UI_RunSvCommand(const char *command);
void JKG_PartyMngt_UpdateNotify(int msg);
void JKG_Inventory_UpdateNotify(int msg);
void JKG_Shop_UpdateNotify(int msg);

void CO_SetEscapeTrapped( qboolean trapped )
{
	coTrapEscape = trapped;
}

uiCrossoverExports_t *UI_InitializeCrossoverAPI( cgCrossoverExports_t *cg )
{
	cgImports = cg;

	ui.HandleServerCommand = UI_RunSvCommand;
	ui.InventoryNotify = JKG_Inventory_UpdateNotify;
	ui.PartyMngtNotify = JKG_PartyMngt_UpdateNotify;
	ui.SetEscapeTrap = CO_SetEscapeTrapped;
	ui.ShopNotify = JKG_Shop_UpdateNotify;

	return &ui;
}