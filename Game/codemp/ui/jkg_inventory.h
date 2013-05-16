#ifndef JKG_INVENTORY_H
#define JKG_INVENTORY_H

#include "../game/q_shared.h"

void JKG_Inventory_OpenDialog ( char **args );
void JKG_Inventory_CloseDialog ( char **args );

void JKG_Inventory_UpdateNotify(int msg);
int JKG_Inventory_FeederCount ( void );
qboolean JKG_Inventory_FeederSelection ( int index );
const char *JKG_Inventory_FeederItemText ( int index, int column, qhandle_t *handle1, qhandle_t *handle2, qhandle_t *handle3 );

void JKG_Inventory_Script_Button ( char **args );
void JKG_Inventory_ACI_Button ( char **args );

#endif
