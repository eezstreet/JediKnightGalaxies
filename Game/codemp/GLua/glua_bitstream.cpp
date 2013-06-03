// Defines the bitstream. namespace and the BitStreamReader and BitStreamWriter classes
// Implements bitstream functionality

#include "../game/g_local.h"
#include "../game/q_shared.h"

#include "encoding/bitstream.h"

#include <stdlib.h>

#include "glua.h"

typedef struct {
	int reading;
	bitstream_t stream;
	unsigned char data[1]; // Dynamic array
} GLua_Bitstream_t;

GLua_Bitstream_t *GLua_CheckBitStream(lua_State *L, int idx) {
	GLua_Bitstream_t *data;
	if (!ValidateObject(L, idx, GO_BITSTREAM)) {
		luaL_typerror(L, idx, "Bitstream");
	}

	data = (GLua_Bitstream_t *)lua_touserdata(L, idx);
	return data;
}

/* Creates a new bitstream for writing */
static int GLua_BitStream_Create(lua_State *L) {
	unsigned int length = luaL_checkinteger(L, 1);
	GLua_Bitstream_t *stream;

	if (length < 1) {
		luaL_error(L, "Cannot create a bitstream with a length of less than 1 byte");
	}

	stream = (GLua_Bitstream_t *)lua_newuserdata(L, sizeof(GLua_Bitstream_t) + length - 1);
	stream->reading = 0;
	BitStream_Init(&stream->stream, stream->data, length);

	luaL_getmetatable(L,"BitStream");
	lua_setmetatable(L,-2);
	return 1;
}

/* Creates a new bitstream for reading */
static int GLua_BitStream_Read(lua_State *L) {
	unsigned int length;
	const char *data = luaL_checklstring(L, 1, &length);
	GLua_Bitstream_t *stream;

	if (length < 1) {
		luaL_error(L, "Cannot create a bitstream with a length of less than 1 byte");
	}

	stream = (GLua_Bitstream_t *)lua_newuserdata(L, sizeof(GLua_Bitstream_t) + length - 1);
	stream->reading = 1;
	memcpy(stream->data, data, length);
	BitStream_Init(&stream->stream, stream->data, length);
	BitStream_BeginReading(&stream->stream);

	luaL_getmetatable(L,"BitStream");
	lua_setmetatable(L,-2);
	return 1;
}

static const struct luaL_reg bitstream_f [] = {
	{"Create", GLua_BitStream_Create},
	{"Read", GLua_BitStream_Read},
	{NULL, NULL},
};

static int GLua_BitStream_Index(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	const char *key = lua_tostring(L,2);
	GLUA_UNUSED(stream);
	lua_getmetatable(L,1);
	lua_getfield(L,-1,key);
	return 1;
}

static int GLua_BitStream_NewIndex(lua_State *L) {
	return 0;
}

static int GLua_BitStream_ToString(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	if (stream->reading) {
		lua_pushstring(L,va("BitStream [ Reading - %i / %i bytes ]", stream->stream.readcount, stream->stream.maxsize ));
	} else {
		lua_pushstring(L,va("BitStream [ Writing - %i / %i bytes ]", stream->stream.cursize, stream->stream.maxsize ));
	}
	return 1;
}

static int GLua_BitStream_WriteBits(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	int bits = luaL_checkinteger(L, 2);
	unsigned int val = luaL_checkinteger(L,3);
	
	if (stream->reading) {
		luaL_error(L, "Attempted to write to a read-only bitstream");
		return 0;
	}

	BitStream_WriteBits(&stream->stream, bits, val);
	
	return 0;
}

static int GLua_BitStream_WriteBool(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	int val = lua_toboolean(L,2);
	
	if (stream->reading) {
		luaL_error(L, "Attempted to write to a read-only bitstream");
		return 0;
	}

	BitStream_WriteBool(&stream->stream, val);
	
	return 0;
}

static int GLua_BitStream_WriteNibble(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	char val = luaL_checkinteger(L,2);
	
	if (stream->reading) {
		luaL_error(L, "Attempted to write to a read-only bitstream");
		return 0;
	}

	BitStream_WriteNibble(&stream->stream, val);
	
	return 0;
}

static int GLua_BitStream_WriteUNibble(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	unsigned char val = luaL_checkinteger(L,2);
	
	if (stream->reading) {
		luaL_error(L, "Attempted to write to a read-only bitstream");
		return 0;
	}

	BitStream_WriteUNibble(&stream->stream, val);
	
	return 0;
}

static int GLua_BitStream_WriteChar(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	char val = luaL_checkinteger(L,2);
	
	if (stream->reading) {
		luaL_error(L, "Attempted to write to a read-only bitstream");
		return 0;
	}

	BitStream_WriteChar(&stream->stream, val);
	
	return 0;
}


