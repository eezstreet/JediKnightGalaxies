#ifndef BG_AMMO_H
#define BG_AMMO_H

#include "q_shared.h"

typedef struct ammo_s
{
    char name[32];
    unsigned int clipSize;

    #ifdef CGAME
    qhandle_t model;
    qhandle_t icon;
    #endif
} ammo_t;

void BG_InitializeAmmo ( void );
const ammo_t *BG_GetAmmo ( const char *ammoName );
unsigned int BG_NumberOfLoadedAmmo ( void );

#endif
