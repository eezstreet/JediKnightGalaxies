// Defines file. namespace

#include "../game/g_local.h"
#include "../game/q_shared.h"

#include "glua.h"


static int GLua_File_Read(lua_State *L) {
	fileHandle_t f;
	int len;
	void *buff;

	len = trap_FS_FOpenFile(va("glua/data/%s", luaL_checkstring(L,1)), &f, FS_READ);
	if (len < 0) {
		lua_pushnil(L);
		return 1;
	} else {
		buff = malloc(len);
		trap_FS_Read(buff, len, f);
		lua_pushlstring(L, (const char *)buff, len);
		free(buff);
		trap_FS_FCloseFile(f);
		return 1;
	}
}

static int GLua_File_Exists(lua_State *L) {
	int len;
	fileHandle_t f;
	const char *file = luaL_checkstring(L,1);

	if (!file || !file[0]) {
		lua_pushboolean(L, 0);
		return 1;
	}
	if (file[0] == '/') {
		len = trap_FS_FOpenFile(&file[1], &f, FS_READ);
	} else {
		len = trap_FS_FOpenFile(va("glua/data/%s", file), &f, FS_READ);
	}
	trap_FS_FCloseFile(f);

	if (len < 0) {
		lua_pushboolean(L, 0);
	} else {
		lua_pushboolean(L, 1);
	}
	return 1;
}

static int GLua_File_Write(lua_State *L) {
	fileHandle_t f;
	int len;
	const void *buff = lua_tolstring(L, 2, (size_t *)&len);

	trap_FS_FOpenFile(va("glua/data/%s", luaL_checkstring(L,1)), &f, FS_WRITE);
	trap_FS_Write(buff, len, f);
	trap_FS_FCloseFile(f);
	return 0;
}

static int GLua_File_ListFiles(lua_State *L) {
	char buff[16384];
	const char *buffpt;
	int count;
	int i;
	count = trap_FS_GetFileList(luaL_checkstring(L,1), luaL_optstring(L,2,""), &buff[0], sizeof(buff));
	lua_newtable(L);
	for (i=0, buffpt = &buff[0]; i < count; i++, buffpt += strlen(buffpt) + 1) {
		lua_pushstring(L, buffpt);
		lua_rawseti(L,-2,i+1);
	}
	return 1;
}


static const struct luaL_reg file_f [] = {
	{"Read", GLua_File_Read},
	{"Write", GLua_File_Write},
	{"Exists", GLua_File_Exists},
	{"ListFiles", GLua_File_ListFiles},
	{NULL, NULL},
};

void GLua_Define_File(lua_State *L) {
	STACKGUARD_INIT(L)

	luaL_register(L, "file", file_f);
	lua_pop(L,1);

	STACKGUARD_CHECK(L)
}