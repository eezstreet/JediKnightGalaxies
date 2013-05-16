#pragma once
#include "jkg_items.h"
#include "g_local.h"

//DIMA system
#define DIMA_PREALLOC	(4)

void JKG_Easy_DIMA_GlobalInit(void);
void JKG_Easy_DIMA_Add(inv_t *inventory, itemInstance_t item);
void JKG_Easy_DIMA_Cleanup(void);
void JKG_Easy_DIMA_Remove(inv_t *inventory, unsigned int invID);
int JKG_Easy_DIMA_CMPInternal(inv_t *inventory, char *c1);
int JKG_Easy_DIMA_CMPItemID(inv_t *inventory, unsigned int itemID);
void JKG_Easy_DIMA_CleanEntity(int entNum);
int JKG_Easy_DIMA_CMPItemID(inv_t *inventory, unsigned int itemID);
int JKG_Easy_DIMA_CMPInternal(inv_t *inventory, char *c1);
unsigned int JKG_Easy_GetItemIDFromInternal(const char *internalName);