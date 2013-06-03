//////////////////////////////////////
// Keypairs system for Jedi Knight Galaxies
//
// Based on code from RoboPhred

#include "g_local.h"

#include "jkg_dynarrays.h"
//#include "jkg_keypairs.h"


unsigned int JKG_Pairs_Add(KeyPairSet_t *set, const char *key, const char *value){
	unsigned int index = JKG_Arrays_AddArrayElement((void **)&set->pairs, sizeof(KeyPair_t), &set->count);
	G_NewString2((void **)&set->pairs[index].key, key);
	G_NewString2((void **)&set->pairs[index].value, value);
	return index;
}

void JKG_Pairs_Remove(KeyPairSet_t *set, int index){
	// Just free, if its NULL it'll bail out anyway, no need to check twice
	G_Free(set->pairs[index].key);
	G_Free(set->pairs[index].value);
	JKG_Arrays_RemoveArrayElement((void **)&set->pairs, index, sizeof(KeyPair_t), &set->count);
}

void JKG_Pairs_Clear(KeyPairSet_t *set){
	int i;
	for(i = 0; i<set->count; i++){
		G_Free(set->pairs[i].key);
		G_Free(set->pairs[i].value);
	}
	JKG_Arrays_RemoveAllElements((void **)&set->pairs);
	set->count = 0;
}

int JKG_Pairs_FindKey(KeyPairSet_t *set, const char *key){
	unsigned int i;
	for(i = 0; i<set->count; i++) {
		if(!Q_stricmp(set->pairs[i].key, key))
			return i;
	}
	return -1;
}

char* JKG_Pairs_GetKey(KeyPairSet_t *set, const char *key) {
	int i = JKG_Pairs_FindKey(set, key);
	if(i >= 0)
		return set->pairs[i].value;
	return NULL;
}

void JKG_Pairs_SetKey(KeyPairSet_t *set, const char *key, const char *value) {
	int index = JKG_Pairs_FindKey(set, key);
	if(index >= 0) {
		G_NewString2((void **)&set->pairs[index].value, value);
	}
	else {
		JKG_Pairs_Add(set, key, value);
	}
}

void JKG_Pairs_Merge(KeyPairSet_t *base, KeyPairSet_t *add) {
	unsigned int i;
	for(i = 0; i < add->count; i++) {
		JKG_Pairs_SetKey(base, add->pairs[i].key, add->pairs[i].value);
	}
}