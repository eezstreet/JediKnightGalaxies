// Defines the cvar object


#include "../game/g_local.h"
#include "../game/q_shared.h"

#include "glua.h"

GLua_Data_Cvar_t *GLua_CheckCvar(lua_State *L, int idx) {
	GLua_Data_Cvar_t *data;
	if (!ValidateObject(L, idx, GO_CVAR)) {
		luaL_typerror(L, idx, "Cvar");
	}
	data = (GLua_Data_Cvar_t *)lua_touserdata(L,1);
	return data;
}

void GLua_Cvar_UpdateCvar(GLua_Data_Cvar_t *cv) {
	cv->lastModifiedCount = cv->cvar.modificationCount;
	trap_Cvar_Update(&cv->cvar);
	cv->lastUpdate = level.time;
}

static int GLua_Cvar_GetString(lua_State *L) {
	GLua_Data_Cvar_t *cv = GLua_CheckCvar(L, 1);

	if (cv->lastUpdate != level.time) {
		GLua_Cvar_UpdateCvar(cv);
	}

	lua_pushstring(L,cv->cvar.string);
	return 1;
}

static int GLua_Cvar_GetFloat(lua_State *L) {
	GLua_Data_Cvar_t *cv = GLua_CheckCvar(L, 1);

	if (cv->lastUpdate != level.time) {
		GLua_Cvar_UpdateCvar(cv);
	}

	lua_pushnumber(L,cv->cvar.value);
	return 1;
}

static int GLua_Cvar_GetInteger(lua_State *L) {
	GLua_Data_Cvar_t *cv = GLua_CheckCvar(L, 1);

	if (cv->lastUpdate != level.time) {
		GLua_Cvar_UpdateCvar(cv);
	}

	lua_pushinteger(L,cv->cvar.integer);
	return 1;
}

static int GLua_Cvar_GetBool(lua_State *L) {
	GLua_Data_Cvar_t *cv = GLua_CheckCvar(L, 1);

	if (cv->lastUpdate != level.time) {
		GLua_Cvar_UpdateCvar(cv);
	}

	lua_pushboolean(L,cv->cvar.integer);
	return 1;
}

static int GLua_Cvar_GetName(lua_State *L) {
	GLua_Data_Cvar_t *cv = GLua_CheckCvar(L, 1);

	lua_pushstring(L,cv->name);
	return 1;
}

static int GLua_Cvar_IsModified(lua_State *L) {
	GLua_Data_Cvar_t *cv = GLua_CheckCvar(L, 1);

	if (cv->lastUpdate != level.time) {
		GLua_Cvar_UpdateCvar(cv);
	}

	lua_pushboolean(L, (cv->cvar.modificationCount != cv->lastModifiedCount));
	return 1;
}

static int GLua_Cvar_Update(lua_State *L) {
	GLua_Data_Cvar_t *cv = GLua_CheckCvar(L, 1);
	GLua_Cvar_UpdateCvar(cv);
	return 0;
}

static int GLua_Cvar_Set(lua_State *L) {
	GLua_Data_Cvar_t *cv = GLua_CheckCvar(L, 1);
	const char *newvalue = luaL_checkstring(L,2);
	trap_Cvar_Set(cv->name, newvalue);
	GLua_Cvar_UpdateCvar(cv);
	return 0;
}

static int GLua_Cvar_Index(lua_State *L) {
	GLua_Data_Cvar_t *cv = GLua_CheckCvar(L, 1);
	const char *key = luaL_checkstring(L,2);
	GLUA_UNUSED(cv);

	lua_getmetatable(L,1);
	lua_getfield(L,-1,key);
	return 1;
}

static int GLua_Cvar_NewIndex(lua_State *L) {
	GLua_Data_Cvar_t *cv = GLua_CheckCvar(L, 1);
	GLUA_UNUSED(cv);
	return 0; // Just deny it
}

static int GLua_Cvar_ToString(lua_State *L) {
	GLua_Data_Cvar_t *cv = GLua_CheckCvar(L, 1);
	lua_pushfstring(L, "Cvar ['%s']", cv->name);
	return 1;
}

static const struct luaL_reg cvar_m [] = {
	{"__index", GLua_Cvar_Index},
	{"__newindex", GLua_Cvar_NewIndex},
	{"__tostring", GLua_Cvar_ToString},
	{"GetString", GLua_Cvar_GetString},
	{"GetFloat", GLua_Cvar_GetFloat},
	{"GetInteger", GLua_Cvar_GetInteger},
	{"GetBool", GLua_Cvar_GetBool},
	{"GetName", GLua_Cvar_GetName},
	{"IsModified", GLua_Cvar_IsModified},
	{"Update", GLua_Cvar_Update},
	{"Set", GLua_Cvar_Set},
	{NULL, NULL},
};

static int GLua_Cvar_Create(lua_State *L) {
	const char *cvarname = luaL_checkstring(L,1);
	const char *defval = luaL_checkstring(L,2);
	int flags = luaL_checkinteger(L,3);
	GLua_Data_Cvar_t *cv = (GLua_Data_Cvar_t *)lua_newuserdata(L, sizeof(GLua_Data_Cvar_t));
	memset(cv,0,sizeof(GLua_Data_Cvar_t));
	trap_Cvar_Register(&cv->cvar, cvarname, defval, flags);
	cv->lastUpdate = level.time;
	Q_strncpyz(cv->name, cvarname,sizeof(cv->name));
	luaL_getmetatable(L,"Cvar");
	lua_setmetatable(L,-2);
	return 1;
}

void GLua_Define_Cvar(lua_State *L) {

	STACKGUARD_INIT(L)

	lua_pushcclosure(L, GLua_Cvar_Create, 0);
	lua_setglobal(L, "CreateCvar");
	
	luaL_newmetatable(L,"Cvar");
	luaL_register(L, NULL, cvar_m);
	
	lua_pushstring(L,"ObjID");
	lua_pushinteger(L, GO_CVAR);
	lua_settable(L,-3);

	lua_pushstring(L,"ObjName");
	lua_pushstring(L,"Cvar");
	lua_settable(L,-3);

	lua_pop(L,1);

	STACKGUARD_CHECK(L)
}