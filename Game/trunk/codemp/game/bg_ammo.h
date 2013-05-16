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
// bg_ammo.h
// Contains data for the ammo.json table
// (c) 2013 Jedi Knight Galaxies

#ifndef BG_AMMO_H
#define BG_AMMO_H

#define JKG_MAX_AMMO_INDICES		64

#include "q_shared.h"

typedef struct ammo_s
{
    char        name[32];
	unsigned int ammoIndex;
    float       damageModifier;
	unsigned int ammoMax;

	// Below appears unused. --eez
    #ifdef CGAME
    qhandle_t   model;
    fxHandle_t  fx;
    fxHandle_t  missFx;
    fxHandle_t  hitFx;
    fxHandle_t  deathFx;
    #endif
} ammo_t;

void BG_InitializeAmmo ( void );
const ammo_t *BG_GetAmmo ( const char *ammoName );

extern ammo_t ammoTable[JKG_MAX_AMMO_INDICES];

#endif
