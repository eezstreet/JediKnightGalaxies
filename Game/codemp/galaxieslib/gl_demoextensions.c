//       ____ ___________________   ___           ____  __ _______   ___  ________  ___ ______________
//      |    |\_   _____/\______ \ |   |         |    |/ _|\      \ |   |/  _____/ /   |   \__    ___/
//      |    | |    __)_  |    |  \|   |         |      <  /   |   \|   /   \  ___/    ~    \|    |   
//  /\__|    | |        \ |    `   \   |         |    |  \/    |    \   \    \_\  \    Y    /|    |   
//  \________|/_______  //_______  /___|         |____|__ \____|__  /___|\______  /\___|_  / |____|   
//                    \/         \/                      \/       \/            \/       \/           
//                         ________    _____   ____       _____  ____  ___ ______________ _________   
//                        /  _____/   /  _  \ |    |     /  _  \ \   \/  /|   \_   _____//   _____/   
//                       /   \  ___  /  /_\  \|    |    /  /_\  \ \     / |   ||    __)_ \_____  \    
//                       \    \_\  \/    |    \    |___/    |    \/     \ |   ||        \/        \   
//                        \______  /\____|__  /_______ \____|__  /___/\  \|___/_______  /_______  /   
//                               \/         \/        \/	   \/	   \_/			  \/        \/ (c)
// gl_demoextensions.c
// This module implements extensions for demo playback.
// (c) 2013 Jedi Knight Galaxies

// =================================================
// Demo extension module
// -------------------------------------------------
// This module implements extensions for
// demo playback.
// =================================================

#include "gl_enginefuncs.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

cvar_t *cl_avidemoformat;
cvar_t *cl_avidemostart;

// clc.demoplaying @ 0x954B6C
static int demoShotNumber;
char	*  va( char *format, ... );


void GL_DPE_Demo(void) {
	demoShotNumber = cl_avidemostart->integer;
	((void (*)(void))0x41D000)();
}

void GL_DPE_Init() {
	// Demo Playback Extensions
	cl_avidemoformat = Cvar_Get("cl_avidemoformat", "jpg", CVAR_ARCHIVE);
	cl_avidemostart = Cvar_Get("cl_avidemostart", "0", CVAR_ARCHIVE);

	// Redefine /demo
	Cmd_RemoveCommand("demo");
	Cmd_AddCommand("demo", GL_DPE_Demo);
}



void GL_DPE_TakeDemoShot() {
	const char *format = cl_avidemoformat->string;
	if (*(int *)0x954B6C) {	// We're playing back a demo
		if (!_stricmp(format, "png")) {	// Check for png
			Cmd_ExecuteString(va("screenshot_png demo%0.6i\n", demoShotNumber++));
		} else if (!_stricmp(format, "tga")) {	// Check for tga
			Cmd_ExecuteString(va("screenshot_tga demo%0.6i\n", demoShotNumber++));
		} else {	// Default to jpg
			Cmd_ExecuteString(va("screenshot demo%0.6i\n", demoShotNumber++));
		}
	} else {
		if (Cvar_GetValueInt("cl_forceavidemo")) {
			if (!_stricmp(format, "png")) {	// Check for png
				Cmd_ExecuteString("screenshot_png silent\n");
			} else if (!_stricmp(format, "tga")) {	// Check for tga
				Cmd_ExecuteString("screenshot_tga silent\n");
			} else {	// Default to jpg
				Cmd_ExecuteString("screenshot silent\n");
			}
		}
	}
}