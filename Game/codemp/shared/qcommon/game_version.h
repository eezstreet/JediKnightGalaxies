// Copyright (C) 2000-2002 Raven Software, Inc.
//

#ifndef _GAME_VERSION_H
#define _GAME_VERSION_H

#include "../win32/AutoVersion.h"

#define JKG_VERSION "0.4.12"
// Current version of the multi player game
#ifdef _DEBUG
	#define JKG_VERSION_SUFFIX "d"
	#define	Q3_VERSION		"(debug)JAmp: v"VERSION_STRING_DOTTED"/JKG: v"JKG_VERSION JKG_VERSION_SUFFIX
	
#elif defined FINAL_BUILD
	#define JKG_VERSION_SUFFIX ""
	#define	Q3_VERSION		"JAmp: v"VERSION_STRING_DOTTED"/JKG: v"JKG_VERSION JKG_VERSION_SUFFIX
	
#else
	#define JKG_VERSION_SUFFIX "r"
	#define	Q3_VERSION		"(internal)JAmp: v"VERSION_STRING_DOTTED"/JKG: v"JKG_VERSION JKG_VERSION_SUFFIX
#endif

#define	GAMEVERSION	"Jedi Knight Galaxies v"JKG_VERSION JKG_VERSION_SUFFIX

#endif

//end
