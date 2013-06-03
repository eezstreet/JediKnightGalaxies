// Defines the encoding. namespace
// Implements base128 and base64 encoding

#include "../game/g_local.h"
#include "../game/q_shared.h"

#include "encoding/base64.h"
#include "encoding/base128.h"

#include <stdlib.h>

#include "glua.h"


/* Encodes data to base64 */
static int GLua_Encoding_EncodeBase64(lua_State *L) {
	unsigned int len;
	unsigned int encodelen;
	const char *data = luaL_checklstring(L, 1, &len);
	char *b64;


	encodelen = Base64_EncodeLength(len);
	b64 = (char *)malloc(encodelen);
	if (!b64) {
		return 0;
	}
	Base64_Encode(data, len, b64, encodelen);
	lua_pushstring(L, b64);
	free(b64);
	return 1;
}

/* Decodes data from base64 */
static int GLua_Encoding_DecodeBase64(lua_State *L) {
	unsigned int len;
	unsigned int decodelen;
	const char *b64 = luaL_checklstring(L, 1, &len);
	char *data;

	decodelen = Base64_DecodeLength(len, b64, 0);
	data = (char *)malloc(decodelen);
	if (!data) {
		return 0;
	}
	Base64_Decode(b64, len, data, decodelen);
	lua_pushlstring(L, data, decodelen);
	free(data);
	return 1;
}

/* Encodes data to url-safe base64 */
static int GLua_Encoding_EncodeBase64URL(lua_State *L) {
	unsigned int len;
	unsigned int encodelen;
	const char *data = luaL_checklstring(L, 1, &len);
	char *b64;


	encodelen = Base64_EncodeLength(len);
	b64 = (char *)malloc(encodelen);
	if (!b64) {
		return 0;
	}
	Base64_EncodeURL((const unsigned char *)data, len, b64, encodelen);
	lua_pushstring(L, b64);
	free(b64);
	return 1;
}

/* Decodes data from url-safe base64 */
static int GLua_Encoding_DecodeBase64URL(lua_State *L) {
	unsigned int len;
	unsigned int decodelen;
	const char *b64 = luaL_checklstring(L, 1, &len);
	char *data;

	decodelen = Base64_DecodeLength(len, b64, 1);
	data = (char *)malloc(decodelen);
	if (!data) {
		return 0;
	}
	Base64_DecodeURL(b64, len, data, decodelen);
	lua_pushlstring(L, data, decodelen);
	free(data);
	return 1;
}

/* Encodes data to base128 */
static int GLua_Encoding_EncodeBase128(lua_State *L) {
	unsigned int len;
	unsigned int encodelen;
	const char *data = luaL_checklstring(L, 1, &len);
	char *b128;


	encodelen = Base128_EncodeLength(len);
	b128 = (char *)malloc(encodelen);
	if (!b128) {
		return 0;
	}
	Base128_Encode(data, len, b128, encodelen);
	lua_pushstring(L, b128);
	free(b128);
	return 1;
}

/* Decodes data from base128 */
static int GLua_Encoding_DecodeBase128(lua_State *L) {
	unsigned int len;
	unsigned int decodelen;
	const char *b128 = luaL_checklstring(L, 1, &len);
	char *data;

	decodelen = Base128_DecodeLength(len);
	data = (char *)malloc(decodelen);
	if (!data) {
		return 0;
	}
	Base128_Decode(b128, len, (void *)data, decodelen);
	lua_pushlstring(L, data, decodelen);
	free(data);
	return 1;
}

/* Encodes data into Hex notation */
static int GLua_Encoding_EncodeHex(lua_State *L) {
	unsigned int i;
	unsigned int len;
	const char *data = luaL_checklstring(L, 1, &len);
	luaL_Buffer B;
	unsigned char tmp;
	const char *table = "0123456789ABCDEF";
	
	luaL_buffinit(L, &B);
	for (i=0; i<len; i++) {
		tmp = (unsigned char)data[i];
		luaL_addchar(&B, table[(tmp >> 4)]);
		luaL_addchar(&B, table[(tmp & 0xF)]);
	}
	luaL_pushresult(&B);
	return 1;
}

/* Decodes data from Hex notation */
static int GLua_Encoding_DecodeHex(lua_State *L) {
	unsigned int i;
	unsigned int len;
	const char *data = luaL_checklstring(L, 1, &len);
	luaL_Buffer B;
	unsigned char newchar;
	int val = 0;
	unsigned char tmp;
	
	luaL_buffinit(L, &B);
	for (i=0; i<len; i += 2) {
		newchar = 0;
		tmp = data[i];
	
		if (tmp >= '0' && tmp <= '9') {
			val = tmp - '0';
		} else if (tmp >= 'a' && tmp <= 'f') {
			val = tmp - 'a' + 10;
		} else if (tmp >= 'a' && tmp <= 'f') {
			val = tmp - 'A' + 10;
		} else {
			luaL_error(L, "Cannot decode hex: Invalid character encountered: '%c'", tmp);
			return 0;
		}

		newchar = val << 4;
		
		tmp = data[i + 1];
	
		if (tmp >= '0' && tmp <= '9') {
			val = tmp - '0';
		} else if (tmp >= 'a' && tmp <= 'f') {
			val = tmp - 'a' + 10;
		} else if (tmp >= 'a' && tmp <= 'f') {
			val = tmp - 'A' + 10;
		} else {
			luaL_error(L, "Cannot decode hex: Invalid character encountered: '%c'", tmp);
		}

		newchar |= val;

		luaL_addchar(&B, newchar);
	}
	luaL_pushresult(&B);
	return 1;
}


static const struct luaL_reg encoding_f [] = {
	{"Base64Encode", GLua_Encoding_EncodeBase64},
	{"Base64Decode", GLua_Encoding_DecodeBase64},
	{"Base64URLEncode", GLua_Encoding_EncodeBase64URL},
	{"Base64URLDecode", GLua_Encoding_DecodeBase64URL},
	{"Base128Encode", GLua_Encoding_EncodeBase128},
	{"Base128Decode", GLua_Encoding_DecodeBase128},
	{"HexEncode", GLua_Encoding_EncodeHex},
	{"HexDecode", GLua_Encoding_DecodeHex},
	{NULL, NULL},
};

void GLua_Define_Encoding(lua_State *L) {

	STACKGUARD_INIT(L)

	luaL_register(L, "encoding", encoding_f);
	lua_pop(L,1);

	STACKGUARD_CHECK(L)
}