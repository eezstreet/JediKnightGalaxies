// Vector library

// Vector factory function

#include "../game/g_local.h"
#include "../game/q_shared.h"

//#include <math.h>
#include "glua.h"

int GLua_IsVector(lua_State *L, int idx) {
	//GLuaVec_t *vec;
	if (!ValidateObject(L, idx, GO_VECTOR)) {
		return 0;
	}
	return 1;
}

GLuaVec_t *GLua_CheckVector(lua_State *L, int idx) {
	//GLuaVec_t *vec;
	if (!ValidateObject(L, idx, GO_VECTOR)) {
		luaL_typerror(L, idx, "Vector");
	}
	return (GLuaVec_t *)lua_touserdata(L,idx);
}

static int GLua_Vector_Create(lua_State *L) {
	GLuaVec_t *vec;
	vec = (GLuaVec_t *)lua_newuserdata(L,12);
	if (lua_type(L,1) == LUA_TSTRING) {
		sscanf( luaL_checkstring(L,1), "%f %f %f", &vec->x, &vec->y, &vec->z );
	} else {
		vec->x = luaL_checknumber(L,1);
		vec->y = luaL_checknumber(L,2);
		vec->z = luaL_checknumber(L,3);
	}
	luaL_getmetatable(L,"Vector");
	lua_setmetatable(L,-2);
	return 1;
}

void GLua_PushVector(lua_State *L, float x, float y, float z) {
	GLuaVec_t *vec;
	vec = (GLuaVec_t *)lua_newuserdata(L,12);
	vec->x = x;
	vec->y = y;
	vec->z = z;
	luaL_getmetatable(L,"Vector");
	lua_setmetatable(L,-2);
	return;
}

// Vector object definitions

// vector.__Index(self, key)
static int GLua_Vector_Index(lua_State *L) {
	GLuaVec_t *vec;
	const char *key;
	vec = GLua_CheckVector(L, 1);
	key = lua_tostring(L,2);
	
	// See if this key is a function/constant in the metatable
	lua_getmetatable(L,1);
	lua_getfield(L,-1,key);
	if (!lua_isnil(L,-1)) {
		return 1;
	}
	// its not a meta function, check if its 'x', 'y' or 'z'
	if (!strcmp(key,"x")) {
		lua_pushnumber(L,vec->x);
		return 1;
	}
	if (!strcmp(key,"y")) {
		lua_pushnumber(L,vec->y);
		return 1;
	}
	if (!strcmp(key,"z")) {
		lua_pushnumber(L,vec->z);
		return 1;
	}
	// None of these, we requested a nonexisting key, return nil
	lua_pushnil(L);
	return 1;
}

// vector.__NewIndex(self, key, value)
static int GLua_Vector_NewIndex(lua_State *L) {
	GLuaVec_t *vec;
	const char *key;
	float value;

	vec = GLua_CheckVector(L,1);
	key = lua_tostring(L,2);

	// its not a meta function, check if its 'x', 'y' or 'z'
	if (!strcmp(key,"x")) {
		value = luaL_checknumber(L,3);
		vec->x = value;
		return 0;
	}
	if (!strcmp(key,"y")) {
		value = luaL_checknumber(L,3);
		vec->y = value;
		return 0;
	}
	if (!strcmp(key,"z")) {
		value = luaL_checknumber(L,3);
		vec->z = value;
		return 0;
	}
	return 0;
}

// vector.__tostring(self)
static int GLua_Vector_ToString(lua_State *L) {
	GLuaVec_t *vec;
	vec = GLua_CheckVector(L, 1);
	
	lua_pushfstring(L, "Vector (%f %f %f)",vec->x, vec->y, vec->z);
	return 1;
}

// vector.__add(vec1, vec2)
static int GLua_Vector_Add(lua_State *L) {
	GLuaVec_t *vec1;
	GLuaVec_t *vec2;
	GLuaVec_t *vec3;

	vec1 = GLua_CheckVector(L,1);
	vec2 = GLua_CheckVector(L,2);
	
	vec3 = (GLuaVec_t *)lua_newuserdata(L,12);
	vec3->x = vec1->x + vec2->x;
	vec3->y = vec1->y + vec2->y;
	vec3->z = vec1->z + vec2->z;
	luaL_getmetatable(L,"Vector");
	lua_setmetatable(L,-2);
	return 1;
}

