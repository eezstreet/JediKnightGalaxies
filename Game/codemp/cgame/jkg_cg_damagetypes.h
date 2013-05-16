#ifndef JKG_CG_DAMAGETYPES_H
#define JKG_CG_DAMAGETYPES_H

#include "cg_local.h"

typedef struct debuffVisualsData_s
{
    int lastFireEFXTime;
    int stunStartTime;
} debuffVisualsData_t;

void JKG_PlayerDebuffVisuals ( centity_t *cent, refEntity_t *refEntity );

#endif
