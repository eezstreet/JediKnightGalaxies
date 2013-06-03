// Defines bit. namespace
// Handles bitwise operations

#include "../game/g_local.h"
#include "../game/q_shared.h"

#include "glua.h"

static int GLua_Bit_Or(lua_State *L) {
	int num1 = luaL_checkinteger(L, 1);
	int num2 = luaL_checkinteger(L, 1);
	lua_pushinteger(L, num1 | num2);
	return 1;
}

static int GLua_Bit_And(lua_State *L) {
	int num1 = luaL_checkinteger(L, 1);
	int num2 = luaL_checkinteger(L, 1);
	lua_pushinteger(L, num1 & num2);
	return 1;
}

static int GLua_Bit_Xor(lua_State *L) {
	int num1 = luaL_checkinteger(L, 1);
	int num2 = luaL_checkinteger(L, 1);
	lua_pushinteger(L, num1 ^ num2);
	return 1;
}

static int GLua_Bit_Not(lua_State *L) {
	int num1 = luaL_checkinteger(L, 1);
	lua_pushinteger(L, ~num1);
	return 1;
}

static int GLua_Bit_ShiftL(lua_State *L) {
	int num1 = luaL_checkinteger(L, 1);
	int bits = luaL_checkinteger(L, 2);
	lua_pushinteger(L, num1 << bits);
	return 1;
}

static int GLua_Bit_ShiftRZ(lua_State *L) {
	unsigned int num1 = luaL_checkinteger(L, 1);
	unsigned int bits = luaL_checkinteger(L, 2);
	lua_pushinteger(L, num1 >> bits);
	return 1;
}

static int GLua_Bit_ShiftR(lua_State *L) {
	int num1 = luaL_checkinteger(L, 1);
	int bits = luaL_checkinteger(L, 2);
	lua_pushinteger(L, num1 >> bits);
	return 1;
}

static const struct luaL_reg bit_f [] = {
	{"Or", GLua_Bit_Or},
	{"And", GLua_Bit_And},
	{"Xor", GLua_Bit_Xor},
	{"Not", GLua_Bit_Not},
	{"ShiftL", GLua_Bit_ShiftL},
	{"ShiftR", GLua_Bit_ShiftR},
	{"ShiftRZ", GLua_Bit_ShiftRZ},
	{NULL, NULL},
};

void GLua_Define_Bit(lua_State *L) {

	STACKGUARD_INIT(L)

	luaL_register(L, "bit", bit_f);
	lua_pop(L,1);

	STACKGUARD_CHECK(L)
}