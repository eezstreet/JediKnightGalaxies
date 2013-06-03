#ifndef JKG_NAVMESH_CREATOR_H
#define JKG_NAVMESH_CREATOR_H

#include "g_local.h"

void JKG_Nav_Init ( const char *mapname );
void JKG_Nav_Shutdown ( void );
void JKG_Nav_Editor_Run ( void );

void JKG_Nav_Cmd_Generate_f ( gentity_t *ent );
void JKG_Nav_Cmd_MarkWalkableSurface_f ( gentity_t *ent );
void JKG_Nav_CreateNavMesh ( const char *mapname );

#endif
