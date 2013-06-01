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

extern uiCrossoverExports_t *trap_CO_InitCrossover( cgCrossoverExports_t *uiImport );
void CG_InitializeCrossoverAPI( void )
{
	co.GetBlueTeam = CO_GetBlueTeam;
	co.GetRedTeam = CO_GetRedTeam;

	uiImports = trap_CO_InitCrossover( &co );
}