static int GLua_BitStream_WriteByte(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	unsigned char val = luaL_checkinteger(L,2);
	
	if (stream->reading) {
		luaL_error(L, "Attempted to write to a read-only bitstream");
		return 0;
	}

	BitStream_WriteByte(&stream->stream, val);
	
	return 0;
}

static int GLua_BitStream_WriteShort(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	short val = luaL_checkinteger(L,2);
	
	if (stream->reading) {
		luaL_error(L, "Attempted to write to a read-only bitstream");
		return 0;
	}

	BitStream_WriteShort(&stream->stream, val);
	
	return 0;
}

static int GLua_BitStream_WriteUShort(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	unsigned short val = luaL_checkinteger(L,2);
	
	if (stream->reading) {
		luaL_error(L, "Attempted to write to a read-only bitstream");
		return 0;
	}

	BitStream_WriteUShort(&stream->stream, val);
	
	return 0;
}


static int GLua_BitStream_WriteTriByte(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	int val = luaL_checkinteger(L,2);
	
	if (stream->reading) {
		luaL_error(L, "Attempted to write to a read-only bitstream");
		return 0;
	}

	BitStream_WriteTriByte(&stream->stream, val);
	
	return 0;
}

static int GLua_BitStream_WriteUTriByte(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	unsigned int val = luaL_checkinteger(L,2);
	
	if (stream->reading) {
		luaL_error(L, "Attempted to write to a read-only bitstream");
		return 0;
	}

	BitStream_WriteUTriByte(&stream->stream, val);
	
	return 0;
}

static int GLua_BitStream_WriteInt(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	int val = luaL_checkinteger(L,2);
	
	if (stream->reading) {
		luaL_error(L, "Attempted to write to a read-only bitstream");
		return 0;
	}

	BitStream_WriteInt(&stream->stream, val);
	
	return 0;
}

static int GLua_BitStream_WriteUInt(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	unsigned int val = luaL_checkinteger(L,2);
	
	if (stream->reading) {
		luaL_error(L, "Attempted to write to a read-only bitstream");
		return 0;
	}

	BitStream_WriteUInt(&stream->stream, val);
	
	return 0;
}

static int GLua_BitStream_WriteFloat(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	float val = luaL_checknumber(L,2);
	
	if (stream->reading) {
		luaL_error(L, "Attempted to write to a read-only bitstream");
		return 0;
	}

	BitStream_WriteFloat(&stream->stream, val);
	
	return 0;
}

static int GLua_BitStream_WriteDouble(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	double val = luaL_checknumber(L,2);
	
	if (stream->reading) {
		luaL_error(L, "Attempted to write to a read-only bitstream");
		return 0;
	}

	BitStream_WriteDouble(&stream->stream, val);
	
	return 0;
}

static int GLua_BitStream_WriteString(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	const char *val = luaL_checkstring(L,2);
	
	if (stream->reading) {
		luaL_error(L, "Attempted to write to a read-only bitstream");
		return 0;
	}

	BitStream_WriteString(&stream->stream, val);
	
	return 0;
}

static int GLua_BitStream_WriteData(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	unsigned int length;
	unsigned int deslength;
	const char *val = luaL_checklstring(L,2, &length);

	if (!lua_isnoneornil(L,3)) {
		deslength = luaL_checkinteger(L,3);
		if (length < deslength) {
			luaL_error(L, "BitStream_WriteData: Specified length is less than data length");
		}
		length = deslength;
	}
	
	if (stream->reading) {
		luaL_error(L, "Attempted to write to a read-only bitstream");
		return 0;
	}

	BitStream_WriteData(&stream->stream, (const unsigned char *)val, length);
	
	return 0;
}


static int GLua_BitStream_ReadBits(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	int bits = luaL_checkinteger(L, 2);
	unsigned int val;
	
	if (!stream->reading) {
		luaL_error(L, "Attempted to read from a write-only bitstream");
		return 0;
	}

	val = BitStream_ReadBits(&stream->stream, bits);
	
	lua_pushinteger(L, val);
	return 1;
}

static int GLua_BitStream_ReadBool(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	int val;
	
	if (!stream->reading) {
		luaL_error(L, "Attempted to read from a write-only bitstream");
		return 0;
	}

	val = BitStream_ReadBool(&stream->stream);
	
	lua_pushboolean(L, val);
	return 1;
}

static int GLua_BitStream_ReadNibble(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	char val;
	
	if (!stream->reading) {
		luaL_error(L, "Attempted to read from a write-only bitstream");
		return 0;
	}

	val = BitStream_ReadNibble(&stream->stream);
	
	lua_pushinteger(L, val);
	return 1;
}