// vector.__sub(vec1, vec2)
static int GLua_Vector_Sub(lua_State *L) {
	GLuaVec_t *vec1;
	GLuaVec_t *vec2;
	GLuaVec_t *vec3;

	vec1 = GLua_CheckVector(L,1);
	vec2 = GLua_CheckVector(L,2);
	
	vec3 = (GLuaVec_t *)lua_newuserdata(L,12);
	vec3->x = vec1->x - vec2->x;
	vec3->y = vec1->y - vec2->y;
	vec3->z = vec1->z - vec2->z;
	luaL_getmetatable(L,"Vector");
	lua_setmetatable(L,-2);
	return 1;
}

// vector.__eq(vec1, vec2)
static int GLua_Vector_Eq(lua_State *L) {
	GLuaVec_t *vec1;
	GLuaVec_t *vec2;

	vec1 = GLua_CheckVector(L,1);
	vec2 = GLua_CheckVector(L,2);
	
	if ((vec1->x == vec2->x) && (vec1->y == vec2->y) && (vec1->z == vec2->z)) {
		lua_pushboolean(L,1);
	} else {
		lua_pushboolean(L,0);
	}
	return 1;
}

// vector.__mul(vec1, num)
static int GLua_Vector_Mul(lua_State *L) {
	GLuaVec_t *vec1;
	GLuaVec_t *vec2;
	float factor;

	vec1 = GLua_CheckVector(L,1);
	factor = luaL_checknumber(L,2);

	vec2 = (GLuaVec_t *)lua_newuserdata(L,12);
	vec2->x = vec1->x * factor;
	vec2->y = vec1->y * factor;
	vec2->z = vec1->z * factor;
	luaL_getmetatable(L,"Vector");
	lua_setmetatable(L,-2);

	return 1;
}

// vector.__div(vec1, num)
static int GLua_Vector_Div(lua_State *L) {
	GLuaVec_t *vec1;
	GLuaVec_t *vec2;
	float factor;

	vec1 = GLua_CheckVector(L,1);
	factor = luaL_checknumber(L,2);

	vec2 = (GLuaVec_t *)lua_newuserdata(L,12);
	vec2->x = vec1->x / factor;
	vec2->y = vec1->y / factor;
	vec2->z = vec1->z / factor;
	luaL_getmetatable(L,"Vector");
	lua_setmetatable(L,-2);

	return 1;
}

// vector.Length(vec1)
static int GLua_Vector_Length(lua_State *L) {
	GLuaVec_t *vec;

	vec = GLua_CheckVector(L,1);
	
	lua_pushnumber(L, sqrt((vec->x * vec->x)+(vec->y * vec->y)+(vec->z * vec->z)));
	return 1;
}

// vector.Normalize(vec1/self)
static int GLua_Vector_Normalize(lua_State *L) {
	GLuaVec_t *self;
	
	double len;

	self = GLua_CheckVector(L,1);

	len = sqrt((self->x * self->x)+(self->y * self->y)+(self->z * self->z));

	self->x = self->x / len;
	self->y = self->y / len;
	self->z = self->z / len; 

	return 0;
}

// vector.GetNormalize(vec1/self)
static int GLua_Vector_GetNormalized(lua_State *L) {
	GLuaVec_t *vec1;
	GLuaVec_t *vec2;
	double len;
	vec1 = GLua_CheckVector(L,1);

	len = sqrt((vec1->x * vec1->x)+(vec1->y * vec1->y)+(vec1->z * vec1->z));

	vec2 = (GLuaVec_t *)lua_newuserdata(L,12);
	vec2->x = vec1->x / len;
	vec2->y = vec1->y / len;
	vec2->z = vec1->z / len; 
	luaL_getmetatable(L,"Vector");
	lua_setmetatable(L,-2);
	return 1;
}

// vector.DotProduct(vec1/self, vec2)
static int GLua_Vector_DotProduct(lua_State *L) {
	GLuaVec_t *vec1;
	GLuaVec_t *vec2;

	vec1 = GLua_CheckVector(L,1);
	vec2 = GLua_CheckVector(L,2);
	
	return vec1->x*vec2->x + vec1->y*vec2->y + vec1->z*vec2->z;
}

// vector.Distance(self, vec)
static int GLua_Vector_Distance(lua_State *L) {
	GLuaVec_t *self;
	GLuaVec_t *vec;

	GLuaVec_t distvec;

	self = GLua_CheckVector(L,1);
	vec = GLua_CheckVector(L,2);
	
	distvec.x = self->x - vec->x;
	distvec.y = self->y - vec->y;
	distvec.z = self->z - vec->z;


	return sqrt(distvec.x*distvec.x + distvec.y*distvec.y + distvec.z*distvec.z);
}	

