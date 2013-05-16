// Jedi Knight Galaxies main interfaces

#include "../game/g_local.h"
#include "../game/q_shared.h"

#include "glua.h"

// This module defines the main interface for Jedi Knight Galaxies Lua



static int GLua_Print(lua_State *L) {
	/*const*/ char *msg;
	char *nl;
	char buff[16384] = {0};
	int args = lua_gettop(L);
	const char *res;
	int i;

	// Lets do this a lil different, concat all args and use that as the message ^^
	GLua_Push_ToString(L); // Ref to tostring (instead of a global lookup, in case someone changes it)
	for (i = 1; i <= args; i++) {
		lua_pushvalue(L,-1);
		lua_pushvalue(L, i);
		lua_call(L, 1, 1); // Assume this will never error out
		res = lua_tostring(L,-1);
		if (res) {
			Q_strcat(&buff[0], sizeof(buff), res);
		}
		lua_pop(L,1);
	}
	lua_pop(L,1);
	msg = &buff[0];
	// Since it'll ONLY do a single line, we parse the message, 
	//	looking for newline characters and use multiple G_Print's
	/*while (1) {
		nl = strstr(msg, "\n");
		if (!nl) {
			trap_Printf(msg);
			break;
		} else {
			*nl = 0;
			trap_Printf(msg);
			msg = nl + 1;
			*nl = '\n'; // Restore the source string ;)
		}
	}*/

	// New implementation with overflow asserts
	nl = msg;
	while (1) {
		if ( !(*nl) ) {
			if ( *msg ) {
				assert( strlen( msg ) < 4095 ); // Failsafe, this should never happen (4096 is engine MAXPRINTMSG, accomodate for the added \n in the next call)
				trap_Printf( va("%s\n", msg) );
			}
			break;
		}
		if ( *nl == '\n' ) {
			*nl = '\0';
			assert( strlen( msg ) < 4095 ); // Failsafe, this should never happen
			trap_Printf( va("%s\n", msg) );
			msg = nl + 1;
			*nl = '\n';
		}
		nl++;
	}
	return 0;
}

static int GLua_Include(lua_State *L) {
	const char *file;
	char buff[1024];
	int i;
	lua_Debug ar;
	int status;
	memset(&ar,0,sizeof(ar));

	if (lua_isnil(L,1)) {
		lua_pushboolean(L,0);
		return 1;
	}
	file = lua_tostring(L,1);
	if (file[0] == '/') {
		// base path off root
		status = GLua_LoadFile(L, &file[1]);
	} else {
		// Get current path and base off that
		lua_getstack(L,1,&ar);
		lua_getinfo(L,"S",&ar);
		Q_strncpyz(buff, ar.source, sizeof(buff));
		// Roll back to the last '/' and cut the buffer after that
		for (i=strlen(buff) -1; i>0; i--) {
			if (buff[i] == '/') {
				buff[i+1] = 0;
				break;
			}
		}
		Q_strcat(buff,sizeof(buff),file);
		status = GLua_LoadFile(L, &buff[1]); // skip the @ (see loader)
	}
	lua_pushboolean(L, status == 0);
	return 1;
}

static int GLua_FindMetatable(lua_State *L) {
	luaL_getmetatable(L, luaL_checkstring(L,1));
	return 1;
}

void GLua_RegisterProperties(lua_State *L, const GLua_Prop *props, int nup)
{
	STACKGUARD_INIT(L)

	// This assumes the (meta)table to assign the properties to is on top of the stack
	// This creates 2 subtables: __propget and __propset, each of which contain a table
	//  of methods and their associated functions. If no function is linked to it, it's set to 0.
	lua_newtable(L); // __propget
	lua_newtable(L); // __propset

	/* Stack layout:
	   [value]		   (-1)
	   <__propset>	-1 (-2)
	   <__propget>	-2 (-3)
	   <nups>		-3 (-4)
	   <table>		-3+nups (-4+nups)
	*/


	for (; props->prop; props++) {
		int i;
		if (props->setfunc) {
			for (i=0; i<nup; i++)  /* copy upvalues to the top */
				lua_pushvalue(L, -(nup+2));
			lua_pushcclosure(L, props->setfunc, nup);
			lua_setfield(L, -2, props->prop);
		} else {
			lua_pushnumber(L, 0);	// 'Not Available' marker
			lua_setfield(L, -2, props->prop);
		}
		if (props->getfunc) {
			for (i=0; i<nup; i++)  /* copy upvalues to the top */
				lua_pushvalue(L, -(nup+2));
			lua_pushcclosure(L, props->getfunc, nup);
			lua_setfield(L, -3, props->prop);
		} else {
			lua_pushnumber(L, 0);	// 'Not Available' marker
			lua_setfield(L, -3, props->prop);
		}
	}
	lua_setfield(L, -(nup+3), "__propset");
	lua_setfield(L, -(nup+2), "__propget");
	// Pop upvalues
	lua_pop(L, nup);

	STACKGUARD_CHECK(L)
}

void GLua_LoadBaseLibs(lua_State *L) {

	STACKGUARD_INIT(L)

	luaL_openlibs(L);
	// Time to make some adjustments
	//lua_pushnil(L); lua_setglobal(L,"debug"); // Debug module = gone
	lua_pushnil(L); lua_setglobal(L,"io"); // io module = gone (we'll create our own)
	// Remove some problematic functions in os
	lua_getglobal(L,"os");
	lua_pushstring(L,"exit"); lua_pushnil(L); lua_settable(L,-3); 
	lua_pushstring(L,"execute"); lua_pushnil(L); lua_settable(L,-3);
	lua_pushstring(L,"remove"); lua_pushnil(L); lua_settable(L,-3);
	lua_pushstring(L,"rename"); lua_pushnil(L); lua_settable(L,-3);
	lua_pushstring(L,"setlocale"); lua_pushnil(L); lua_settable(L,-3);
	lua_pushstring(L,"tmpname"); lua_pushnil(L); lua_settable(L,-3);
	lua_pop(L,1);
	// Add print and include
	lua_pushcclosure(L, GLua_Print, 0); lua_setglobal(L, "print");
	lua_pushcclosure(L, GLua_Include, 0); lua_setglobal(L, "include");
	lua_pushcclosure(L, GLua_FindMetatable, 0); lua_setglobal(L, "findmetatable");
	// Remove different ways to execute code, we only want include to be available
	lua_pushnil(L); lua_setglobal(L,"dofile"); 
	lua_pushnil(L); lua_setglobal(L,"loadfile");
	lua_pushnil(L); lua_setglobal(L,"load");
	lua_pushnil(L); lua_setglobal(L,"loadstring");

	STACKGUARD_CHECK(L)
}

void GLua_LoadLibs(lua_State *L) {
	GLua_Define_Sys(L);
	GLua_Define_Bit(L);
	GLua_Define_Encoding(L);
	GLua_Define_Cryptography(L);
	GLua_Define_BitStream(L);
	GLua_Define_PRNG(L);
	GLua_Define_File(L);
	GLua_Define_Cvar(L);
	GLua_Define_Vector(L);
	GLua_Define_Entity(L);
	GLua_Define_Player(L);
	GLua_Define_NPC(L);
}