static int GLua_BitStream_ReadUNibble(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	unsigned char val;
	
	if (!stream->reading) {
		luaL_error(L, "Attempted to read from a write-only bitstream");
		return 0;
	}

	val = BitStream_ReadUNibble(&stream->stream);
	
	lua_pushinteger(L, val);
	return 1;
}

static int GLua_BitStream_ReadChar(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	char val;
	
	if (!stream->reading) {
		luaL_error(L, "Attempted to read from a write-only bitstream");
		return 0;
	}

	val = BitStream_ReadChar(&stream->stream);
	
	lua_pushinteger(L, val);
	return 1;
}

static int GLua_BitStream_ReadByte(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	unsigned char val;
	
	if (!stream->reading) {
		luaL_error(L, "Attempted to read from a write-only bitstream");
		return 0;
	}

	val = BitStream_ReadByte(&stream->stream);
	
	lua_pushinteger(L, val);
	return 1;
}

static int GLua_BitStream_ReadShort(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	short val;
	
	if (!stream->reading) {
		luaL_error(L, "Attempted to read from a write-only bitstream");
		return 0;
	}

	val = BitStream_ReadShort(&stream->stream);
	
	lua_pushinteger(L, val);
	return 1;
}

static int GLua_BitStream_ReadUShort(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	unsigned short val;
	
	if (!stream->reading) {
		luaL_error(L, "Attempted to read from a write-only bitstream");
		return 0;
	}

	val = BitStream_ReadUShort(&stream->stream);
	
	lua_pushinteger(L, val);
	return 1;
}


static int GLua_BitStream_ReadTriByte(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	int val;
	
	if (!stream->reading) {
		luaL_error(L, "Attempted to read from a write-only bitstream");
		return 0;
	}

	val = BitStream_ReadTriByte(&stream->stream);
	
	lua_pushinteger(L, val);
	return 1;
}

static int GLua_BitStream_ReadUTriByte(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	unsigned int val;
	
	if (!stream->reading) {
		luaL_error(L, "Attempted to read from a write-only bitstream");
		return 0;
	}

	val = BitStream_ReadUTriByte(&stream->stream);
	
	lua_pushinteger(L, val);
	return 1;
}


static int GLua_BitStream_ReadInt(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	int val;
	
	if (!stream->reading) {
		luaL_error(L, "Attempted to read from a write-only bitstream");
		return 0;
	}

	val = BitStream_ReadInt(&stream->stream);
	
	lua_pushinteger(L, val);
	return 1;
}

static int GLua_BitStream_ReadUInt(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	unsigned int val;
	
	if (!stream->reading) {
		luaL_error(L, "Attempted to read from a write-only bitstream");
		return 0;
	}

	val = BitStream_ReadUInt(&stream->stream);
	
	lua_pushinteger(L, val);
	return 1;
}

static int GLua_BitStream_ReadFloat(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	float val;
	
	if (!stream->reading) {
		luaL_error(L, "Attempted to read from a write-only bitstream");
		return 0;
	}

	val = BitStream_ReadFloat(&stream->stream);
	
	lua_pushnumber(L, val);
	return 1;
}

static int GLua_BitStream_ReadDouble(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	double val;
	
	if (!stream->reading) {
		luaL_error(L, "Attempted to read from a write-only bitstream");
		return 0;
	}

	val = BitStream_ReadDouble(&stream->stream);
	
	lua_pushnumber(L, val);
	return 1;
}

static int GLua_BitStream_ReadString(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	luaL_Buffer B;
	char ch;
	
	if (!stream->reading) {
		luaL_error(L, "Attempted to read from a write-only bitstream");
		return 0;
	}

	luaL_buffinit(L, &B);
	while (1) {
		ch = BitStream_ReadChar(&stream->stream);
		if (!ch) {
			break;
		}
		luaL_addchar(&B, ch);
	}
	luaL_pushresult(&B);
	return 1;
}

static int GLua_BitStream_ReadData(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	unsigned int len = luaL_checkinteger(L, 2);
	luaL_Buffer B;
	char s[4];
	
	if (!stream->reading) {
		luaL_error(L, "Attempted to read from a write-only bitstream");
		return 0;
	}

	luaL_buffinit(L, &B);

	while (len >= 4) {
		*(unsigned int *)s = BitStream_ReadBits(&stream->stream, 32);
		luaL_addlstring(&B, s, 4);
		len -= 4;
	}
	switch (len)
	{
		case 3:
			luaL_addchar(&B, BitStream_ReadBits(&stream->stream, 8));
		case 2:
			luaL_addchar(&B, BitStream_ReadBits(&stream->stream, 8));
		case 1:
			luaL_addchar(&B, BitStream_ReadBits(&stream->stream, 8));
		default:
			break;
	}

	luaL_pushresult(&B);
	return 1;
}