// vector.CrossProduct(self, vec)

static int GLua_Vector_CrossProduct(lua_State *L) {
	GLuaVec_t *self;
	GLuaVec_t *vec;
	GLuaVec_t *vec2;

	self = GLua_CheckVector(L,1);
	vec = GLua_CheckVector(L,2);
	
	vec2 = (GLuaVec_t *)lua_newuserdata(L,12);
	vec2->x = self->y*vec->z - self->z*vec->y; 
	vec2->y = self->z*vec->x - self->x*vec->z;
	vec2->z = self->x*vec->y - self->y*vec->x; 
	luaL_getmetatable(L,"Vector");
	lua_setmetatable(L,-2);
	return 1;
}

	//cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
	//cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
	//cross[2] = v1[0]*v2[1] - v1[1]*v2[0];

// vector.add(self, vec)
static int GLua_Vector_Add2(lua_State *L) {
	GLuaVec_t *self;
	GLuaVec_t *vec;

	self = GLua_CheckVector(L,1);
	vec = GLua_CheckVector(L,2);
	
	self->x += vec->x;
	self->y += vec->y;
	self->z += vec->z;
	return 0;
}

// vector.sub(self, vec)
static int GLua_Vector_Sub2(lua_State *L) {
	GLuaVec_t *self;
	GLuaVec_t *vec;

	self = GLua_CheckVector(L,1);
	vec = GLua_CheckVector(L,2);
	
	self->x -= vec->x;
	self->y -= vec->y;
	self->z -= vec->z;
	return 0;
}

// vector.Scale(self, scale)
static int GLua_Vector_Scale(lua_State *L) {
	GLuaVec_t *self;
	float scale;

	self = GLua_CheckVector(L,1);
	scale = luaL_checknumber(L,2);
	
	self->x *= scale;
	self->y *= scale;
	self->z *= scale;
	return 0;
}

static int GLua_Vector_ToAngles(lua_State *L) {
	GLuaVec_t *self;
	vec3_t vec2;

	self = GLua_CheckVector(L,1);

	vectoangles((const vec_t *)self, vec2);

	self->x = vec2[0];
	self->y = vec2[1];
	self->z = vec2[2];
	return 0;
}

static int GLua_Vector_ToForward(lua_State *L) {
	GLuaVec_t *self;
	vec3_t vec;
	vec3_t vec2;

	self = GLua_CheckVector(L,1);
	vec[0] = self->x;
	vec[1] = self->y;
	vec[2] = self->z;

	AngleVectors(vec, vec2, NULL, NULL);

	self->x = vec2[0];
	self->y = vec2[1];
	self->z = vec2[2];
	return 0;
}

static int GLua_Vector_ToRight(lua_State *L) {
	GLuaVec_t *self;
	vec3_t vec;
	vec3_t vec2;

	self = GLua_CheckVector(L,1);
	vec[0] = self->x;
	vec[1] = self->y;
	vec[2] = self->z;

	AngleVectors(vec, NULL, vec2, NULL);

	self->x = vec2[0];
	self->y = vec2[1];
	self->z = vec2[2];
	return 0;
}

static int GLua_Vector_ToUp(lua_State *L) {
	GLuaVec_t *self;
	vec3_t vec;
	vec3_t vec2;

	self = GLua_CheckVector(L,1);
	vec[0] = self->x;
	vec[1] = self->y;
	vec[2] = self->z;

	AngleVectors(vec, NULL, NULL, vec2);

	self->x = vec2[0];
	self->y = vec2[1];
	self->z = vec2[2];
	return 0;
}

static int GLua_Vector_GetAngles(lua_State *L) {
	GLuaVec_t *self;
	GLuaVec_t *vec;
	vec3_t vec2;

	self = GLua_CheckVector(L,1);

	vectoangles((const vec_t *)self, vec2);

	vec = (GLuaVec_t *)lua_newuserdata(L,12);
	vec->x = vec2[0];
	vec->y = vec2[1];
	vec->z = vec2[2];
	luaL_getmetatable(L,"Vector");
	lua_setmetatable(L,-2);

	return 1;
}

