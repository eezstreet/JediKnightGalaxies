// Implements prng. namespace
// Encapsulated Pseudo-random number generation
// Works indentically to math.random and math.randomseed, but without affecting their states

#include "../game/g_local.h"
#include "../game/q_shared.h"

#include <stdlib.h>

#include "glua.h"

unsigned int *GLua_CheckPRNG(lua_State *L, int idx) {
	unsigned int *seed;
	if (!ValidateObject(L, idx, GO_PRNG)) {
		luaL_typerror(L, idx, "PRNG");
	}

	seed = (unsigned int *)lua_touserdata(L, idx);
	return seed;
}


static int GLua_PRNG_Index(lua_State *L) {
	/*unsigned int *seed = */
	const char *key;
	GLua_CheckPRNG(L, 1);
	key = lua_tostring(L,2);
	lua_getmetatable(L,1);
	lua_getfield(L,-1,key);
	return 1;
}

static int GLua_PRNG_NewIndex(lua_State *L) {
	return 0;
}

static int GLua_PRNG_ToString(lua_State *L) {
	GLua_CheckPRNG(L, 1);
	lua_pushstring(L, "PRNG");
	return 1;
}

#define PRNG_MAX 0x7fff

static int GLua_PRNG_Rand(lua_State *L) {
	unsigned int *seed = GLua_CheckPRNG(L, 1);
	unsigned int randval;
	lua_Number r;

	*seed = (69069 * *seed + 1);

	randval = *seed & 0x7fff; 
	
	/* the `%' avoids the (rare) case of r==1, and is needed also because on
	some systems (SunOS!) `rand()' may return a value larger than RAND_MAX */
	r = (lua_Number)(randval % PRNG_MAX) / (lua_Number)PRNG_MAX;
	switch (lua_gettop(L)) {  /* check number of arguments */
		case 1: {  /* no arguments */
			lua_pushnumber(L, r);  /* Number between 0 and 1 */
			break;
		}
		case 2: {  /* only upper limit */
			int u = luaL_checkint(L, 2);
			luaL_argcheck(L, 1<=u, 2, "interval is empty");
			lua_pushnumber(L, floor(r*u)+1);  /* int between 1 and `u' */
			break;
		}
		case 3: {  /* lower and upper limits */
			int l = luaL_checkint(L, 2);
			int u = luaL_checkint(L, 3);
			luaL_argcheck(L, l<=u, 3, "interval is empty");
			lua_pushnumber(L, floor(r*(u-l+1))+l);  /* int between `l' and `u' */
			break;
		}
		default:
			return luaL_error(L, "wrong number of arguments");
	}
	return 1;
}

static int GLua_PRNG_Seed(lua_State *L) {
	unsigned int *seed = GLua_CheckPRNG(L, 1);
	unsigned int newSeed = luaL_checkinteger(L, 2);

	*seed = newSeed;
	return 0;
}

static const struct luaL_reg prng_m [] = {
	// Meta functions
	{"__index", GLua_PRNG_Index},
	{"__newindex", GLua_PRNG_NewIndex},
	{"__tostring", GLua_PRNG_ToString},
	// PRNG functions
	{"Rand", GLua_PRNG_Rand},
	{"Seed", GLua_PRNG_Seed},
	{NULL, NULL},
};

// Creates new pseudo-random number generator
static int GLua_PRNG_Create(lua_State *L) {
	unsigned int initseed = lua_tointeger(L, 1);
	unsigned int *seed;

	seed = (unsigned int *)lua_newuserdata(L, 4);

	*seed = initseed;

	luaL_getmetatable(L,"PRNG");
	lua_setmetatable(L,-2);
	return 1;
}

static const struct luaL_reg prng_f [] = {
	{"Create", GLua_PRNG_Create},
	{NULL, NULL},
};

void GLua_Define_PRNG(lua_State *L) {
	STACKGUARD_INIT(L)

	luaL_register(L, "prng", prng_f);
	lua_pop(L,1);

	luaL_newmetatable(L,"PRNG");
	luaL_register(L, NULL, prng_m);
	
	lua_pushstring(L,"ObjID");
	lua_pushinteger(L, GO_PRNG);
	lua_settable(L,-3);

	lua_pushstring(L,"ObjName");
	lua_pushstring(L,"PRNG");
	lua_settable(L,-3);

	lua_pop(L,1);

	STACKGUARD_CHECK(L)
}