static int GLua_BitStream_BytesRemaining(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	
	if (stream->reading) {
		lua_pushinteger(L, stream->stream.maxsize - stream->stream.readcount);
	} else {
		lua_pushinteger(L, stream->stream.maxsize - stream->stream.cursize);
	}
	return 1;
}

static int GLua_BitStream_BitsRemaining(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	
	lua_pushinteger(L, (stream->stream.maxsize * 8) - stream->stream.bit);

	return 1;
}


static int GLua_BitStream_GetData(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	
	if (stream->reading) {
		lua_pushlstring(L, reinterpret_cast<const char *>(stream->data), stream->stream.maxsize);
	} else {
		lua_pushlstring(L, reinterpret_cast<const char *>(stream->data), stream->stream.cursize);
	}
	return 1;
}

static int GLua_BitStream_GetLength(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	
	if (stream->reading) {
		lua_pushinteger(L, stream->stream.maxsize);
	} else {
		lua_pushinteger(L, stream->stream.cursize);
	}
	return 1;
}

static int GLua_BitStream_GetMaxLength(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	
	lua_pushinteger(L, stream->stream.maxsize);
	return 1;
}

static int GLua_BitStream_Reset(lua_State *L) {
	GLua_Bitstream_t *stream = GLua_CheckBitStream(L, 1);
	if (stream->reading) {
		BitStream_BeginReading(&stream->stream);
	} else {
		stream->stream.cursize = 0;
		stream->stream.bit = 0;
	}
	return 0;
}

static const struct luaL_reg bitstream_m [] = {
	// Meta functions
	{"__index", GLua_BitStream_Index},
	{"__newindex", GLua_BitStream_NewIndex},
	{"__tostring", GLua_BitStream_ToString},
	// Write functions
	{"WriteBits", GLua_BitStream_WriteBits},
	{"WriteBool", GLua_BitStream_WriteBool},
	{"WriteNibble", GLua_BitStream_WriteNibble},
	{"WriteUNibble", GLua_BitStream_WriteUNibble},
	{"WriteChar", GLua_BitStream_WriteChar},
	{"WriteByte", GLua_BitStream_WriteByte},
	{"WriteShort", GLua_BitStream_WriteShort},
	{"WriteUShort", GLua_BitStream_WriteUShort},
	{"WriteTriByte", GLua_BitStream_WriteTriByte},
	{"WriteUTriByte", GLua_BitStream_WriteUTriByte},
	{"WriteInt", GLua_BitStream_WriteInt},
	{"WriteUInt", GLua_BitStream_WriteUInt},
	{"WriteFloat", GLua_BitStream_WriteFloat},
	{"WriteDouble", GLua_BitStream_WriteDouble},
	{"WriteString", GLua_BitStream_WriteString},
	{"WriteData", GLua_BitStream_WriteData},
	// Read functions
	{"ReadBits", GLua_BitStream_ReadBits},
	{"ReadBool", GLua_BitStream_ReadBool},
	{"ReadNibble", GLua_BitStream_ReadNibble},
	{"ReadUNibble", GLua_BitStream_ReadUNibble},
	{"ReadChar", GLua_BitStream_ReadChar},
	{"ReadByte", GLua_BitStream_ReadByte},
	{"ReadShort", GLua_BitStream_ReadShort},
	{"ReadUShort", GLua_BitStream_ReadUShort},
	{"ReadTriByte", GLua_BitStream_ReadTriByte},
	{"ReadUTriByte", GLua_BitStream_ReadUTriByte},
	{"ReadInt", GLua_BitStream_ReadInt},
	{"ReadUInt", GLua_BitStream_ReadUInt},
	{"ReadFloat", GLua_BitStream_ReadFloat},
	{"ReadDouble", GLua_BitStream_ReadDouble},
	{"ReadString", GLua_BitStream_ReadString},
	{"ReadData", GLua_BitStream_ReadData},
	// General functions
	{"BytesRemaining", GLua_BitStream_BytesRemaining},
	{"BitsRemaining", GLua_BitStream_BitsRemaining},
	{"GetData", GLua_BitStream_GetData},
	{"GetLength", GLua_BitStream_GetLength},
	{"GetMaxLength", GLua_BitStream_GetMaxLength},
	{"Reset", GLua_BitStream_Reset},
	{NULL, NULL},
};

void GLua_Define_BitStream(lua_State *L) {
	STACKGUARD_INIT(L)

	luaL_register(L, "bitstream", bitstream_f);
	lua_pop(L,1);

	luaL_newmetatable(L,"BitStream");
	luaL_register(L, NULL, bitstream_m);
	
	lua_pushstring(L,"ObjID");
	lua_pushinteger(L, GO_BITSTREAM);
	lua_settable(L,-3);

	lua_pushstring(L,"ObjName");
	lua_pushstring(L,"BitStream");
	lua_settable(L,-3);

	lua_pop(L,1);

	STACKGUARD_CHECK(L)
}