static int GLua_Vector_GetForward(lua_State *L) {
	GLuaVec_t *self;
	GLuaVec_t *vec3;
	vec3_t vec;
	vec3_t vec2;

	self = GLua_CheckVector(L,1);
	vec[0] = self->x;
	vec[1] = self->y;
	vec[2] = self->z;

	AngleVectors(vec, vec2, NULL, NULL);

	vec3 = (GLuaVec_t *)lua_newuserdata(L,12);
	vec3->x = vec2[0];
	vec3->y = vec2[1];
	vec3->z = vec2[2];
	luaL_getmetatable(L,"Vector");
	lua_setmetatable(L,-2);
	return 1;
}

static int GLua_Vector_GetRight(lua_State *L) {
	GLuaVec_t *self;
	GLuaVec_t *vec3;
	vec3_t vec;
	vec3_t vec2;

	self = GLua_CheckVector(L,1);
	vec[0] = self->x;
	vec[1] = self->y;
	vec[2] = self->z;

	AngleVectors(vec, NULL, vec2, NULL);

	vec3 = (GLuaVec_t *)lua_newuserdata(L,12);
	vec3->x = vec2[0];
	vec3->y = vec2[1];
	vec3->z = vec2[2];
	luaL_getmetatable(L,"Vector");
	lua_setmetatable(L,-2);
	return 1;
}

static int GLua_Vector_GetUp(lua_State *L) {
	GLuaVec_t *self;
	GLuaVec_t *vec3;
	vec3_t vec;
	vec3_t vec2;

	self = GLua_CheckVector(L,1);
	vec[0] = self->x;
	vec[1] = self->y;
	vec[2] = self->z;

	AngleVectors(vec, NULL, NULL, vec2);

	vec3 = (GLuaVec_t *)lua_newuserdata(L,12);
	vec3->x = vec2[0];
	vec3->y = vec2[1];
	vec3->z = vec2[2];
	luaL_getmetatable(L,"Vector");
	lua_setmetatable(L,-2);
	return 1;
}

static int GLua_Vector_Copy(lua_State *L) {
	GLuaVec_t *self;
	GLuaVec_t *vec2;

	self = GLua_CheckVector(L,1);
	
	vec2 = (GLuaVec_t *)lua_newuserdata(L,12);
	vec2->x = self->x;
	vec2->y = self->y;
	vec2->z = self->z;
	luaL_getmetatable(L,"Vector");
	lua_setmetatable(L,-2);
	return 1;
}

static const struct luaL_reg vector_m [] = {
	{"__index", GLua_Vector_Index},
	{"__newindex", GLua_Vector_NewIndex},
	{"__add", GLua_Vector_Add},
	{"__sub", GLua_Vector_Sub},
	{"__eq", GLua_Vector_Eq},
	{"__mul", GLua_Vector_Mul},
	{"__div", GLua_Vector_Div},
	{"__tostring", GLua_Vector_ToString},
	{"Sub", GLua_Vector_Sub2},
	{"Add", GLua_Vector_Add2},
	{"Scale", GLua_Vector_Scale},
	{"Length", GLua_Vector_Length},
	{"Normalize", GLua_Vector_Normalize},
	{"GetNormalized", GLua_Vector_GetNormalized},
	{"DotProduct", GLua_Vector_DotProduct},
	{"CrossProduct", GLua_Vector_CrossProduct},
	{"ToAngles", GLua_Vector_ToAngles},
	{"ToForward", GLua_Vector_ToForward},
	{"ToRight", GLua_Vector_ToRight},
	{"ToUp", GLua_Vector_ToUp},
	{"GetAngles", GLua_Vector_GetAngles},
	{"GetForward", GLua_Vector_GetForward},
	{"GetRight", GLua_Vector_GetRight},
	{"GetUp", GLua_Vector_GetUp},
	{"Copy", GLua_Vector_Copy},
	{NULL, NULL},
};


// Registration

void GLua_Define_Vector(lua_State *L) {

	STACKGUARD_INIT(L)

	lua_pushcclosure(L,GLua_Vector_Create,0);
	lua_setglobal(L,"Vector");

	luaL_newmetatable(L,"Vector");
	luaL_register(L, NULL, vector_m);

	lua_pushstring(L,"ObjID");
	lua_pushinteger(L,GO_VECTOR);
	lua_settable(L,-3);

	lua_pushstring(L,"ObjName");
	lua_pushstring(L,"Vector");
	lua_settable(L,-3);

	lua_pop(L,1);

	STACKGUARD_CHECK(L)
}