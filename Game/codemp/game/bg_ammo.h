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
