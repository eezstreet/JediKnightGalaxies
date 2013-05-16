//////////////////////////////////////
// Keypairs system for Jedi Knight Galaxies
//
// Based on code from RoboPhred

#ifndef JKG_KEYPAIRS_H
#define JKG_KEYPAIRS_H

typedef struct KeyPair_s{
	char *key;
	char *value;
}KeyPair_t;

//typedef struct jkgArray_s jkgArray_t;
typedef struct KeyPairSet_s{
	unsigned int count;	
	KeyPair_t *pairs;
	//jkgArray_t *pairs;
}KeyPairSet_t;


unsigned int JKG_Pairs_Add(KeyPairSet_t *set, const char *key, const char *value);
void JKG_Pairs_Remove(KeyPairSet_t *set, int index);
void JKG_Pairs_Clear(KeyPairSet_t *set);

int JKG_Pairs_FindKey(KeyPairSet_t *set, const char *key);
char* JKG_Pairs_GetKey(KeyPairSet_t *set, const char *key);
void JKG_Pairs_SetKey(KeyPairSet_t *set, const char *key, const char *value);
void JKG_Pairs_Merge(KeyPairSet_t *base, KeyPairSet_t *add);

#endif
